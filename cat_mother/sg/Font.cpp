#include "Font.h"
#include <sg/VertexLock.h>
#include <sg/TriangleList.h>
#include <sg/VertexFormat.h>
#include <sg/Texture.h>
#include <sg/Material.h>
#include <io/IOException.h>
#include <io/InputStreamReader.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <lang/Character.h>
#include <util/Vector.h>
#include <util/Hashtable.h>
#include <math/Vector4.h>
#include <math/ShapeBuffer.h>
#include <math/Matrix4x4.h>
#include <assert.h>
#include <stdint.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

/** Safe margin, in pixels, around stored glyphs. */
#define GLYPH_MARGIN 1

/** Space between adjacent glyphs in rendered text. */
#define GLYPH_SPACE 2

//-----------------------------------------------------------------------------

using namespace io;
using namespace gd;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{



/** Glyph rectangle. */
class GlyphRect
{
public:
	/** Source image top left X-coordinate (inclusive). */
	int		x0;
	/** Source image top left Y-coordinate (inclusive). */
	int		y0;
	/** Source image bottom right X-coordinate (inclusive). */
	int		x1;
	/** Source image bottom right Y-coordinate (inclusive). */
	int		y1;
	/** Target (fitted) image top left X-coordinate (inclusive). */
	int		tx;
	/** Target (fitted) image top left Y-coordinate (inclusive). */
	int		ty;

	GlyphRect() 
	{
		x0 = y0 = x1 = y1 = tx = ty = 0;
	}

	GlyphRect( int left, int top, int right, int bottom )
	{
		x0 = left;
		y0 = top;
		x1 = right;
		y1 = bottom;
		tx = 0;
		ty = 0;
	}

	int width() const
	{
		return x1 - x0 + 1;
	}

	int height() const
	{
		return y1 - y0 + 1;
	}
};

/** Sequence of characters that gets rendered as a single unit. */
class Glyph
{
public:
	/** String of characters to render. */
	String	str;
	/** Top left U-coordinate (inclusive). */
	float	x0;
	/** Top left V-coordinate (inclusive). */
	float	y0;
	/** Bottom right U-coordinate (exclusive). */
	float	x1;
	/** Bottom right V-coordinate (exclusive). */
	float	y1;

	/** 
	 * Sort longest strings first so that
	 * we can stop as soon as we find a full match.
	 * If the strings have equal length then we compare bitmap widths.
	 */
	bool operator<( const Glyph& other ) const	
	{
		int thislen = str.length();
		int otherlen = other.str.length();
		if ( thislen != otherlen )
			return thislen > otherlen;
		return str < other.str;
	}
};

class Font::FontShared :
	public Object
{
public:
	P(Shader) shader;

	FontShared( InputStream* imgin, InputStream* csin, TriangleList* tri ) :
		shader( 0 ), 
		m_glyphs( Allocator<Glyph>(__FILE__,__LINE__) ),
		m_model2( tri )
	{
		// load glyph image
		String image = imgin->toString();
		Image img( imgin, image );

		// find glyph rectangles
		Vector<GlyphRect> glyphs( Allocator<GlyphRect>(__FILE__,__LINE__) );
		Image img32( SurfaceFormat::SURFACE_A8R8G8B8, &img );
		uint32_t* img32data = (uint32_t*)img32.surface(0).data();
		findGlyphs( img32data, img.surface(0).width(), img.surface(0).height(), glyphs );

		// DEBUG: mark & save glyph positions
		/*if ( image.lastIndexOf("/") > 0 )
		{
			for ( int i = 0 ; i < glyphs.size() ; ++i )
			{
				img.setPixel( glyphs[i].x0, glyphs[i].y0, 0xFFFF0000 );
				img.setPixel( glyphs[i].x1, glyphs[i].y1, 0xFF00FF00 );
				img.setPixel( glyphs[i].x0, glyphs[i].y1, 0xFFFF0000 );
				img.setPixel( glyphs[i].x1, glyphs[i].y0, 0xFF00FF00 );
			}

			String fname = String( "/tmp/out/marked_") + image.substring( image.lastIndexOf("/")+1 );
			FileOutputStream imgout( fname );
			img.save( &imgout );
		}*/

		// convert empty color pixels to transparent
		int pixels = img.surface(0).width() * img.surface(0).height();
		uint32_t empty = *img32data;
		for ( int i = 0 ; i < pixels ; ++i )
		{
			if ( empty == img32data[i] )
				img32data[i] = 0;
		}

		// fit glyphs to square image
		int side = 64;
		int glyphHeight = img.surface(0).height();
		while ( side < glyphHeight )
			side += side;
		ShapeBuffer sb( side, side/glyphHeight );
		while ( !fitGlyphs(glyphs,sb,glyphHeight) )
		{
			side *= 2;
			sb.setSize( side, side/glyphHeight );
			if ( side > 4000 )
				throw IOException( Format("Too large font: {0}", image) );
		}

		// blit glyphs to square image (16-bit with 4-bit alpha)
		Image target( side, side, SurfaceFormat::SURFACE_A4R4G4B4 );
		for ( int i = 0 ; i < side ; ++i )
			for ( int j = 0 ; j < side ; ++j )
				target.surface(0).setPixel( i, j, 0 );
		for ( int i = 0 ; i < glyphs.size() ; ++i )
		{
			target.surface(0).blt( glyphs[i].tx, glyphs[i].ty, 
				glyphs[i].width(), glyphs[i].height(),
				&img32.surface(0), glyphs[i].x0, glyphs[i].y0, 
				glyphs[i].width(), glyphs[i].height() );
		}
		m_targetWidth = (float)target.surface(0).width();
		m_targetHeight = (float)target.surface(0).height();

		// DEBUG: save fitted glyphs
		/*if ( image.lastIndexOf("/") > 0 )
		{
			String fname = String( "/tmp/out/fitted_") + image.substring( image.lastIndexOf("/")+1 );
			FileOutputStream imgout( fname );
			target.save( &imgout );
		}*/

		// set up font shader
		SurfaceFormat texfmt = target.surface(0).format();
		if ( image.indexOf("_NOTC") == -1 )
			texfmt.setCompressable( false );
		P(Texture) tex = new Texture( side, side, texfmt, Texture::USAGE_NORMAL );
		tex->blt( &target.surface(0) );
		P(Material) mat = new Material;
		mat->setTexture( 0, tex );
		mat->setTextureColorCombine( 0, Material::TA_TEXTURE, Material::TOP_SELECTARG1, Material::TextureArgument() );
		mat->setTextureAlphaCombine( 0, Material::TA_TEXTURE, Material::TOP_SELECTARG1, Material::TextureArgument() );
		mat->setLighting( false );
		mat->setDepthWrite( false );
		mat->setFogDisabled( true );
		mat->setTextureFilter( 0, Material::TEXF_POINT );
		mat->setBlend( Material::BLEND_SRCALPHA, Material::BLEND_INVSRCALPHA );
		mat->setVertexFormat( defaultVertexFormat() );
		shader = mat.ptr();

		// read charset and setup glyphs
		const String charset = csin->toString();
		InputStreamReader reader( csin );
		Char ch;
		Char buf[256];
		int len = 0;
		int glyph = 0;
		float invw = 1.f / target.surface(0).width();
		float invh = 1.f / target.surface(0).height();
		while ( reader.read(&ch,1) == 1 )
		{
			if ( Character::isWhitespace(ch) )
			{
				if ( len > 0 )
				{
					if ( glyph >= glyphs.size() )
						throw IOException( Format("Number of glyphs in font image ({2}) and text file ({3}) do not match.\nImage was {0} and text file was {1}.", image, charset, glyphs.size(), glyph+1) );

					float gw = (float)glyphs[glyph].width();
					float gh = (float)glyphs[glyph].height();
					Glyph g;
					g.str = buf;
					g.x0 = ( glyphs[glyph].tx - 1.f ) * invw;
					g.y0 = ( glyphs[glyph].ty - 1.f ) * invh;
					g.x1 = ( glyphs[glyph].tx + gw ) * invw;
					g.y1 = ( glyphs[glyph].ty + gh ) * invh;
					m_glyphs.add( g );
					++glyph;
				}
				len = 0;
			}
			else
			{
				buf[len++] = ch;
				buf[len] = 0;
			}
		}
		std::sort( m_glyphs.begin(), m_glyphs.end() );

		if ( glyph < glyphs.size() )
			throw IOException( Format("Number of glyphs in font image ({2}) and text file ({3}) do not match.\nImage was {0} and text file was {1}.", image, charset, glyphs.size(), glyph) );
	}

	static VertexFormat defaultVertexFormat()
	{
		VertexFormat vf;
		vf.addRHW().addTextureCoordinate(2);
		return vf;
	}

	void drawText( float x, float y, float scalex, float scaley, const String& str,
		float* x1, float* y1 )
	{
		if ( !m_model2 )
			m_model2 = new TriangleList( 100*6, defaultVertexFormat(), TriangleList::USAGE_DYNAMIC );

		x -= .5f;
		y -= .5f;

		float maxdy = 0.f;
		int i = 0;
		while ( i < str.length() )
		{
			int verts = 0;

			{VertexLock<TriangleList> lock( m_model2, TriangleList::LOCK_WRITE );
			while ( i < str.length() && 
				verts < m_model2->maxVertices() )
			{
				Glyph* glyph = 0;
				if ( getGlyph(str, i, &glyph) )
				{
					assert( glyph->str.length() > 0 );

					float dx = getGlyphWidth( glyph ) * scalex;
					float dy = getGlyphHeight( glyph ) * scaley;
					if ( dy > maxdy )
						maxdy = dy;
					
					Vector4 pos[6] =
					{
						Vector4(x,y,0,1), 
						Vector4(x+dx,y,0,1),
						Vector4(x+dx,y+dy,0,1),
						Vector4(x,y,0,1), 
						Vector4(x+dx,y+dy,0,1),
						Vector4(x,y+dy,0,1)
					};
					m_model2->setVertexPositionsRHW( verts, pos, 6 );
					
					float uv[2*6] =
					{
						glyph->x0, glyph->y0,
						glyph->x1, glyph->y0,
						glyph->x1, glyph->y1,
						 glyph->x0, glyph->y0,
						 glyph->x1, glyph->y1,
						glyph->x0, glyph->y1
					};
					m_model2->setVertexTextureCoordinates( verts, 0, 2, uv, 6 );
					
					verts += 6;
					x += dx + GLYPH_SPACE;
					i += glyph->str.length();
				}
				else
				{
					if ( Character::isWhitespace( str.charAt(i) ) )
						x += spaceWidth();
					++i;
				}
			}}

			if ( verts > 0 )
			{
				m_model2->setVertices( verts );
				m_model2->setShader( shader );
				m_model2->draw();
			}
		}

		if ( x1 )
			*x1 = x;
		if ( y1 )
			*y1 = y + maxdy;
	}

	float spaceWidth() const
	{
		// prefer half of the small 'w' as space width
		const Glyph* glyph = 0;
		if ( !getGlyph("w", 0, &glyph) )
			glyph = &m_glyphs[ m_glyphs.size()/2 ];

		return getGlyphWidth( glyph ) *  .5f;
	}

	float getWidth( const String& str ) const
	{
		float x = 0.f;
		int i = 0;
		while ( i < str.length() )
		{
			const Glyph* glyph = 0;
			if ( getGlyph(str, i, &glyph) )
			{
				assert( glyph->str.length() > 0 );

				float dx = getGlyphWidth( glyph );
				x += dx + GLYPH_SPACE;
				i += glyph->str.length();
			}
			else
			{
				if ( Character::isWhitespace( str.charAt(i) ) )
					x += spaceWidth();
				++i;
			}
		}
		return x;
	}

	float height() const
	{
		return (m_glyphs[0].y1 - m_glyphs[0].y0) * m_targetHeight;
	}

	const Glyph* getGlyph( int index ) const
	{
		return &m_glyphs[index];
	}

	int getGlyphIndex( const String& str ) const
	{
		for ( int i = 0 ; i < m_glyphs.size() ; ++i )
		{
			if ( m_glyphs[i].str == str ) return i;
		}
		return -1;
	}

	int	numGlyphs() const
	{
		return m_glyphs.size();
	}

	VertexFormat vertexFormat() const
	{
		if ( m_model2 )
		{
			return m_model2->vertexFormat();
		}
		else
		{
			VertexFormat vf;
			vf.addRHW().addTextureCoordinate(2);
			return vf;
		}
	}

private:

	/**
	 * Finds glyph.
	 * @return true if found.
	 */
	bool getGlyph( const String& str, int pos, Glyph** glyph )
	{
		for ( int i = 0 ; i < m_glyphs.size() ; ++i )
		{
			if ( str.regionMatches(pos, m_glyphs[i].str, 0, 
				m_glyphs[i].str.length()) )
			{
				*glyph = &m_glyphs[i];
				return true;
			}
		}
		return false;
	}

	/**
	 * Finds glyph.
	 * @return true if found.
	 */
	bool getGlyph( const String& str, int pos, const Glyph** glyph ) const
	{
		for ( int i = 0 ; i < m_glyphs.size() ; ++i )
		{
			if ( str.regionMatches(pos, m_glyphs[i].str, 0, 
				m_glyphs[i].str.length()) )
			{
				*glyph = &m_glyphs[i];
				return true;
			}
		}
		return false;
	}

	/** 
	 * Fits glyphs to the shape buffer. 
	 * Fills in target positions of the glyphs.
	 * @return true if all shapes fit.
	 */
	bool fitGlyphs( Vector<GlyphRect>& glyphs, ShapeBuffer& sb, int glyphHeight )
	{
		sb.clear();
		for ( int i = 0 ; i < glyphs.size() ; ++i )
		{
			GlyphRect& g = glyphs[i];
			int x0 = 0;
			int y0 = 0;
			int x1 = g.width() + GLYPH_MARGIN;
			int y1 = 0;
			int dx = 0;
			int dy = 0;
			if ( !sb.fitRectangle(x0, y0, x1, y1, &dx, &dy) )
			{
				sb.clear();
				return false;
			}
			x0 += dx;
			x1 += dx;
			y0 += dy;
			y1 += dy;
			sb.drawRectangle( x0, y0, x1, y1, i );
			
			g.tx = dx;
			g.ty = dy * glyphHeight;
		}
		return true;
	}

	/** Returns true if all the column pixels are the same color. */
	bool isSolidColumn( int x, const uint32_t* img, int w, int h, uint32_t colr )
	{
		uint32_t colrRGB = colr &= 0xFFFFFF;
		for ( int y = 0 ; y < h ; ++y )
		{
			uint32_t c = img[x+y*w];
			//if ( c != colr )					// testing alpha
			if ( (c&0xFFFFFF) != colrRGB )		// ignoring alpha
				return false;
		}
		return true;
	}

	/** 
	 * Lists all glyphs from an image. 
	 * First row of pixels can be used to mark glyph widths.
	 * If the glyph width is not marked then the function
	 * guesses the width by scanning for 'empty' columns of pixels.
	 * Empty color is taken from the first pixel of the image.
	 */
	void findGlyphs( const uint32_t* img, int w, int h, Vector<GlyphRect>& glyphs )
	{
		uint32_t empty = *img;
		
		int x = 0;
		while ( x < w )
		{
			// find glyph begin (inclusive)
			for ( ; x < w ; ++x )
				if ( !isSolidColumn(x,img,w,h,empty) )
					break;

			// no more glyphs?
			if ( x >= w )
				break;

			// find glyph end (exclusive)
			int x1 = x + 1;
			for ( ; x1 < w ; ++x1 )
				if ( isSolidColumn(x1,img,w,h,empty) )
					break;

			// add found glyph rect (inclusive)
			if ( x1-x > 0 )
			{
				GlyphRect rc( x, 1, x1-1, h-1 );
				glyphs.add( rc );
			}

			// proceed to next glyph
			x = x1;
		}
	}

	inline float getGlyphWidth( const Glyph* g ) const
	{
		return (g->x1 - g->x0) * m_targetWidth;
	}

	inline float getGlyphHeight( const Glyph* g ) const
	{
		return (g->y1 - g->y0) * m_targetWidth;
	}

	Vector<Glyph>		m_glyphs;
	P(TriangleList)		m_model2;
	float				m_targetWidth;
	float				m_targetHeight;

	FontShared( const FontShared& );
	FontShared& operator=( const FontShared& );
};

//-----------------------------------------------------------------------------

Font::Font( InputStream* imgin, InputStream* csin, TriangleList* tri )
{
	m_shared = new FontShared( imgin, csin, tri );
	m_text = "abc";
	m_scale = 0.1f;
	
	setShader( m_shared->shader );
}

Font::Font( const Font& other, int shareFlags ) :
	Primitive( other, shareFlags )
{
	m_shared = other.m_shared;
	m_text = other.m_text;
	m_scale = other.m_scale;
}

Font::~Font()
{
}

Primitive* Font::clone( int shareFlags ) const
{
	return new Font( *this, shareFlags );
}

void Font::destroy()
{
}

void Font::load()
{
}

void Font::unload()
{
}

void Font::drawText( float x, float y, const String& str,
	float* x1, float* y1, float scalex, float scaley ) 
{
	m_shared->shader = shader();
	m_shared->drawText( x, y, scalex, scaley, str, x1, y1 ); // Parameter order of this function is different, because the class function parameter order had to be backward compatible
}

void Font::draw()
{
}

void Font::setText( const String& str )
{
	m_text = str;
}

void Font::setScale( float scale )
{
	m_scale = scale;
}

float Font::getWidth( const String& str ) const
{
	return m_shared->getWidth( str );
}

float Font::height() const
{
	return m_shared->height();
}

int Font::getGlyphIndex( const lang::String& str ) const 
{
	return m_shared->getGlyphIndex( str );		
}

lang::String Font::getGlyphString( int index ) const 
{
	const Glyph* g = m_shared->getGlyph( index );	
	return g->str;
}

int Font::numGlyphs() const 
{
	return m_shared->numGlyphs();
}

VertexFormat Font::vertexFormat() const
{
	return m_shared->vertexFormat();
}


} // sg
