#include "Texture.h"
#include "Context.h"
#include "TextureCache.h"
#include "LockException.h"
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <gd/Texture.h>
#include <gd/LockMode.h>
#include <gd/GraphicsDriver.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceUtil.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


//-----------------------------------------------------------------------------

Texture::Texture( int width, int height, const pix::SurfaceFormat& format, UsageType usage )
{
	m_tex = Context::driver()->createTexture();
	int err = m_tex->create( Context::device(), width, height, format, (gd::Texture::UsageType)usage );
	if ( err )
		throw Exception( Format("Failed to create rendering device texture {0}x{1}", width, height) );
}

Texture::Texture( io::InputStream* in )
{
	String filename = in->toString();
	m_tex = getTextureCache().loadTexture( in, filename, Context::driver(), Context::device() );
	BaseTexture::setName(filename);
}

Texture::Texture( io::InputStream* in, const lang::String& filename )
{
	m_tex = getTextureCache().loadTexture( in, filename, Context::driver(), Context::device() );
	BaseTexture::setName(filename);
}

/*Texture::Texture( const String& filename )
{
	FileInputStream in( filename );
	m_tex = s_texcache.loadTexture( &in, filename, Context::driver(), Context::device() );
	in.close();
	m_name = filename;
}*/

Texture::~Texture()
{
	destroy();
}

void Texture::load()
{
	gd::GraphicsDevice* device = Context::device();
	if ( m_tex && device )
		m_tex->load( device );
}

void Texture::unload()
{
	if ( m_tex )
		m_tex->unload();
}

void Texture::destroy()
{
	m_tex = 0;
	ContextObject::destroy();
}

void Texture::blt( const pix::Surface* img )
{
	assert( m_tex );

	if ( m_tex->width() > 0 && m_tex->height() > 0 )
	{
		if ( !m_tex->lock( gd::LockMode::LOCK_WRITE ) )
			throw LockException( "Texture" );

		pix::Surface::blt( m_tex->data(), 0, 0, m_tex->width(), m_tex->height(), m_tex->pitch(), m_tex->format(),
			img->data(), img->width(), img->height(), img->pitch(), img->format() );

		m_tex->unlock();
		load();
	}
}

int Texture::width() const
{
	assert( m_tex );
	return m_tex->width();
}

int Texture::height() const
{
	assert( m_tex );
	return m_tex->height();
}

const pix::SurfaceFormat& Texture::format() const
{
	assert( m_tex );
	return m_tex->format();
}

int	Texture::pitch() const
{
	assert( m_tex );
	return m_tex->pitch();
}

long Texture::textureMemoryUsed() const
{
	assert( m_tex );
	return m_tex->textureMemoryUsed();
}

void Texture::flushTextures()
{
	getTextureCache().flushTextures();
}

void Texture::setDownScaling( bool enabled )
{
	getTextureCache().setDownScaling( enabled );
	if ( enabled )
		Debug::println( "Half resolution textures" );
	else
		Debug::println( "Full resolution textures" );
}

void Texture::setDefaultBitDepth( int bits )
{
	getTextureCache().setDefaultBitDepth( bits );
	Debug::println( "Setting default texture bit depth to {0}", bits );
}

long Texture::totalTextureMemoryUsed()
{
	return getTextureCache().textureMemoryUsed();
}

gd::Texture* Texture::texture() const
{
	return m_tex;
}

gd::BaseTexture* Texture::baseTexture() const
{
	return m_tex;
}

Texture::MapType Texture::mapType() const
{
	return MAP_BITMAP;
}

long Texture::getPixel( float u, float v ) const
{
	if ( !m_tex->lock( gd::LockMode::LOCK_READ ) )
		throw LockException( "Texture" );

	int						w		= m_tex->width();
	int						h		= m_tex->height();
	int						x		= Math::max( Math::min((int)(u * (float)w + .5f), w-1), 0 );
	int						y		= Math::max( Math::min((int)(v * (float)h + .5f), h-1), 0 );
	const void*				data	= m_tex->data();
	int						pitch	= m_tex->pitch();
	const SurfaceFormat&	format	= m_tex->format();

	long pixel = SurfaceUtil::getPixel( x, y, w, h, data, pitch, format );

	/*bool save = false;
	if ( save )
	{
		FileOutputStream fout( "C:/Documents and Settings/jani.CATMOTHER_02/My Documents/tmp/out/texread.tga" );
		Image img( width(), height(), SurfaceFormat::SURFACE_A8R8G8B8 );
		
		for ( int j = 0 ; j < h ; ++j )
			for ( int i = 0 ; i < w ; ++i )
				img.surface().setPixel( i, j, SurfaceUtil::getPixel(i,j,w,h,data,pitch,format) );
		
		img.save( &fout );
	}*/

	m_tex->unlock();
	return pixel;
}


} // sg
