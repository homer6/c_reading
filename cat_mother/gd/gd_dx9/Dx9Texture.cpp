#include "StdAfx.h"
#include "Array.h"
#include "toDx9.h"
#include "Dx9Texture.h"
#include "error.h"
#include "toString.h"
#include "toSurfaceFormat.h"
#include "Dx9SurfaceLock.h"
#include "Dx9GraphicsDevice.h"
#include <gd/Errors.h>
#include <gd/LockMode.h>
#include <pix/SurfaceUtil.h>
#include <assert.h>
#include <stdio.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace gd;
using namespace pix;

//-----------------------------------------------------------------------------

Dx9Texture::Dx9Texture() :
	m_refs(0),
	m_width(0),
	m_height(0),
	m_format(),
	m_data(0),
	m_pitch(0)
{
	m_tex = 0;
}

Dx9Texture::~Dx9Texture()
{
	destroy();
}

void Dx9Texture::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx9Texture::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx9Texture::destroy()
{
	destroyDeviceObject();

	if ( m_tex )
	{
		m_tex->Release();
		m_tex = 0;
	}
}

void Dx9Texture::load( gd::GraphicsDevice* device ) {}

void Dx9Texture::load( GraphicsDevice* device, Surface* surfaces, int mipmaplevels, UsageType usage )
{
	assert( device );
	if ( !device )
		return;

	Dx9GraphicsDevice*	dev		= static_cast<Dx9GraphicsDevice*>( device );
	IDirect3DDevice9*	d3ddev	= dev->d3dDevice();
	const D3DCAPS9&		caps	= dev->caps();

	if ( !m_tex )
	{
		UINT			width	= surfaces[0].width();
		UINT			height	= surfaces[0].height();
		UINT			levels	= mipmaplevels;
		DWORD			usagef	= 0;
		SurfaceFormat	fmt		= surfaces[0].format();
		D3DFORMAT		format	= toDx9( fmt );
		D3DPOOL			pool	= D3DPOOL_MANAGED;

		// create mipmaps?
//		if ( device->mipMapFilter() != GraphicsDevice::TEXF_NONE )
//			levels = 0;

		// 2^x aligned texture size?
		if ( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
		{
			width = roundUpPow2(width);
			height = roundUpPow2(height);
		}

		// square textures?
		if ( caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
		{
			if ( width > height )
				height = width;
			else if ( height > width )
				width = height;
		}

		// DXT textures have minimum texture size
		if ( dev->textureCompressionEnabled() && fmt.compressable() ||
			fmt.compressed() )
		{
			if ( width < 4 )
				width = 4;
			if ( height < 4 )
				height = 4;
		}

		// render-to-texture ?
		if ( usage == USAGE_RENDERTARGET )
		{
			usagef |= D3DUSAGE_RENDERTARGET;
			pool = D3DPOOL_DEFAULT;
		}

		// get device creation parameters
		D3DDEVICE_CREATION_PARAMETERS cp;
		d3ddev->GetCreationParameters( &cp );

		// create in the closest matching pixel format
		assert( format != D3DFMT_UNKNOWN );
		D3DFORMAT originalFormat = format;
		int tryCount = 0;
		HRESULT hr;
		do
		{
			format = getNextBestFormat( format, tryCount++ );
			//if ( tryCount > 1 )
			//	message( "Device does not support texture format %s, now trying %s.", toString(originalFormat), toString(format) );

			if ( format == D3DFMT_UNKNOWN )
			{
				error( "Failed to create texture: width=%i, height=%i, levels=%i, usage=0x%X, format=%s, pool=0x%X.", width, height, levels, (int)usage, toString(originalFormat), pool );
				return;
			}
			
			if ( dev->textureCompressionEnabled() && dev->textureCompressionSupported() == GraphicsDevice::TCSUPPORT_SUPPORTED && fmt.compressable() )
				format = useCompressionFormat( fmt, format, dev->tcFormatsSupported() );

			D3DFORMAT adapterFormat = dev->displayMode().Format;
			hr = dev->d3d()->CheckDeviceFormat( cp.AdapterOrdinal, cp.DeviceType, adapterFormat, 0, D3DRTYPE_TEXTURE, format );
			if ( hr == D3D_OK )
			{
				hr = d3ddev->CreateTexture( width, height, levels, usagef, format, pool, &m_tex, 0 );

				if ( hr == D3D_OK )
				{
					// store creation parameters
					m_width = width;
					m_height = height;
					m_format = toSurfaceFormat(format);
				}
			}
		} while ( hr != D3D_OK );
	}

	DWORD levelCount = m_tex->GetLevelCount();
	for ( DWORD level = 0 ; level < levelCount ; ++level )
	{
		// get texture level
		IDirect3DSurface9* dstSurface = 0;
		HRESULT hr = m_tex->GetSurfaceLevel( level, &dstSurface );
		if ( D3D_OK != hr )
		{
			error( "Failed to get texture level." );
			return;
		}

		RECT srcRect;
		srcRect.top = 0;
		srcRect.left = 0;

		if ( mipmaplevels > 0 && (int)level < mipmaplevels )
		{
			srcRect.right = surfaces[level].width();
			srcRect.bottom = surfaces[level].height();
			hr = D3DXLoadSurfaceFromMemory( dstSurface, 0, 0, surfaces[level].data(), toDx9( surfaces[level].format() ), surfaces[level].pitch(), 0, &srcRect, D3DX_FILTER_POINT, 0 ); 
		}
		else
		{
			srcRect.right = surfaces[0].width();
			srcRect.bottom = surfaces[0].height();
			hr = D3DXLoadSurfaceFromMemory( dstSurface, 0, 0, surfaces[0].data(), toDx9( surfaces[0].format() ), surfaces[0].pitch(), 0, &srcRect, D3DX_FILTER_BOX, 0 ); 
		}

		dstSurface->Release();
		if ( D3D_OK != hr )
		{
			error( "Failed to load texture: %s.", toString(hr) );
			return;	
		}
	}
}

void Dx9Texture::unload()
{
	destroyDeviceObject();
}

void Dx9Texture::destroyDeviceObject()
{
}

int Dx9Texture::create( GraphicsDevice* device, int width, int height, const SurfaceFormat& format, UsageType usage )
{
	destroy();

	Surface surfaces( width, height, format );
	load( device, &surfaces, 0, usage );

	return ERROR_NONE;
}

int Dx9Texture::create( GraphicsDevice* device, Surface* surfaces, int mipmaplevels )
{
	destroy();
	load( device, surfaces, mipmaplevels, USAGE_NORMAL );
	return ERROR_NONE;
}

bool Dx9Texture::lock( const LockMode& mode )
{
	DWORD flags = 0;
	if ( !mode.canWrite() )
		flags |= D3DLOCK_READONLY;

	D3DLOCKED_RECT rc;
	HRESULT hr = m_tex->LockRect( 0, &rc, 0, flags );
	m_data = rc.pBits;
	m_pitch = rc.Pitch;
	if ( hr != D3D_OK )
	{
		error( "Failed to lock texture surface." );
		m_data = 0;
	}

	return hr == D3D_OK;
}

void Dx9Texture::unlock()
{
	HRESULT hr = m_tex->UnlockRect( 0 );
	if ( hr != D3D_OK )
		error( "Failed to unlock texture surface." );
	m_data = 0;
}

void* Dx9Texture::data()
{
	return m_data;
}

int Dx9Texture::width() const
{
	return m_width;
}

int Dx9Texture::height() const
{
	return m_height;
}

const SurfaceFormat& Dx9Texture::format() const
{
	return m_format;
}

bool Dx9Texture::locked() const
{
	return m_data != 0;
}

const void* Dx9Texture::data() const
{
	return m_data;
}

int	Dx9Texture::pitch() const
{
	return m_pitch;
}

IDirect3DBaseTexture9* Dx9Texture::getDx9Texture( Dx9GraphicsDevice* /*dev*/ )
{
	return m_tex;
}

long Dx9Texture::textureMemoryUsed() const
{
	long bytes = 0;

	if ( m_tex )
	{
		D3DSURFACE_DESC desc;
		for ( int i = 0 ; i < (int)m_tex->GetLevelCount() ; ++i )
		{
			m_tex->GetLevelDesc( i, &desc );
			//bytes += desc.Size;
		}
	}

	return bytes;
}

int Dx9Texture::roundUpPow2( int x )
{
	int n = 1;
	while ( n < x && n+n > 0 )
		n += n;
	return n;
}

D3DFORMAT Dx9Texture::getNextBestFormat( D3DFORMAT format, int index )
{
	static const D3DFORMAT formats[][11] =
	{
		{D3DFMT_R8G8B8,		D3DFMT_X8R8G8B8, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN },
		{D3DFMT_R5G6B5,		D3DFMT_X1R5G5B5, D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_UNKNOWN},
		{D3DFMT_X1R5G5B5,	D3DFMT_X8R8G8B8, D3DFMT_R8G8B8, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN},
		{D3DFMT_X8R8G8B8,	D3DFMT_R8G8B8, D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_UNKNOWN},
		{D3DFMT_A8R8G8B8,	D3DFMT_A4R4G4B4, D3DFMT_UNKNOWN},
		{D3DFMT_A4R4G4B4,	D3DFMT_A8R8G8B8, D3DFMT_UNKNOWN},
		{D3DFMT_INDEX16,	D3DFMT_INDEX32, D3DFMT_UNKNOWN},
		{D3DFMT_DXT1,		D3DFMT_DXT1, D3DFMT_UNKNOWN},
		{D3DFMT_DXT3,		D3DFMT_DXT3, D3DFMT_UNKNOWN},
		{D3DFMT_DXT5,		D3DFMT_DXT5, D3DFMT_UNKNOWN},
		{D3DFMT_UNKNOWN}
	};

	for ( int i = 0 ; formats[i][0] != D3DFMT_UNKNOWN ; ++i )
	{
		if ( formats[i][0] == format )
		{
			for ( int k = 0 ; k < index ; ++k )
			{
				if ( formats[i][k] == D3DFMT_UNKNOWN )
					return D3DFMT_UNKNOWN;
			}
			return formats[i][index];
		}
	}

	return D3DFMT_UNKNOWN;
}

D3DFORMAT Dx9Texture::useCompressionFormat( const SurfaceFormat& textureformat, D3DFORMAT defaultformat, const Dx9GraphicsDevice::TCFormatsSupported& supported ) 
{
	if ( textureformat.getChannelBitCount( 3 ) > 1 )
	{
		if ( supported.dxt3 )
			return D3DFMT_DXT3;
		return defaultformat;
	}

	if ( supported.dxt1) 
		return D3DFMT_DXT1;

	return defaultformat;
}

gd::BaseTextureImplInterface* Dx9Texture::impl()
{
	return this;
}
