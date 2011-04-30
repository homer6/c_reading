#include "StdAfx.h"
#include "Array.h"
#include "toDx9.h"
#include "Dx9CubeTexture.h"
#include "error.h"
#include "toString.h"
#include "toSurfaceFormat.h"
#include "Dx9SurfaceLock.h"
#include "Dx9GraphicsDevice.h"
#include <gd/Errors.h>
#include <gd/LockMode.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceUtil.h>
#include <assert.h>
#include <stdio.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace gd;
using namespace pix;

//-----------------------------------------------------------------------------

Dx9CubeTexture::Dx9CubeTexture() :
	m_refs(0)
{
	m_tex			= 0;
	m_dirty			= true;
	m_locked		= false;
	m_mipMapLevels	= 0;
}

Dx9CubeTexture::~Dx9CubeTexture()
{
	destroy();
}

void Dx9CubeTexture::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx9CubeTexture::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx9CubeTexture::destroy()
{
	destroyDeviceObject();
}

void Dx9CubeTexture::load( GraphicsDevice* device )
{
	if ( !device )
		return;

	Dx9GraphicsDevice*	dev		= static_cast<Dx9GraphicsDevice*>( device );
	IDirect3DDevice9*	d3ddev	= dev->d3dDevice();
	const D3DCAPS9&		caps	= dev->caps();

	if ( !m_tex )
	{
		UINT			height	= m_imgs[0].height();
		UINT			levels	= m_mipMapLevels;
		DWORD			usage	= 0;
		SurfaceFormat	fmt		= m_imgs[0].format();
		D3DFORMAT		format	= toDx9( fmt );
		D3DPOOL			pool	= D3DPOOL_MANAGED;

		// create mipmaps?
//		if ( device->mipMapFilter() != GraphicsDevice::TEXF_NONE )
//			levels = 1;

		// 2^x aligned texture size?
		if ( caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
		{
			height = roundUpPow2(height);
		}

		// DXT textures have minimum texture size
		if ( dev->textureCompressionEnabled() && fmt.compressable() ||
			fmt.compressed() )
		{
			if ( height < 4 )
				height = 4;
		}

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
				error( "Failed to create texture: width=%i, height=%i, levels=%i, usage=0x%X, format=%s, pool=0x%X.", width, height, levels, usage, toString(originalFormat), pool );
				return;
			}
			
			if ( dev->textureCompressionEnabled() && dev->textureCompressionSupported() == gd::GraphicsDevice::TCSUPPORT_SUPPORTED && fmt.compressable() )
				format = useCompressionFormat( fmt, format, dev->tcFormatsSupported() );

			D3DFORMAT adapterFormat = dev->displayMode().Format;
			hr = dev->d3d()->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, 0, D3DRTYPE_CUBETEXTURE, format );
			if ( hr == D3D_OK )
			{
				hr = d3ddev->CreateCubeTexture( height, levels, usage, format, pool, &m_tex, 0 );
			}
		} while ( hr != D3D_OK );
		
		m_dirty = true;
	}

	if ( m_dirty )
	{
		for ( int f = D3DCUBEMAP_FACE_POSITIVE_X; f <= D3DCUBEMAP_FACE_NEGATIVE_Z; ++f )
		{
			DWORD levelCount = m_tex->GetLevelCount();
			for ( DWORD level = 0 ; level < levelCount ; ++level )
			{
				// get texture level
				IDirect3DSurface9* dstSurface = 0;
				HRESULT hr = m_tex->GetCubeMapSurface( (D3DCUBEMAP_FACES)f, level, &dstSurface );
				if ( D3D_OK != hr )
				{
					error( "Failed to get texture level." );
					return;
				}

				RECT srcRect;
				srcRect.top = 0;
				srcRect.left = 0;
				if ( m_mipMapLevels > 0 )
				{
					int ndx = (f-D3DCUBEMAP_FACE_POSITIVE_X) * m_mipMapLevels + level;
					assert( ndx < m_imgs.size() );
					
					srcRect.right = m_imgs[ndx].width();
					srcRect.bottom = m_imgs[ndx].height();
					hr = D3DXLoadSurfaceFromMemory( dstSurface, 0, 0, m_imgs[ndx].data(), toDx9( m_imgs[ndx].format() ), m_imgs[ndx].pitch(), 0, &srcRect, D3DX_FILTER_POINT, 0 ); 
				}
				else
				{
					srcRect.right = m_imgs[0].width();
					srcRect.bottom = m_imgs[0].height();
					hr = D3DXLoadSurfaceFromMemory( dstSurface, 0, 0, m_imgs[f].data(), toDx9( m_imgs[f].format() ), m_imgs[f].pitch(), 0, &srcRect, D3DX_FILTER_BOX, 0 ); 
				}

				dstSurface->Release();
				if ( D3D_OK != hr )
				{
					error( "Failed to load texture: %s.", toString(hr) );
					return;	
				}
			}
		}
		m_dirty = false;
	}
}

void Dx9CubeTexture::unload()
{
	destroyDeviceObject();
}

void Dx9CubeTexture::destroyDeviceObject()
{
	if ( m_tex ) 
	{
		m_tex->Release();
		m_tex = 0;
		m_dirty = true;
	}
}

int Dx9CubeTexture::create( int edgelength, const SurfaceFormat& format )
{
	destroy();
	m_imgs.setSize(6);
	for ( int i = 0; i < 6; ++i )
		m_imgs[i].create( edgelength, edgelength, format );

	m_dirty			= true;
	m_locked		= false;
	m_mipMapLevels	= 0;

	return ERROR_NONE;
}

int Dx9CubeTexture::create( pix::Surface* surfaces, int mipmaplevels )
{
	destroy();
	if ( mipmaplevels > 0 )
	{
		m_imgs.setSize(6 * mipmaplevels);
		for ( int i = 0; i < 6 * mipmaplevels; ++i )
			m_imgs[i].swap(surfaces[i]);	
	}
	else
	{
		m_imgs.setSize(6);
		for ( int i = 0; i < 6; ++i )
			m_imgs[i].swap(surfaces[i]);	
	}

	m_dirty			= true;
	m_locked		= false;
	m_mipMapLevels	= mipmaplevels;	

	return ERROR_NONE;
}

bool Dx9CubeTexture::lock( const LockMode& mode, int subsurface )
{
	assert( subsurface >= 0 && subsurface < m_imgs.size() ); subsurface=subsurface;

	if ( mode.canWrite() )
		m_dirty = true;
	
	m_locked = true;
	return true;
}

void Dx9CubeTexture::unlock( int subsurface )
{
	assert( subsurface >= 0 && subsurface < m_imgs.size() ); subsurface=subsurface;
	m_locked = false;
}

void* Dx9CubeTexture::data( int subsurface)
{
	assert( subsurface >= 0 && subsurface < m_imgs.size() ); subsurface=subsurface;
	return m_imgs[subsurface].data();
}

int Dx9CubeTexture::width() const
{
	return m_imgs[0].width();
}

int Dx9CubeTexture::height() const
{
	return m_imgs[0].height();
}

const SurfaceFormat& Dx9CubeTexture::format() const
{
	return m_imgs[0].format();
}

bool Dx9CubeTexture::locked( int subsurface ) const
{
	assert( subsurface >= 0 && subsurface < m_imgs.size() ); subsurface=subsurface;
	return m_locked;
}

const void* Dx9CubeTexture::data(int subsurface) const
{
	assert( m_locked );
	const Surface& surf = m_imgs[subsurface];
	return surf.data();
}

int	Dx9CubeTexture::pitch() const
{
	return m_imgs[0].pitch();
}

IDirect3DBaseTexture9* Dx9CubeTexture::getDx9Texture( Dx9GraphicsDevice* dev )
{
	if ( m_dirty )
		load( dev );

	return m_tex;
}

long Dx9CubeTexture::textureMemoryUsed() const
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

bool Dx9CubeTexture::dirty() const
{
	return m_dirty;
}

int Dx9CubeTexture::roundUpPow2( int x )
{
	int n = 1;
	while ( n < x && n+n > 0 )
		n += n;
	return n;
}

D3DFORMAT Dx9CubeTexture::getNextBestFormat( D3DFORMAT format, int index )
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

D3DFORMAT Dx9CubeTexture::useCompressionFormat( const SurfaceFormat& textureformat, D3DFORMAT defaultformat, const Dx9GraphicsDevice::TCFormatsSupported& supported ) 
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

gd::BaseTextureImplInterface* Dx9CubeTexture::impl()
{
	return this;
}
