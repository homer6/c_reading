#include "CubeTexture.h"
#include "Context.h"
#include "TextureCache.h"
#include "LockException.h"
#include <io/FileInputStream.h>
#include <gd/CubeTexture.h>
#include <gd/LockMode.h>
#include <gd/GraphicsDriver.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


//-----------------------------------------------------------------------------

CubeTexture::CubeTexture( int edgelength, const pix::SurfaceFormat& format )
{
	m_tex = Context::driver()->createCubeTexture();
	int err = m_tex->create( edgelength, format );
	if ( err )
		throw Exception( Format("Failed to create rendering device texture {0}x{1}", edgelength, edgelength) );
}

CubeTexture::CubeTexture( io::InputStream* in )
{
	String filename = in->toString();
	m_tex = getTextureCache().loadCubeTexture( in, filename, Context::driver(), Context::device() );
	BaseTexture::setName(filename);
}

CubeTexture::CubeTexture( io::InputStream* in, const lang::String& filename )
{
	m_tex = getTextureCache().loadCubeTexture( in, filename, Context::driver(), Context::device() );
	BaseTexture::setName(filename);
}

/*CubeTexture::CubeTexture( const String& filename )
{
	FileInputStream in( filename );
	m_tex = getTextureCache().loadTexture( &in, filename, Context::driver(), Context::device() );
	in.close();
	m_name = filename;
}*/

CubeTexture::~CubeTexture()
{
	destroy();
}

void CubeTexture::load()
{
	gd::GraphicsDevice* device = Context::device();
	if ( m_tex && device )
		m_tex->load( device );
}

void CubeTexture::unload()
{
	if ( m_tex )
		m_tex->unload();
}

void CubeTexture::destroy()
{
	m_tex = 0;
	ContextObject::destroy();
}

void CubeTexture::blt( const pix::Surface* img, int subsurface )
{
	assert( m_tex );

	if ( m_tex->width() > 0 && m_tex->height() > 0 )
	{
		if ( !m_tex->lock( gd::LockMode::LOCK_WRITE, subsurface ) )
			throw LockException( "Texture" );

		pix::Surface::blt( m_tex->data(subsurface), 0, 0, m_tex->width(), m_tex->height(), m_tex->pitch(), m_tex->format(),
			img->data(), img->width(), img->height(), img->pitch(), img->format() );

		m_tex->unlock( subsurface );
		load();
	}
}

int CubeTexture::width() const
{
	assert( m_tex );
	return m_tex->width();
}

int CubeTexture::height() const
{
	assert( m_tex );
	return m_tex->height();
}

const pix::SurfaceFormat& CubeTexture::format() const
{
	assert( m_tex );
	return m_tex->format();
}

int	CubeTexture::pitch() const
{
	assert( m_tex );
	return m_tex->pitch();
}

long CubeTexture::textureMemoryUsed() const
{
	assert( m_tex );
	return m_tex->textureMemoryUsed();
}

void CubeTexture::flushTextures()
{
	getTextureCache().flushTextures();
}

void CubeTexture::setDownScaling( bool enabled )
{
	getTextureCache().setDownScaling( enabled );
	if ( enabled )
		Debug::println( "Half resolution textures" );
	else
		Debug::println( "Full resolution textures" );
}

void CubeTexture::setDefaultBitDepth( int bits )
{
	getTextureCache().setDefaultBitDepth( bits );
	Debug::println( "Setting default texture bit depth to {0}", bits );
}

long CubeTexture::totalTextureMemoryUsed()
{
	return getTextureCache().textureMemoryUsed();
}

gd::CubeTexture* CubeTexture::texture() const
{
	return m_tex;
}

gd::BaseTexture* CubeTexture::baseTexture() const
{
	return m_tex;
}

CubeTexture::MapType CubeTexture::mapType() const
{
	return MAP_CUBEMAP;
}


} // sg
