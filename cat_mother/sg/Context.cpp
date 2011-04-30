#include "Context.h"
#include "ContextObject.h"
#include "Texture.h"
#include "CubeTexture.h"
#include <lang/Exception.h>
#include <lang/DynamicLinkLibrary.h>
#include <gd/Errors.h>
#include <gd/GraphicsDriver.h>
#include <gd/GraphicsDevice.h>
#include <pix/Color.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


class Context::ContextImpl :
	public Object
{
public:
	DynamicLinkLibrary		dll;
	P(gd::GraphicsDriver)	drv;
	P(gd::GraphicsDevice)	dev;

	explicit ContextImpl( const String& name ) :
		dll( name ), drv(0), dev(0)
	{
		gd::createGraphicsDriverFunc createGraphicsDriver = (gd::createGraphicsDriverFunc)dll.getProcAddress( "createGraphicsDriver" );
		if ( !createGraphicsDriver )
			throw Exception( Format("Corrupted graphics library driver: {0}", name) );

		// check version
		gd::getGraphicsDriverVersionFunc getGraphicsDriverVersion = (gd::getGraphicsDriverVersionFunc)dll.getProcAddress( "getGraphicsDriverVersion" );
		if ( !getGraphicsDriverVersion )
			throw Exception( Format("Old graphics library driver: {0}", name) );
		int ver = getGraphicsDriverVersion();
		if ( ver != gd::GraphicsDriver::VERSION )
			throw Exception( Format("Wrong version ({1,#}) of the graphics library driver: {0}", name, ver) );

		drv = (*createGraphicsDriver)();
		if ( !drv )
			throw Exception( Format("Failed to init graphics library driver: {0}", name) );
	}

	void destroy()
	{
		close();

		Texture::flushTextures();
		ContextObject::destroyAll();

		if ( drv )
		{
			drv->destroy();
			drv = 0;
		}

		dll.close();
	}

	void open( int width, int height, int bitsPerPixel, int refreshRate,
		int surfaceFlags, RasterizerType rz, VertexProcessingType vp, gd::GraphicsDevice::TextureCompression tc )
	{
		assert( !dev );

		gd::GraphicsDevice::RasterizerType gdrz = togd( rz );
		gd::GraphicsDevice::VertexProcessingType gdvp = togd( vp );

		int gdbuf = 0;
		if ( surfaceFlags & SURFACE_TARGET )
			gdbuf += gd::GraphicsDevice::SURFACE_TARGET;
		if ( surfaceFlags & SURFACE_DEPTH )
			gdbuf += gd::GraphicsDevice::SURFACE_DEPTH;
		if ( surfaceFlags & SURFACE_STENCIL )
			gdbuf += gd::GraphicsDevice::SURFACE_STENCIL;

		gd::GraphicsDevice::WindowType gdwin = gd::GraphicsDevice::WINDOW_DESKTOP;
		if ( bitsPerPixel > 0 )
			gdwin = gd::GraphicsDevice::WINDOW_FULLSCREEN;

		dev = drv->createGraphicsDevice();
		int err = dev->create( width, height, bitsPerPixel, refreshRate,
			gdwin, gdrz, gdvp, gdbuf, tc );

		if ( gd::ERROR_NONE != err )
		{
			String str;
			if ( surfaceFlags & SURFACE_TARGET )
				str = str + "target / ";
			if ( surfaceFlags & SURFACE_DEPTH )
				str = str + "depth / ";
			if ( surfaceFlags & SURFACE_STENCIL )
				str = str + "stencil / ";
			if ( str.length() >= 2 )
				str = str.substring(0,str.length()-2);

			if ( 0 != bitsPerPixel )
				throw Exception( Format("Failed to initialize rendering device: {0,#}x{1,#} {2,#} bit, buffers: {3}", width, height, bitsPerPixel, str) );
			else
				throw Exception( Format("Failed to initialize rendering device: {0,#}x{1,#} desktop window, buffers: {2}", width, height, str) );
		}

		clear( SURFACE_TARGET|SURFACE_DEPTH|SURFACE_STENCIL );
	}

	void close()
	{
		if ( dev )
		{
			Texture::flushTextures();
			dev->destroy();
			dev = 0;
		}
	}

	void flushDeviceObjects()
	{
		dev->flushDeviceObjects();
	}

	void clear( int surfaces )
	{
		int gdsurfaces = 0;
		if ( surfaces & SURFACE_TARGET )
			gdsurfaces |= gd::GraphicsDevice::SURFACE_TARGET;
		if ( surfaces & SURFACE_DEPTH )
			gdsurfaces |= gd::GraphicsDevice::SURFACE_DEPTH;
		if ( surfaces & SURFACE_STENCIL )
			gdsurfaces |= gd::GraphicsDevice::SURFACE_STENCIL;

		if ( dev )
			dev->clear( gdsurfaces, pix::Color(0,0,0), 0 );
	}

	void present()
	{
		if ( dev )
			dev->present();
	}

	void setMipMapFilter( TextureFilterType mode )
	{
		if ( dev )
			dev->setMipMapFilter( togd(mode) );
	}

	void setMipMapLODBias( float bias )
	{
		if ( dev )
			dev->setMipMapLODBias( bias );
	}

	bool restore()
	{
		if ( dev )
			return dev->restore();
		else
			return false;
	}

	bool ready() const
	{
		if ( dev )
			return dev->ready();
		else
			return false;
	}

private:
	static gd::GraphicsDevice::TextureFilterType togd( TextureFilterType type )
	{
		return (gd::GraphicsDevice::TextureFilterType)type;
	}

	static gd::GraphicsDevice::VertexProcessingType togd( 
		Context::VertexProcessingType vp )
	{
		switch ( vp )
		{
		case Context::VERTEXP_HW:	return gd::GraphicsDevice::VERTEXP_HW;
		case Context::VERTEXP_SW:	return gd::GraphicsDevice::VERTEXP_SW;
		}
		return gd::GraphicsDevice::VERTEXP_SW;
	}

	static gd::GraphicsDevice::RasterizerType togd( 
		Context::RasterizerType rz )
	{
		switch ( rz )
		{
		case Context::RASTERIZER_HW:	return gd::GraphicsDevice::RASTERIZER_HW;
		case Context::RASTERIZER_SW:	return gd::GraphicsDevice::RASTERIZER_SW;
		}
		return gd::GraphicsDevice::RASTERIZER_HW;
	}

	ContextImpl( const ContextImpl& );
	ContextImpl& operator=( const ContextImpl& );
};

//-----------------------------------------------------------------------------

Context::ContextImpl* Context::sm_active = 0;

//-----------------------------------------------------------------------------

Context::Context( const String& name )
{
	assert( !sm_active );

	m_this = new ContextImpl( name );
	sm_active = m_this;
}

Context::~Context()
{
}

void Context::destroy()
{
	if ( m_this )
	{
		if ( m_this == sm_active )
			sm_active = 0;

		m_this->destroy();
	}
}

void Context::open()
{
	assert( m_this );
	m_this->open( 640, 480, 0, 0, SURFACE_TARGET|SURFACE_DEPTH|SURFACE_STENCIL, RASTERIZER_HW, VERTEXP_HW, gd::GraphicsDevice::TC_NONE );
}

void Context::open( int width, int height, int bitsPerPixel, int refreshRate,
				   int surfaceFlags, RasterizerType rz, VertexProcessingType vp, 
				   TextureCompressionType textureCompression )
{
	assert( m_this );
	
	gd::GraphicsDevice::TextureCompression tc = gd::GraphicsDevice::TC_NONE;
	if ( textureCompression == TC_COMPRESSED )
		tc = gd::GraphicsDevice::TC_COMPRESSED;

	m_this->open( width, height, bitsPerPixel, refreshRate, surfaceFlags, rz, vp, tc );
}

void Context::close()
{
	assert( m_this );
	m_this->close();
}

void Context::present()
{
	assert( m_this );
	m_this->present();
}

void Context::clear( int surfaces )
{
	assert( m_this );
	m_this->clear( surfaces );
}

void Context::beginScene()
{
	assert( m_this );
	m_this->dev->beginScene();
}

void Context::endScene()
{
	assert( m_this );
	m_this->dev->endScene();
}

bool Context::fullscreen() const
{
	assert( m_this );
	return m_this->dev && m_this->dev->fullscreen();
}

int	Context::width() const
{
	assert( m_this );
	if ( m_this->dev )
		return m_this->dev->width();
	else
		return 0;
}

int	Context::height() const
{
	assert( m_this );
	if ( m_this->dev )
		return m_this->dev->height();
	else
		return 0;
}

bool Context::initialized()
{
	return sm_active != 0 && sm_active->dev != 0;
}

gd::GraphicsDriver* Context::driver()
{
	assert( sm_active );
	return sm_active->drv;
}

gd::GraphicsDevice* Context::device()
{
	assert( sm_active );
	return sm_active->dev;
}

void Context::setMipMapFilter( TextureFilterType mode )
{
	m_this->setMipMapFilter( mode );
}

void Context::setMipMapLODBias( float bias )
{
	m_this->setMipMapLODBias( bias );
}

void Context::flushDeviceObjects()
{
	m_this->flushDeviceObjects();
}

bool Context::restore()
{
	return m_this->restore();
}

bool Context::ready() const
{
	return m_this->ready();
}


} // sg
