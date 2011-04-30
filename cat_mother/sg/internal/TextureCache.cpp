#include "TextureCache.h"
#include <sg/Context.h>
#include <gd/Texture.h>
#include <gd/CubeTexture.h>
#include <gd/LockMode.h>
#include <gd/GraphicsDriver.h>
#include <io/File.h>
#include <io/FileInputStream.h>
#include <dev/Profile.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace dev;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace sg
{


TextureCache::TextureCache() :
	Object( OBJECT_INITMUTEX ),
	m_textures( Allocator< HashtablePair<lang::String,P(gd::BaseTexture)> >(__FILE__,__LINE__) )
{
	m_downScaling = false;
	m_bitDepth = 32;
}

gd::Texture* TextureCache::loadTexture( InputStream* in, const String& name, gd::GraphicsDriver* drv, gd::GraphicsDevice* dev )
{
	//Profile pr( "sg.TextureCache.loadTexture" );
	synchronized( this );
	return loadTextureImpl( in, name, drv, dev );
}

gd::CubeTexture* TextureCache::loadCubeTexture( InputStream* in, const String& name, gd::GraphicsDriver* drv, gd::GraphicsDevice* dev )
{
	//Profile pr( "sg.TextureCache.loadTexture" );
	synchronized( this );
	return loadCubeTextureImpl( in, name, drv, dev );
}

gd::Texture* TextureCache::loadTextureImpl( InputStream* in, 
	const String& name, gd::GraphicsDriver* drv, gd::GraphicsDevice* dev )
{
	// load from cache if texture is already there
	String fname = File(name).getName();
	if ( m_textures.containsKey( fname ) )
	{
		gd::BaseTexture* tex = m_textures.get( fname );
		return static_cast<gd::Texture*>(tex);
	}
	//Debug::println( "Loading texture {0}", fname );

	// load from input stream
	P(Image) img = 0;

	P(Image)		img0	= new Image( in, name );
	bool			changed	= false;
	SurfaceFormat	fmt		= img0->format();	

	if ( fmt.bltSupported() )
	{
		for ( int i = 0; i < img0->surfaces(); ++i )
		{
			int				w	= img0->surface(i).width();
			int				h	= img0->surface(i).height();

			// pixel format conversion (if requested)
			if ( 16 == m_bitDepth && fmt.pixelSize() > 2 )
			{
				changed = true;
				if ( fmt.hasAlpha() )
					fmt = SurfaceFormat::SURFACE_A4R4G4B4;
				else
					fmt = SurfaceFormat::SURFACE_R5G5B5;
			}

			// halve texture size (if requested)
			if ( m_downScaling )
			{
				if ( w > 32 )
				{
					w /= 2;
					changed = true;
				}
				if ( h > 32 )
				{
					h /= 2;
					changed = true;
				}
			}

			if ( fname.indexOf("_NOTC") != -1 )
				fmt.setCompressable( false );
			
			// update modified texture
			if ( changed )
			{
				Surface surf0( w, h, fmt );
				surf0.blt( &img0->surface(i) );
				surf0.swap( img0->surface(i) );
			}
		}
	}
	img = img0;

	// create rendering device texture and put to cache
	P(gd::Texture) tex = drv->createTexture();
	// create texture, generate mipmaps if file had only one level
	tex->create( Context::device(), img->surfaceArray(), img->mipMapLevels() == 1 ? 0 : img->mipMapLevels() );
	m_textures.put( fname, tex.ptr() );

	// upload to rendering device if the device has been created
	//Debug::println( "Loaded texture {0}", fname );
	if ( dev )
		tex->load( dev );

	//Debug::println( "Uploaded texture {0}", fname );
	return tex;
}

gd::CubeTexture* TextureCache::loadCubeTextureImpl( InputStream* in, 
	const String& name, gd::GraphicsDriver* drv, gd::GraphicsDevice* dev )
{
	// load from cache if texture is already there
	String fname = File(name).getName();
	if ( m_textures.containsKey( fname ) )
	{
		gd::BaseTexture* tex = m_textures.get( fname );
		return static_cast<gd::CubeTexture*>(tex);
	}
	Debug::println( "Loading cube texture {0}", fname );

	// load from input stream
	P(Image)		img = 0;

	P(Image)		img0	= new Image( in, name );
	bool			changed	= false;
	SurfaceFormat	fmt		= img0->format();	

	if ( fmt.bltSupported() )
	{
		for ( int i = 0; i < img0->surfaces(); ++i )
		{
			int				w	= img0->surface(i).width();
			int				h	= img0->surface(i).height();

			// pixel format conversion (if requested)
			if ( 16 == m_bitDepth && fmt.pixelSize() > 2 )
			{
				changed = true;
				if ( fmt.hasAlpha() )
					fmt = SurfaceFormat::SURFACE_A4R4G4B4;
				else
					fmt = SurfaceFormat::SURFACE_R5G5B5;
			}

			// halve texture size (if requested)
			if ( m_downScaling )
			{
				if ( w > 32 )
				{
					w /= 2;
					changed = true;
				}
				if ( h > 32 )
				{
					h /= 2;
					changed = true;
				}
			}

			if ( fname.indexOf("_NOTC") != -1 )
				fmt.setCompressable( false );
		
			// update modified texture
			if ( changed )
			{
				Surface surf0( w, h, fmt );
				surf0.blt( &img0->surface(i) );
				surf0.swap( img0->surface(i) );
			}
		}
	}
	img = img0;

	// create rendering device texture and put to cache
	P(gd::CubeTexture) tex = drv->createCubeTexture();
	// create texture, generate mipmaps if not in file
	tex->create( img->surfaceArray(), img->mipMapLevels() == 1 ? 0 : img->mipMapLevels() );
	m_textures.put( fname, tex.ptr() );

	// upload to rendering device if the device has been created
	Debug::println( "Loaded cube texture {0}", fname );
	if ( dev )
		tex->load( dev );

	Debug::println( "Uploaded cube texture {0}", fname );
	return tex;
}

void TextureCache::flushTextures()
{
	synchronized( this );

	for ( HashtableIterator<String,P(gd::BaseTexture)> it = m_textures.begin() ; 
		it != m_textures.end() ; ++it )
	{
		P(gd::BaseTexture) tex = it.value();
		
		tex->destroy();
		tex = 0;
		it.value() = 0;
	}

	m_textures.clear();
	m_textures = Hashtable<lang::String,P(gd::BaseTexture)>( Allocator< HashtablePair<lang::String,P(gd::BaseTexture)> >(__FILE__) );
}

long TextureCache::textureMemoryUsed() const
{
	synchronized( this );

	long bytes = 0;
	for ( HashtableIterator<String,P(gd::BaseTexture)> it = m_textures.begin() ; 
		it != m_textures.end() ; ++it )
	{
		P(gd::BaseTexture) tex = it.value();
		bytes += tex->textureMemoryUsed();
	}
	return bytes;
}

void TextureCache::setDownScaling( bool enabled )
{
	synchronized( this );

	m_downScaling = enabled;
}

void TextureCache::setDefaultBitDepth( int bits )
{
	synchronized( this );

	m_bitDepth = bits;
}


} // sg
