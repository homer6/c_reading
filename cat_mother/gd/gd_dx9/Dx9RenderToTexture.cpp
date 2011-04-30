#include "StdAfx.h"
#include "Dx9RenderToTexture.h"
#include "Dx9GraphicsDevice.h"
#include "Dx9Texture.h"
#include "toString.h"
#include "error.h"
#include "toDx9.h"
#include "config.h"

//-----------------------------------------------------------------------------

Dx9RenderToTexture::Dx9RenderToTexture() :
	m_refs(0),
	m_rts(0),
	m_dev(0)
{
}

Dx9RenderToTexture::~Dx9RenderToTexture()
{
	destroy();
}

void Dx9RenderToTexture::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx9RenderToTexture::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

int Dx9RenderToTexture::create( gd::GraphicsDevice* device, int width, int height, const pix::SurfaceFormat& format, bool depthstencil, const pix::SurfaceFormat& depthformat )
{
	Dx9GraphicsDevice*	dev			= static_cast<Dx9GraphicsDevice*>( device );
	IDirect3DDevice9*	d3ddev		= dev->d3dDevice();
	D3DFORMAT			d3dformat	= toDx9(format);
	D3DFORMAT			d3ddepth	= toDx9(depthformat);
	
	m_dev = dev;
	HRESULT hr = D3DXCreateRenderToSurface( d3ddev, width, height, d3dformat, depthstencil, toDx9(depthformat), &m_rts );
	if ( hr != D3D_OK )
		error( "Failed to create render-to-texture: %s, width=%i, height=%i, format=%s, depthstencil=%i, depthformat=%s", toString(hr), width, height, toString(d3dformat), (int)depthstencil, toString(d3ddepth) );

	return hr != D3D_OK;
}

void Dx9RenderToTexture::destroy()
{
	if ( m_rts )
	{
		m_rts->Release();
		m_rts = 0;
	}

	m_dev = 0;
}

void Dx9RenderToTexture::beginScene( gd::Texture* texture, int x, int y, int width, int height )
{
	Dx9Texture* tex = static_cast<Dx9Texture*>( texture );
	IDirect3DTexture9* d3dtex = tex->d3dTexture();
	
	IDirect3DSurface9* sf = 0;
	HRESULT hr = d3dtex->GetSurfaceLevel( 0, &sf );
	if ( hr != D3D_OK )
	{
		error( "Failed to beginScene on render-to-texture (GetSurfaceLevel failed): %s", toString(hr) );
		return;
	}

	D3DVIEWPORT9 vp;
	vp.X = x;
	vp.Y = y;
	vp.Width = width;
	vp.Height = height;
	vp.MinZ = 0.f;
	vp.MaxZ = 1.f;

	hr = m_rts->BeginScene( sf, &vp );
	sf->Release();
	if ( hr != D3D_OK )
	{
		error( "Failed to beginScene on render-to-texture (RTS:BeginScene failed, vp=x=%i y=%i h=%i w=%i): %s", toString(hr), x, y, width, height );
		return;
	}
}

void Dx9RenderToTexture::endScene( gd::GraphicsDevice::TextureFilterType mipfilter )
{
	HRESULT hr = m_rts->EndScene( toDx9(mipfilter) );
	if ( hr != D3D_OK )
	{
		error( "Failed to beginScene on render-to-texture (GetSurfaceLevel failed): %s", toString(hr) );
		return;
	}
}

void Dx9RenderToTexture::resetDeviceObject()
{
	m_rts->OnResetDevice();
}

void Dx9RenderToTexture::destroyDeviceObject()
{
	m_rts->OnLostDevice();
}
