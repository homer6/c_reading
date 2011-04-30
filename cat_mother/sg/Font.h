#ifndef _SG_FONT_H
#define _SG_FONT_H


#include <sg/Primitive.h>
#include <lang/String.h>


namespace io {
	class InputStream;}


namespace sg
{


class TriangleList;


/** 
 * Bitmap text class. 
 * Can be used to render both 2D and 3D text.
 * 2D text is rendered using explicit call to drawText.
 * 3D text is rendered by adding the primitive to a mesh,
 * which in turn calls draw() when the mesh primitives are rendered.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Font :
	public Primitive
{
public:
	/**
	 * Loads the font from two files: Glyph image file and 
	 * character set text file.
	 *
	 * About character set file: Character set file consists of whitespace 
	 * delimited character sequences which correspond to the glyph indices
	 * in the image file. 
	 *
	 * About glyph image file: Glyph image file consists of a row
	 * of glyphs. First row of pixels can be used to mark 
	 * glyph widths. If the glyph width is not marked then the function
	 * guesses the width by scanning for 'empty' columns of pixels.
	 * Empty color is taken from the first pixel of the image.
	 *
	 * @param imgin Input stream of image file.
	 * @param csin Input stream of character sequences defined in image file.
	 * @param tri Buffer for rendered triangle. Can be 0 (created run-time). Vertex format must have uv and RHW.
	 * @exception IOException
	 */
	Font( io::InputStream* imgin, io::InputStream* csin, sg::TriangleList* tri=0 );

	/**
	 * Copy by value. (immutable properties, texture and character set, 
	 * are still shared)
	 */
	Font( const Font& other, int shareFlags );

	///
	~Font();

	/**
	 * Copy by value. (immutable properties, texture and character set, 
	 * are still shared)
	 */
	Primitive* clone( int shareFlags ) const;

	/** Releases resources of the object. */
	void	destroy();

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** 
	 * Draws 2D text to active rendering device. Text is rendered 
	 * in device coordinates using original bitmap size.
	 * @param x Top left X-coordinate (pixels) of the rendered text. 
	 * @param y Top left Y-coordinate (pixels) of the rendered text. 
	 * @param str Text to render.
	 * @param x1 [out] Receives bottom right X-coordinate (pixels). Pass 0 if not needed.
	 * @param y1 [out] Receives bottom right Y-coordinate (pixels). Pass 0 if not needed.
	 * @param scalex Width scaling (optional, defaults to 1).
	 * @param scaley Heigth scaling (optional, defaults to 1).
	 */
	void	drawText( float x, float y, const lang::String& str,
				float* x1=0, float* y1=0, float scalex=1.f, float scaley=1.f );

	/** Draws 3D text primitive to the active rendering device. */
	void	draw();
	
	/** Sets 3D text primitive string. */
	void	setText( const lang::String& str );

	/** 
	 * Sets scale (3D, model space units) of the 3D text.
	 * Scale is applied to the size (pixels) of the bitmaps 
	 * before rendering them in 3D.
	 * Default scale is 0.1 (=1 meter is 10 pixels).
	 */
	void	setScale( float scale );

	/** Returns width of the font text in pixels. */
	float	getWidth( const lang::String& str ) const;

	/** Returns height of the font in pixels. */
	float	height() const;

	/** Returns glyph index by string. */
	int		getGlyphIndex( const lang::String& str ) const;

	/** Returns glyph string by index. */
	lang::String	getGlyphString( int index ) const;

	/** Returns number of glyphs. */
	int		numGlyphs() const;

	/** Returns vertex format used by the font. */
	VertexFormat	vertexFormat() const;

private:
	class FontShared;
	P(FontShared)	m_shared;
	lang::String	m_text;
	float			m_scale;

	Font( const Font& );
	Font& operator=( const Font& );
};


} // sg


#endif // _SG_FONT_H
