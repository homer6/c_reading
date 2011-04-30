#include "StdAfx.h"
#include "Dx9GraphicsDevice.h"
#include "Dx9Material.h"
#include "Dx9Texture.h"
#include "Dx9CubeTexture.h"
#include "Dx9BaseTextureImplInterface.h"
#include "toSurfaceFormat.h"
#include "toString.h"
#include "toDx9.h"
#include "zero.h"
#include "error.h"
#include "LogFile.h"
#include <gd/LockMode.h>
#include <gd/Errors.h>
#include <assert.h>
#include <memory.h>
#include <dxerr8.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace gd;
using namespace pix;
using namespace math;

//-----------------------------------------------------------------------------

static void getClientSize( HWND hwnd, int* w, int* h )
{
	if ( !hwnd )
		hwnd = GetActiveWindow();

	RECT cr = {0,0,0,0};
	if ( hwnd )
		GetClientRect( hwnd, &cr );

	*w = (int)cr.right;
	*h = (int)cr.bottom;
}

static DWORD floatToInt32( float v )
{
	assert( sizeof(float) == sizeof(DWORD) );
	return *reinterpret_cast<DWORD*>( &v );
}

static D3DFORMAT findDepthBuffer( D3DFORMAT* depthFormats, IDirect3D9* d3d, D3DFORMAT displayMode, D3DDEVTYPE devtype )
{
	for ( int i = 0 ; D3DFORMAT(-1) != depthFormats[i] ; ++i )
	{
		HRESULT hr = d3d->CheckDeviceFormat( 
			D3DADAPTER_DEFAULT, devtype,
			displayMode,
			D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE,
			depthFormats[i] );

		if ( D3D_OK == hr )
			return depthFormats[i];
	}
	return D3DFMT_UNKNOWN;
}

//-----------------------------------------------------------------------------

Dx9GraphicsDevice::Dx9GraphicsDevice() :
	m_refs(0)
{
	defaults();
}

Dx9GraphicsDevice::~Dx9GraphicsDevice()
{
	destroy();
}

void Dx9GraphicsDevice::destroy()
{
	assert( !m_lockedBackBuffer );

	DrvObject::destroyAllDeviceObjects();

	if ( m_device )
	{
		m_device->Release();
		m_device = 0;
	}

	if ( m_d3d )
	{
		m_d3d->Release();
		m_d3d = 0;
	}

	defaults();

	// profiling statistics
	/*Debug::println( "gd_d3d8 performance statistics:" );
	for ( int k = 0 ; k < Profile::count() ; ++k )
	{
		Profile::BlockInfo* b = Profile::get( k );
		Debug::println( "{0} (x{2,#}): {1} ms", b->name(), b->time()*1e3f, b->count() );
	}*/
}

void Dx9GraphicsDevice::flushDeviceObjects()
{
	if ( m_device )
		m_device->EvictManagedResources();
}

void Dx9GraphicsDevice::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx9GraphicsDevice::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

int Dx9GraphicsDevice::create( int width, int height, int bitsPerPixel, int refreshRate, WindowType win, RasterizerType rz, VertexProcessingType vp, int buffers, TextureCompression tc )
{
	destroy();

	LogFile logfile( "gd_dx9.log" );
	logfile.printf( "Initializing rendering device: %ix%i, %ibits/pixel", width, height, bitsPerPixel );

	m_buffers = buffers;

	HWND		hwnd			= GetActiveWindow();
	bool		fullscreen		= (WINDOW_FULLSCREEN == win);
	D3DDEVTYPE	devtype			= toDx9( rz );

	// check that we have active window
	logfile.printf( "Checking for active window" );
	if ( !hwnd )
	{
		logfile.printf( "ERROR: No active window" );
		return ERROR_NOACTIVEWINDOW;
	}
	logfile.printf( "ok" );

	// create Direct3D8 object
	logfile.printf( "Direct3DCreate9" );
	m_d3d = Direct3DCreate9( D3D_SDK_VERSION );
	if ( !m_d3d )
	{
		error( "ERROR_DIRECTXNOTINSTALLED" );
		logfile.printf( "ERROR: DirectX 9 not installed" );
		return ERROR_DIRECTXNOTINSTALLED;
	}
	m_d3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &m_displaymode );
	logfile.printf( "ok" );

	// find matching display mode
	logfile.printf( "Find matching display mode" );
	if ( fullscreen )
	{
		D3DFORMAT allowedModes[] = {D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_A2B10G10R10, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5, D3DFMT_R5G6B5};
		const int allowedModeCount = sizeof(allowedModes)/sizeof(allowedModes[0]);
		bool modeFound = false;

		for ( int k = 0 ; k < allowedModeCount && !modeFound ; ++k )
		{
			HRESULT hr = D3D_OK;
			for ( int i = 0 ; D3D_OK == hr ; ++i )
			{
				hr = m_d3d->EnumAdapterModes( D3DADAPTER_DEFAULT, allowedModes[k], i, &m_displaymode );

				logfile.printf( "  Comparing %ix%i", m_displaymode.Width, m_displaymode.Height );
				if ( hr == D3D_OK )
				{
					if ( (int)m_displaymode.Width == width && 
						(int)m_displaymode.Height == height )
					{
						SurfaceFormat pixFmt = toSurfaceFormat( m_displaymode.Format );
						if ( pixFmt.pixelSize() == bitsPerPixel/8 )
						{
							modeFound = true;
							break;
						}
					}
				}
			}
		}

		if ( !modeFound )
		{
			logfile.printf( "ERROR: Matching display mode not found" );
			return ERROR_FULLSCREENINITFAILED;
		}
	}
	logfile.printf( "ok" );

	// find depth buffer format
	logfile.printf( "Find depth buffer format" );
	D3DFORMAT depthFormat = D3DFMT_UNKNOWN;
	if ( SURFACE_DEPTH & buffers )
	{
		D3DFORMAT depthFormats[] =
		{
			D3DFMT_D32, D3DFMT_D24S8, D3DFMT_D16, D3DFMT_D24X8,
			D3DFMT_D24X4S4, D3DFMT_D16, D3DFMT_D15S1,
			D3DFORMAT(-1)
		};
		D3DFORMAT depthStencilFormats[] =
		{
			D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D15S1,
			D3DFORMAT(-1)
		};
		
		if ( buffers & SURFACE_STENCIL )
			depthFormat = findDepthBuffer( depthStencilFormats, m_d3d, m_displaymode.Format, devtype );
		else
			depthFormat = findDepthBuffer( depthFormats, m_d3d, m_displaymode.Format, devtype );

		if ( depthFormat == D3DFMT_UNKNOWN )
		{
			m_d3d->Release();
			m_d3d = 0;

			if ( buffers & SURFACE_STENCIL )
			{
				logfile.printf( "ERROR: No stencil buffer" );
				error( "ERROR_NOSTENCIL" );
				return ERROR_NOSTENCIL;
			}
			else
			{
				logfile.printf( "ERROR: No depth buffer" );
				error( "ERROR_NODEPTH" );
				return ERROR_NODEPTH;
			}
		}
	}
	else if ( SURFACE_STENCIL & buffers )
	{
		logfile.printf( "ERROR: No stencil without depth" );
		error( "ERROR_NOSTENCILWITHOUTSDEPTH" );
		return ERROR_NOSTENCILWITHOUTSDEPTH;
	}
	logfile.printf( "ok" );

	// set up presentation parameters
	zero( m_present );
	m_present.hDeviceWindow = hwnd;
	m_present.BackBufferFormat = m_displaymode.Format;
	m_present.BackBufferCount = 1;
	m_present.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_present.EnableAutoDepthStencil = ( 0 != (SURFACE_DEPTH & buffers) );
	m_present.AutoDepthStencilFormat = depthFormat;
	m_present.Windowed = !fullscreen;
	m_present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	if ( fullscreen )
	{
		//m_present.FullScreen_RefreshRateInHz = refreshRate;
		m_present.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		m_present.BackBufferWidth = width;
		m_present.BackBufferHeight = height;
	}
	//m_present.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	//m_present.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	// set up device creation flags
	DWORD deviceFlags = 0;
	D3DCAPS9 devcaps;
	m_d3d->GetDeviceCaps( D3DADAPTER_DEFAULT, devtype, &devcaps );
	if ( vp != VERTEXP_SW &&
		0 != (D3DDEVCAPS_HWTRANSFORMANDLIGHT  & devcaps.DevCaps) )
	{
		if ( vp == VERTEXP_HW )
		{
			deviceFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
			m_vp = VERTEXP_HW;
		}
		else 
		{
			deviceFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
			m_vp = VERTEXP_MIXED;
		}
	}
	else
	{
		deviceFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		m_vp = VERTEXP_SW;
	}

	// create device
	logfile.printf( "Creating device object" );
	HRESULT hr = m_d3d->CreateDevice( D3DADAPTER_DEFAULT, devtype, hwnd, deviceFlags, &m_present, &m_device );

	if ( D3D_OK != hr )
	{
		m_d3d->Release();
		m_d3d = 0;

		if ( fullscreen )
		{
			logfile.printf( "ERROR: Full screen init failed: %s", toString(hr) );
			error( "ERROR_FULLSCREENINITFAILED: %s", toString(hr) );
			return ERROR_FULLSCREENINITFAILED;
		}
		else
		{
			logfile.printf( "ERROR: Desktop init failed: %s", toString(hr) );
			error( "ERROR_DESKTOPINITFAILED: %s", toString(hr) );
			return ERROR_DESKTOPINITFAILED;
		}
	}

	// store device capabilities
	m_device->GetDeviceCaps( &m_caps );

	// DEBUG: print some rendering device caps
	message( "Direct3D rendering device caps:" );
	message( "  PixelShaderVersion = %i.%i", (int)((m_caps.PixelShaderVersion & 0xFF00)>>8), (int)((m_caps.PixelShaderVersion & 0xFF)>>0) );
	message( "  VertexShaderVersion = %i.%i", (int)((m_caps.VertexShaderVersion & 0xFF00)>>8), (int)((m_caps.VertexShaderVersion & 0xFF)>>0) );

	// store size of rendering area
	if ( m_present.Windowed )
	{
		getClientSize( m_present.hDeviceWindow, &m_renderWidth, &m_renderHeight );
	}
	else
	{
		m_renderWidth = m_present.BackBufferWidth;
		m_renderHeight = m_present.BackBufferHeight;
	}

	setFog( FOG_NONE );
	setFogColor( Color(0,0,0) );
	setFogDensity( 0.5f );
	setFogStart( 0.f );
	setFogEnd( 1.f );

	// check for texture compression support
	m_textureCompressionSupport = TCSUPPORT_NONE;

	m_tcFormatsSupported.uyvy = textureFormatSupported( D3DFMT_UYVY );
	m_tcFormatsSupported.yuy2 = textureFormatSupported( D3DFMT_YUY2 );
	m_tcFormatsSupported.dxt1 = textureFormatSupported( D3DFMT_DXT1 );
	m_tcFormatsSupported.dxt2 = textureFormatSupported( D3DFMT_DXT2 );
	m_tcFormatsSupported.dxt3 = textureFormatSupported( D3DFMT_DXT3 );
	m_tcFormatsSupported.dxt4 = textureFormatSupported( D3DFMT_DXT4 );
	m_tcFormatsSupported.dxt5 = textureFormatSupported( D3DFMT_DXT5 );

	if ( m_tcFormatsSupported.anyFormatSupported() ) 
		m_textureCompressionSupport = TCSUPPORT_SUPPORTED;

	if ( tc == TC_COMPRESSED )
		if ( m_textureCompressionSupport == TCSUPPORT_SUPPORTED )
			m_useTextureCompression = tc;
		else
			m_useTextureCompression = TC_NONE;

	logfile.printf( "Device init ok" );
	return ERROR_NONE;
}

void Dx9GraphicsDevice::setAmbient( const Color& ambientLight )
{
	setRenderState( D3DRS_AMBIENT, ambientLight.toInt32() );
}

void Dx9GraphicsDevice::addLight( const gd::LightState& lightDesc )
{
	if ( (DWORD)m_lights < m_caps.MaxActiveLights )
	{
		D3DLIGHT9 d3dlight;
		toDx9( lightDesc, d3dlight );
		DWORD index = m_lights;
		HRESULT hr = m_device->SetLight( index, &d3dlight );

		if ( D3D_OK != hr )
			error( "Failed to set a light source to the rendering device" );
		hr = m_device->LightEnable( index, TRUE );
		if ( D3D_OK != hr )
			error( "Failed to enable a light source to the rendering device" );

		m_lights++;
	}
	else
	{
		// too many lights for the device, ignore rest
	}
}

void Dx9GraphicsDevice::removeLights()
{
	for ( ; m_lights > 0 ; --m_lights )
	{
		DWORD index = m_lights-1;
		m_device->LightEnable( index, FALSE );
	}
}

void Dx9GraphicsDevice::beginScene()
{
	HRESULT hr = m_device->BeginScene();
	if ( D3D_OK != hr )
		error( "Failed to begin rendering: %s", toString(hr) );

	m_sceneInProgress = true;

	resetRenderState();
}

void Dx9GraphicsDevice::endScene()
{
	if ( m_sceneInProgress )
	{
		m_sceneInProgress = false;
		m_device->EndScene();
	}
}

bool Dx9GraphicsDevice::ready() const
{
	bool ok = false;

	if ( m_device )
	{
		HRESULT hr = m_device->TestCooperativeLevel();
		ok = (D3D_OK == hr);
	}

	return ok;
}

bool Dx9GraphicsDevice::restore()
{
	bool ok = false;

	if ( m_device )
	{
		HRESULT hr = m_device->TestCooperativeLevel();
		ok = (D3D_OK == hr);

		switch ( hr )
		{
		case D3DERR_DEVICELOST:		break;
		case D3DERR_DEVICENOTRESET:	DrvObject::destroyAllDeviceObjects();
									hr = m_device->Reset( &m_present );
									ok = (D3D_OK == hr);
									if ( ok )
										DrvObject::resetAllDeviceObjects();
									break;
		}
	}

	return ok;
}

void Dx9GraphicsDevice::present()
{
	HRESULT hr = m_device->Present( 0, 0, 0, 0 );
	
	if ( D3DERR_DEVICELOST == hr )
		message( "Dx9GraphicsDevice.present(): Device lost" );
}

void Dx9GraphicsDevice::setWorldTransform( const math::Matrix4x4& modelToWorld )
{
	D3DMATRIX d3dm;
	toDx9( modelToWorld, d3dm );

	HRESULT hr = m_device->SetTransform( D3DTS_WORLDMATRIX(0), &d3dm );
	if ( hr != D3D_OK )
		error( "Failed to set world transform: %s", toString(hr) );

	m_worldTM[0] = modelToWorld;
}

void Dx9GraphicsDevice::setWorldTransform( int index, const math::Matrix4x4& modelToWorld )
{
	D3DMATRIX d3dm;
	toDx9( modelToWorld, d3dm );

	HRESULT hr = m_device->SetTransform( D3DTS_WORLDMATRIX(index), &d3dm );
	if ( hr != D3D_OK )
		error( "Failed to set bone world transform: %s", toString(hr) );

	if ( m_worldTM.size() <= index )
	{
		m_worldTM.setSize( index+1 );
	}
	m_worldTM[index] = modelToWorld;
}

void Dx9GraphicsDevice::setViewTransform( const math::Matrix4x4& worldToView )
{
	D3DMATRIX d3dm;
	toDx9( worldToView, d3dm );
	HRESULT hr = m_device->SetTransform( D3DTS_VIEW, &d3dm );
	if ( hr != D3D_OK )
		error( "Failed to set view transform: %s", toString(hr) );
}

void Dx9GraphicsDevice::setProjectionTransform( const math::Matrix4x4& cameraToScreen )
{
	D3DMATRIX d3dm;
	toDx9( cameraToScreen, d3dm );
	HRESULT hr = m_device->SetTransform( D3DTS_PROJECTION, &d3dm );
	if ( hr != D3D_OK )
		error( "Failed to set projection transform: %s", toString(hr) );
}

void Dx9GraphicsDevice::setViewport( int x, int y, int width, int height )
{
	D3DVIEWPORT9 vp;
	vp.X = x;
	vp.Y = y;
	vp.Width = width;
	vp.Height = height;
	vp.MinZ = 0.f;
	vp.MaxZ = 1.f;
	HRESULT hr = m_device->SetViewport( &vp );
	if ( hr != D3D_OK )
		error( "Failed to set viewport: %s", toString(hr) );
}

void Dx9GraphicsDevice::clear( int flags, const Color& color, int stencil )
{
	DWORD d3dflags = 0;
	if ( SURFACE_TARGET & flags )
		d3dflags |= D3DCLEAR_TARGET;
	if ( SURFACE_DEPTH & flags && 0 != (SURFACE_DEPTH & m_buffers) )
		d3dflags |= D3DCLEAR_ZBUFFER;
	if ( SURFACE_STENCIL & flags && 0 != (SURFACE_STENCIL & m_buffers) )
		d3dflags |= D3DCLEAR_STENCIL;

	HRESULT hr = m_device->Clear( 0, 0, d3dflags, color.toInt32(), 1.f, stencil );
	if ( hr != D3D_OK )
		error( "Failed to clear viewport: %s", toString(hr) );
}

void Dx9GraphicsDevice::setClipping( bool enabled )
{
	setRenderState( D3DRS_CLIPPING, (enabled ? TRUE : FALSE) );
}

int Dx9GraphicsDevice::width() const
{
	return m_renderWidth;
}

int Dx9GraphicsDevice::height() const
{
	return m_renderHeight;
}

bool Dx9GraphicsDevice::sceneInProgress() const
{
	return m_sceneInProgress;
}

bool Dx9GraphicsDevice::fullscreen() const
{
	return !m_present.Windowed;
}

bool Dx9GraphicsDevice::clipping() const
{
	return 0 != getRenderState(D3DRS_CLIPPING);
}

void Dx9GraphicsDevice::setRenderState( D3DRENDERSTATETYPE state, DWORD value )
{
	HRESULT hr = m_device->SetRenderState( state, value );
	if ( D3D_OK != hr )
		error( "Failed to set render state: %s", toString(hr) );
}

DWORD Dx9GraphicsDevice::getRenderState( D3DRENDERSTATETYPE state ) const
{
	DWORD value;
	HRESULT hr = m_device->GetRenderState( state, &value );
	if ( D3D_OK != hr )
		error( "Failed to get render state: %s", toString(hr) );
	return value;
}

void Dx9GraphicsDevice::getWorldTransform( math::Matrix4x4* modelToWorld ) const
{
	*modelToWorld = m_worldTM[0];
}

const math::Matrix4x4* Dx9GraphicsDevice::worldTransforms() const
{
	return &m_worldTM[0];
}

int Dx9GraphicsDevice::worldTransformCount() const
{
	return m_worldTM.size();
}

void Dx9GraphicsDevice::getViewTransform( math::Matrix4x4* worldToView ) const
{
	D3DMATRIX d3dm;
	HRESULT hr = m_device->GetTransform( D3DTS_VIEW, &d3dm );
	fromDx9( d3dm, *worldToView );
	if ( hr != D3D_OK )
		error( "Failed to get view transform: %s", toString(hr) );
}

void Dx9GraphicsDevice::getProjectionTransform( math::Matrix4x4* cameraToScreen ) const
{
	D3DMATRIX d3dm;
	HRESULT hr = m_device->GetTransform( D3DTS_PROJECTION, &d3dm );
	fromDx9( d3dm, *cameraToScreen );
	if ( hr != D3D_OK )
		error( "Failed to get projection transform: %s", toString(hr) );
}

void Dx9GraphicsDevice::updateStatistics( int triangles )
{
	m_renderedPrimitives += 1;
	m_renderedTriangles += triangles;
}

void Dx9GraphicsDevice::resetStatistics()
{
	m_lockedIndices			= 0;
	m_lockedVertices		= 0;
	m_renderedPrimitives	= 0;
	m_renderedTriangles		= 0;
}

int* Dx9GraphicsDevice::getTemporaryIntegerBuffer( int capacity )
{
	assert( capacity > 0 );

	m_intBuffer.setSize( capacity );
	return &m_intBuffer[0];
}

int Dx9GraphicsDevice::renderedPrimitives() const
{
	return m_renderedPrimitives;
}

int Dx9GraphicsDevice::renderedTriangles() const
{
	return m_renderedTriangles;
}

int Dx9GraphicsDevice::materialChanges() const
{
	return m_materialChanges;
}

long Dx9GraphicsDevice::textureMemoryUsed() const
{
	return DrvObject::totalTextureMemoryUsed();
}

bool Dx9GraphicsDevice::lockBackBuffer( void** surface, int* pitch )
{
	assert( !m_lockedBackBuffer );

	m_lockedBackBuffer = 0;
	HRESULT hr = m_device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &m_lockedBackBuffer );
	if ( D3D_OK != hr )
	{
		error( "Failed to get back buffer for locking: %s", toString(hr) );
		return false;
	}

	D3DLOCKED_RECT lr;
	hr = m_lockedBackBuffer->LockRect( &lr, 0, D3DLOCK_READONLY );
	if ( D3D_OK != hr )
	{
		m_lockedBackBuffer->Release();
		m_lockedBackBuffer = 0;
		error( "Failed to lock back buffer: %s", toString(hr) );
		return false;
	}

	*surface	= lr.pBits;
	*pitch		= lr.Pitch;
	return true;
}

void Dx9GraphicsDevice::unlockBackBuffer()
{
	assert( m_lockedBackBuffer );

	if ( m_lockedBackBuffer )
	{
		m_lockedBackBuffer->UnlockRect();
		m_lockedBackBuffer->Release();
		m_lockedBackBuffer = 0;
	}
}

void Dx9GraphicsDevice::getFormat( SurfaceFormat* fmt ) const
{
	IDirect3DSurface9* back = 0;
	HRESULT hr = m_device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &back );
	if ( D3D_OK != hr )
		error( "Failed to get back buffer pixel format: %s", toString(hr) );

	D3DSURFACE_DESC desc;
	back->GetDesc( &desc );
	*fmt = fromDx9( desc.Format );
	back->Release();
}

void Dx9GraphicsDevice::setFog( FogMode mode )
{
	m_fog = mode;
	
	if ( mode != FOG_NONE && fogSupported() )
	{
		HRESULT hr = m_device->SetRenderState( D3DRS_FOGENABLE, TRUE );
		if ( D3D_OK != hr )
			error( "Failed to enable rendering device fog: %s", toString(hr) );

		hr = m_device->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
		if ( D3D_OK != hr )
			error( "Failed to enable rendering device fog type: %s", toString(hr) );

		D3DFOGMODE d3dmode = toDx9( mode );
		hr = m_device->SetRenderState( D3DRS_FOGVERTEXMODE, d3dmode );
		if ( D3D_OK != hr )
			error( "Failed to enable rendering device fog mode: %s", toString(hr) );
	}
	else
	{
		m_device->SetRenderState( D3DRS_FOGENABLE, FALSE );
	}
}

void Dx9GraphicsDevice::setFogColor( const Color& color )
{
	m_fogColor = color;

	if ( fogSupported() )
	{
		HRESULT hr = m_device->SetRenderState( D3DRS_FOGCOLOR, color.toInt32() );
		if ( D3D_OK != hr )
			error( "Failed to enable rendering device fog color: %s", toString(hr) );
	}
}

void Dx9GraphicsDevice::setFogStart( float start )
{
	m_fogStart = start;

	if ( fogSupported() )
	{
		HRESULT hr = m_device->SetRenderState( D3DRS_FOGSTART, floatToInt32(m_fogStart) );
		if ( D3D_OK != hr )
			error( "Failed to enable rendering device fog start: %s", toString(hr) );
	}
}

void Dx9GraphicsDevice::setFogEnd( float end )
{
	m_fogEnd = end;

	if ( fogSupported() )
	{
		HRESULT hr = m_device->SetRenderState( D3DRS_FOGEND, floatToInt32(m_fogEnd) );
		if ( D3D_OK != hr )
			error( "Failed to enable rendering device fog end: %s", toString(hr) );
	}
}

void Dx9GraphicsDevice::setFogDensity( float density )
{
	m_fogDensity = density;

	if ( fogSupported() )
	{
		HRESULT hr = m_device->SetRenderState( D3DRS_FOGDENSITY, floatToInt32(m_fogDensity) );
		if ( D3D_OK != hr )
			error( "Failed to enable rendering device fog density: %s", toString(hr) );
	}
}

void Dx9GraphicsDevice::setMipMapFilter( Dx9GraphicsDevice::TextureFilterType mode ) 
{
	m_mipMapFilter = mode;

	if ( m_caps.TextureCaps & D3DPTEXTURECAPS_MIPMAP )
	{
		D3DTEXTUREFILTERTYPE d3dfilt = toDx9( mode );
		for ( DWORD i = 0 ; i < m_caps.MaxTextureBlendStages ; ++i )
			m_device->SetSamplerState( i, D3DSAMP_MIPFILTER, d3dfilt );
	}
}

void Dx9GraphicsDevice::setMipMapLODBias( float bias ) 
{
	m_mipMapLODBias = bias;

	if ( m_caps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS )
	{
		DWORD d3dbias = *reinterpret_cast<DWORD*>(&bias);
		for ( DWORD i = 0 ; i < m_caps.MaxTextureBlendStages ; ++i )
			m_device->SetSamplerState( i, D3DSAMP_MIPMAPLODBIAS, d3dbias );
	}
}

Dx9GraphicsDevice::FogMode Dx9GraphicsDevice::fog() const
{
	return m_fog;
}

const Color& Dx9GraphicsDevice::fogColor() const
{
	return m_fogColor;
}

float Dx9GraphicsDevice::fogStart() const
{
	return m_fogStart;
}

float Dx9GraphicsDevice::fogEnd() const
{
	return m_fogEnd;
}

float Dx9GraphicsDevice::fogDensity() const
{
	return m_fogDensity;
}

bool Dx9GraphicsDevice::stencil() const
{
	return 0 != (SURFACE_STENCIL & m_buffers);
}

void Dx9GraphicsDevice::destroyDeviceObject()
{
	// All textures, shaders and geometry must be cleaned up first
	// before we delete d3d device -- see destroy() --
	// so we dont do anything yet in this point
}

void Dx9GraphicsDevice::defaults()
{
	m_d3d					= 0;
	m_device				= 0;
	m_lights				= 0;
	m_sceneInProgress		= false;
	zero( m_displaymode );
	zero( m_caps );
	zero( m_present );
	m_buffers				= 0;
	m_renderedTriangles		= 0;
	m_renderedPrimitives	= 0;
	m_lockedIndices			= 0;
	m_lockedVertices		= 0;
	m_materialChanges		= 0;
	m_renderWidth			= 0;
	m_renderHeight			= 0;
	m_lockedBackBuffer		= 0;
	m_renderEnabled			= true;
	m_vp					= (VertexProcessingType)-1;
	m_rs.invalidate();
	m_fogColor				= Color(0,0,0);
	m_fogStart				= 0.f;
	m_fogEnd				= 0.f;
	m_fogDensity			= 0.f;
	m_fog					= FOG_NONE;
	m_mipMapFilter			= TEXF_NONE;
	m_mipMapLODBias			= 0.f;
	m_worldTM.setSize( 1 );
}

void Dx9GraphicsDevice::resetRenderState()
{
	m_rs.invalidate();
	setRenderState( m_defaultRS );

	setMipMapFilter( m_mipMapFilter );
	setMipMapLODBias( m_mipMapLODBias );
	setFog( m_fog );
	setFogStart( m_fogStart );
	setFogEnd( m_fogEnd );
	setFogDensity( m_fogDensity );

	for ( int i = 0 ; i < m_lights ; ++i )
		m_device->LightEnable( i, FALSE );
}

void Dx9GraphicsDevice::setDefaultRenderState()
{
	m_rs.invalidate();
	setRenderState( m_defaultRS );
}

Dx9GraphicsDevice::TextureFilterType Dx9GraphicsDevice::mipMapFilter() const
{
	return m_mipMapFilter;
}

bool Dx9GraphicsDevice::validate()
{
	assert( m_device );
	
	DWORD numPasses = 0;
	HRESULT hr = m_device->ValidateDevice( &numPasses );
	switch ( hr )
	{
	case D3D_OK:	break;
	default:		error( "ValidateDevice failed: %s", toString(hr) );
	}

	return hr == D3D_OK;
}

void Dx9GraphicsDevice::updateLockStatistics( int vertices, int indices )
{
	m_lockedIndices += indices;
	m_lockedVertices += vertices;
}

int Dx9GraphicsDevice::lockedIndices() const
{
	return m_lockedIndices;
}

int Dx9GraphicsDevice::lockedVertices() const
{
	return m_lockedVertices;
}

bool Dx9GraphicsDevice::textureFormatSupported( D3DFORMAT format ) const 
{
	assert( m_d3d );
	assert( m_device );

	D3DDEVICE_CREATION_PARAMETERS cp;
	m_device->GetCreationParameters( &cp );

	HRESULT hr = m_d3d->CheckDeviceFormat( cp.AdapterOrdinal, cp.DeviceType, m_displaymode.Format, 0, D3DRTYPE_TEXTURE, format );

	return SUCCEEDED( hr );
}

void Dx9GraphicsDevice::setRenderState( const Dx9RenderingState& rs )
{
	assert( &rs != &m_rs );

	IDirect3DDevice9*	device	= m_device;
	const D3DCAPS9&		caps	= m_caps;
	Dx9RenderingState&	cur		= m_rs;

	// set vertex processing mode. 
	// WARNING: resets streams and shaders
	/*if ( changed(cur.mixedVP,rs.mixedVP) )
	{
		HRESULT hr = device->SetSoftwareVertexProcessing( cur.mixedVP );
		if ( hr != D3D_OK )
			error( "Failed to toggle mixed mode vertex processing: %s", toString(hr) );

		// invalidate vertex streams and shaders
		cur.d3dfvf = -1;
	}*/

	// set vertex format
	assert( rs.d3dfvf != -1 ); // vertex format to must be set
	if ( changed(cur.d3dfvf,rs.d3dfvf) )
	{
		cur.d3dfvf = rs.d3dfvf;
		HRESULT hr = device->SetFVF( rs.d3dfvf );
		if ( D3D_OK != hr ) 
			error( "Could not set rendering device FVF: %s", toString(hr) );
	}

	// set vertex blending
	int weights = rs.vertexWeights;
	if ( changed(cur.vertexWeights,weights) )
	{
		if ( weights > 0 )
		{
			D3DVERTEXBLENDFLAGS vblend = (D3DVERTEXBLENDFLAGS)
				(D3DVBF_1WEIGHTS + weights - 1);
			
			HRESULT hr = device->SetRenderState( D3DRS_VERTEXBLEND, vblend );
			if ( D3D_OK != hr ) 
				error( "Could not set rendering device vertex blending state: %s", toString(hr) );

			hr = device->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, TRUE );
			if ( D3D_OK != hr )
				error( "Could not set rendering device indexed vertex blending state: %s", toString(hr) );
		}
		else
		{
			device->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE );
			device->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		}
	}

	// diffuse, ambient, specular, emissive, power
	HRESULT hr;
	if ( changed(cur.refl,rs.refl) )
	{
		D3DMATERIAL9 d3dmat;
		toDx9( rs.refl, d3dmat );
		hr = device->SetMaterial( &d3dmat );
		if ( D3D_OK != hr ) 
			error( "Failed to set material reflectance: %s", toString(hr) );
	}

	// alpha blending
	if ( changed(cur.srcBlend,rs.srcBlend) )
	{
		cur.srcBlend = rs.srcBlend;
		hr = device->SetRenderState( D3DRS_SRCBLEND, toDx9(rs.srcBlend) );
		if ( D3D_OK != hr ) 
			error( "Failed to set source blending state: %s", toString(hr) );
	}
	if ( changed(cur.dstBlend,rs.dstBlend) )
	{
		cur.dstBlend = rs.dstBlend;
		hr = device->SetRenderState( D3DRS_DESTBLEND, toDx9(rs.dstBlend) );
		if ( D3D_OK != hr ) 
			error( "Failed to set destination blending state: %s", toString(hr) );

		// is alpha blending enabled?
		bool alphaBlend = false;
		if ( Dx9Material::BLEND_ZERO != rs.dstBlend ) 
			alphaBlend = true;
		hr = device->SetRenderState( D3DRS_ALPHABLENDENABLE, alphaBlend );
		if ( D3D_OK != hr ) 
			error( "Failed to set alpha blending state: %s", toString(hr) );
	}

	// depth test
	if ( changed(cur.depthEnabled,rs.depthEnabled) )
	{
		hr = device->SetRenderState( D3DRS_ZENABLE, rs.depthEnabled );
		if ( D3D_OK != hr ) 
			error( "Failed to toggle depth buffer usage: %s", toString(hr) );
	}

	// depth write
	if ( changed(cur.depthWrite,rs.depthWrite) )
	{
		hr = device->SetRenderState( D3DRS_ZWRITEENABLE, rs.depthWrite );
		if ( D3D_OK != hr ) 
			error( "Failed to toggle depth buffer write: %s", toString(hr) );
	}

	// depth func
	if ( changed(cur.depthFunc,rs.depthFunc) )
	{
		hr = device->SetRenderState( D3DRS_ZFUNC, toDx9(rs.depthFunc) );
		if ( D3D_OK != hr ) 
			error( "Failed to set depth buffer compare function: %s", toString(hr) );
	}

	// cull
	if ( changed(cur.cull,rs.cull) )
	{
		hr = device->SetRenderState( D3DRS_CULLMODE, toDx9(rs.cull) );
		if ( D3D_OK != hr ) 
			error( "Failed to set cull mode: %s", toString(hr) );
	}

	// specular enabled
	if ( changed(cur.specularEnabled,rs.specularEnabled) )
	{
		DWORD specularEnabled = rs.specularEnabled;
		if ( !(D3DPSHADECAPS_SPECULARGOURAUDRGB & caps.ShadeCaps) )
			specularEnabled = FALSE;
		hr = device->SetRenderState( D3DRS_SPECULARENABLE, specularEnabled );
		if ( D3D_OK != hr ) 
			error( "Failed to toggle specular reflection: %s", toString(hr) );
	}

	// lighting
	if ( changed(cur.lighting,rs.lighting) )
	{
		hr = device->SetRenderState( D3DRS_LIGHTING, rs.lighting );
		if ( D3D_OK != hr ) 
			error( "Failed to toggle lighting: %s", toString(hr) );
	}

	// vertex color
	if ( changed(cur.vertexColor,rs.vertexColor) )
	{
		hr = device->SetRenderState( D3DRS_COLORVERTEX, rs.vertexColor );
		if ( D3D_OK != hr ) 
			error( "Failed to toggle vertex color: %s", toString(hr) );
	}

	// fog disabled for this material
	if ( m_fog != Dx9GraphicsDevice::FOG_NONE &&
		changed(cur.fogDisabled,rs.fogDisabled) )
	{
		hr = device->SetRenderState( D3DRS_FOGENABLE, !rs.fogDisabled );
		if ( D3D_OK != hr )
			error( "Failed to toggle fog: %s", toString(hr) );
	}

	// diffuse source
	if ( changed(cur.diffuseSource,rs.diffuseSource) )
	{
		hr = device->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, toDx9(rs.diffuseSource) );
		if ( D3D_OK != hr ) 
			error( "Failed to set material diffuse source: %s", toString(hr) );
	}

	// specular source
	if ( changed(cur.specularSource,rs.specularSource) )
	{
		hr = device->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, toDx9(rs.specularSource) );
		if ( D3D_OK != hr ) 
			error( "Failed to set material specular source: %s", toString(hr) );
	}

	// ambient source
	if ( changed(cur.ambientSource,rs.ambientSource) )
	{
		hr = device->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, toDx9(rs.ambientSource) );
		if ( D3D_OK != hr ) 
			error( "Failed to set material ambient source: %s", toString(hr) );
	}

	// emissive source
	if ( changed(cur.emissiveSource,rs.emissiveSource) )
	{
		hr = device->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, toDx9(rs.emissiveSource) );
		if ( D3D_OK != hr ) 
			error( "Failed to set material emissive source: %s", toString(hr) );
	}

	// texture layers
	const int							maxLayers	= Dx9Material::TEXTURE_LAYERS < m_caps.MaxTextureBlendStages ? Dx9Material::TEXTURE_LAYERS : m_caps.MaxTextureBlendStages;
	const Dx9Material::TextureLayer*	layerEnd	= rs.textureLayers + maxLayers;
	const Dx9Material::TextureLayer*	layer		= rs.textureLayers;
	int									layerIndex	= 0;

	while ( layer != layerEnd )
	{
		Dx9Material::TextureLayer* curlayer = &cur.textureLayers[layerIndex];

		// last layer?
		if ( !layer->enabled() )
		{
			if ( changed(curlayer->cOp,layer->cOp) || changed(curlayer->aOp,layer->aOp) )
			{
				device->SetTexture( layerIndex, 0 );
				device->SetTextureStageState( layerIndex, D3DTSS_COLOROP, D3DTOP_DISABLE );
				device->SetTextureStageState( layerIndex, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
			}
			break;
		}

		// layer color combine
		if ( changed(curlayer->cOp,layer->cOp) )
		{
			hr = device->SetTextureStageState( layerIndex, D3DTSS_COLOROP, toDx9(layer->cOp) );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture layer color combine operation: %s", toString(hr) );
		}
		if ( changed(curlayer->cArg1,layer->cArg1) )
		{
			hr = device->SetTextureStageState( layerIndex, D3DTSS_COLORARG1, toDx9(layer->cArg1) );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture layer color combine argument 1: %s", toString(hr) );
		}
		if ( changed(curlayer->cArg2,layer->cArg2) )
		{
			hr = device->SetTextureStageState( layerIndex, D3DTSS_COLORARG2, toDx9(layer->cArg2) );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture layer color combine argument 2: %s", toString(hr) );
		}

		// layer alpha combine
		if ( changed(curlayer->aOp,layer->aOp) )
		{
			hr = device->SetTextureStageState( layerIndex, D3DTSS_ALPHAOP, toDx9(layer->aOp) );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture layer alpha combine operation: %s", toString(hr) );
		}
		if ( changed(curlayer->aArg1,layer->aArg1) )
		{
			hr = device->SetTextureStageState( layerIndex, D3DTSS_ALPHAARG1, toDx9(layer->aArg1) );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture layer alpha combine argument 1: %s", toString(hr) );
		}
		if ( changed(curlayer->aArg2,layer->aArg2) )
		{
			hr = device->SetTextureStageState( layerIndex, D3DTSS_ALPHAARG2, toDx9(layer->aArg2) );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture layer alpha combine argument 2: %s", toString(hr) );
		}

		// set texture surface
		IDirect3DBaseTexture9* d3dtex = 0;
		gd::BaseTexture* layerTex = layer->texture();
		if ( layerTex )
		{
			d3dtex = static_cast<Dx9BaseTextureImplInterface*>( layerTex->impl() )->getDx9Texture( this );
		}
		hr = device->SetTexture( layerIndex, d3dtex );
		if ( D3D_OK != hr ) 
			error( "Failed to set texture: %s", toString(hr) );

		// texture coordinate source
		if ( changed(curlayer->coordinateSource,layer->coordinateSource) ||
			 changed(curlayer->coordinateSet, layer->coordinateSet) )
		{
			DWORD texCoordinateSource = 0;
			if ( Material::TCS_VERTEXDATA == layer->coordinateSource )
				texCoordinateSource = layer->coordinateSet;
			else
				texCoordinateSource = toDx9( layer->coordinateSource );
			hr = device->SetTextureStageState( layerIndex, D3DTSS_TEXCOORDINDEX, texCoordinateSource );
			if ( D3D_OK != hr ) 
				error( "Failed to set used texture coordinate layer: %s", toString(hr) );
		}

		// texture coordinate transform type
		if ( changed(curlayer->coordinateTransform,layer->coordinateTransform) )
		{
			hr = device->SetTextureStageState( layerIndex, D3DTSS_TEXTURETRANSFORMFLAGS, toDx9(layer->coordinateTransform) );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture coordinate transform type: %s", toString(hr) );
		}

		// texture coordinate transform matrix
		if ( Material::TTFF_DISABLE != layer->coordinateTransform )
		{
			if ( changed(curlayer->coordinateTransformMatrix,layer->coordinateTransformMatrix) )
			{
				D3DMATRIX d3dm;
				toDx9( layer->coordinateTransformMatrix, d3dm );
				hr = device->SetTransform( D3DTRANSFORMSTATETYPE(D3DTS_TEXTURE0+layerIndex), &d3dm );
				if ( D3D_OK != hr ) 
					error( "Failed to set texture coordinate transform matrix: %s", toString(hr) );
			}
		}

		// texture addressing mode
		if ( changed(curlayer->addressMode,layer->addressMode) )
		{
			D3DSAMPLERSTATETYPE addr[] = { D3DSAMP_ADDRESSU, D3DSAMP_ADDRESSV, D3DSAMP_ADDRESSW };
			D3DTEXTUREADDRESS addrMode = toDx9(layer->addressMode);
			for ( int j = 0 ; j < 3 ; ++j )
			{
				hr = device->SetSamplerState( layerIndex, addr[j], addrMode );
				if ( D3D_OK != hr ) 
					error( "Failed to set texture adressing mode: %s", toString(hr) );
			}
		}

		// texture filtering modes
		if ( changed(curlayer->filter,layer->filter) )
		{
			D3DTEXTUREFILTERTYPE d3dfilt = toDx9( layer->filter );

			hr = device->SetSamplerState( layerIndex, D3DSAMP_MAGFILTER, d3dfilt );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture mag filter: %s", toString(hr) );

			hr = device->SetSamplerState( layerIndex, D3DSAMP_MINFILTER, d3dfilt );
			if ( D3D_OK != hr ) 
				error( "Failed to set texture min filter: %s", toString(hr) );
		}

		++layerIndex;
		++layer;
	}

	// stenciling
	if ( changed(cur.stencil,rs.stencil) )
	{
		hr = device->SetRenderState( D3DRS_STENCILENABLE, rs.stencil );
		if ( D3D_OK != hr ) 
			error( "Failed to toggle stenciling: %s", toString(hr) );
	}
	if ( rs.stencil )
	{
		hr = device->SetRenderState( D3DRS_STENCILFAIL, toDx9(rs.stencilFail) );
		if ( D3D_OK != hr ) 
			error( "Failed to set stencil fail operation: %s", toString(hr) );
		
		hr = device->SetRenderState( D3DRS_STENCILZFAIL, toDx9(rs.stencilZFail) );
		if ( D3D_OK != hr ) 
			error( "Failed to set stencil Z-fail operation: %s", toString(hr) );

		hr = device->SetRenderState( D3DRS_STENCILPASS, toDx9(rs.stencilPass) );
		if ( D3D_OK != hr ) 
			error( "Failed to set stencil pass operation: %s", toString(hr) );

		hr = device->SetRenderState( D3DRS_STENCILFUNC, toDx9(rs.stencilFunc) );
		if ( D3D_OK != hr ) 
			error( "Failed to set stencil function: %s", toString(hr) );

		hr = device->SetRenderState( D3DRS_STENCILREF, rs.stencilRef );
		if ( D3D_OK != hr ) 
			error( "Failed to set stencil fail reference value: %s", toString(hr) );

		hr = device->SetRenderState( D3DRS_STENCILMASK, rs.stencilMask );
		if ( D3D_OK != hr ) 
			error( "Failed to set stencil mask: %s", toString(hr) );
		hr = device->SetRenderState( D3DRS_STENCILWRITEMASK, rs.stencilMask );
		if ( D3D_OK != hr ) 
			error( "Failed to set stencil write mask: %s", toString(hr) );
	}

	// alpha test
	if ( changed(cur.alphaTestEnabled,rs.alphaTestEnabled) )
	{
		hr = device->SetRenderState( D3DRS_ALPHATESTENABLE, rs.alphaTestEnabled );
		if ( D3D_OK != hr ) 
			error( "Failed to toggle alpha test: %s", toString(hr) );
	}
	if ( rs.alphaTestEnabled )
	{
		hr = device->SetRenderState( D3DRS_ALPHAFUNC, toDx9(rs.alphaCompareFunc) );
		if ( D3D_OK != hr ) 
			error( "Failed to set alpha test compare function: %s", toString(hr) );

		hr = device->SetRenderState( D3DRS_ALPHAREF, rs.alphaReferenceValue );
		if ( D3D_OK != hr ) 
			error( "Failed to set alpha test reference value: %s", toString(hr) );
	}

	// per polygon depth sorting
	cur.polygonSorting = (uint8_t)(rs.polygonSorting ? 1 : 0);
}

DWORD Dx9GraphicsDevice::getDeviceFVF( const VertexFormat& format ) const
{
	DWORD fvf = 0;
	int texcoords = format.textureCoordinates();

	if ( format.hasNormal() )
		fvf |= D3DFVF_NORMAL;
	if ( format.hasDiffuse() )
		fvf |= D3DFVF_DIFFUSE;
	if ( format.hasSpecular() )
		fvf |= D3DFVF_SPECULAR;

	for ( int i = 0 ; i < format.textureCoordinates() ; ++i )
	{
		int dim = format.getTextureCoordinateSize( i );
		switch ( dim )
		{
		case 1:		fvf |= D3DFVF_TEXCOORDSIZE1(i); break;
		case 2:		fvf |= D3DFVF_TEXCOORDSIZE2(i); break;
		case 3:		fvf |= D3DFVF_TEXCOORDSIZE3(i); break;
		case 4:		fvf |= D3DFVF_TEXCOORDSIZE4(i); break;
		}
	}

	int weights = format.weights();
	if ( 0 == weights )
	{
		if ( format.hasRHW() )
			fvf |= D3DFVF_XYZRHW;
		else
			fvf |= D3DFVF_XYZ;
	}
	else
	{
		if ( m_vp != VERTEXP_HW )
		{
			// Note that the last 'weight' is actually a DWORD in indexed vertex blending
			switch ( weights )
			{
			case 1:		fvf |= D3DFVF_XYZB2; break;
			case 2:		fvf |= D3DFVF_XYZB3; break;
			default:	fvf |= D3DFVF_XYZB4; break;
			}
			fvf |= D3DFVF_LASTBETA_UBYTE4;
		}
		else
		{
			// HW vertex processing, pass weights only as texcoords
			fvf |= D3DFVF_XYZ;
		}

		// add weights & indices to two extra texcoord layers
		// because hardware GF3 compatible vertex shader can't use UBYTE4 for matrix skinning
		// NOTE: this breaks legacy skinning combined with legacy multitexturing material
		fvf |= D3DFVF_TEXCOORDSIZE4(texcoords) | D3DFVF_TEXCOORDSIZE4((texcoords+1));
		texcoords += 2;
	}

	switch ( texcoords )
	{
	case 0:		fvf |= D3DFVF_TEX0; break;
	case 1:		fvf |= D3DFVF_TEX1; break;
	case 2:		fvf |= D3DFVF_TEX2; break;
	case 3:		fvf |= D3DFVF_TEX3; break;
	case 4:		fvf |= D3DFVF_TEX4; break;
	case 5:		fvf |= D3DFVF_TEX5; break;
	case 6:		fvf |= D3DFVF_TEX6; break;
	case 7:		fvf |= D3DFVF_TEX7; break;
	case 8:		fvf |= D3DFVF_TEX8; break;
	}

/*#ifdef _DEBUG
	// DEBUG: double check vertex size
	int vsize2 = 12;
	if ( format.hasRHW() )
		vsize2 += 4;
	if ( format.hasDiffuse() )
		vsize2 += 4;
	if ( format.hasSpecular() )
		vsize2 += 4;
	if ( format.hasNormal() )
		vsize2 += 12;
	if ( format.weights() > 0 && m_vp != VERTEXP_HW )
		vsize2 += (format.weights()+1)*4;
	if ( format.weights() > 0 && m_vp == VERTEXP_HW )
		vsize2 += 2*4*4;
	for ( int i = 0 ; i < format.textureCoordinates() ; ++i )
		vsize2 += format.getTextureCoordinateSize(i)*4;
	int vsize = D3DXGetFVFVertexSize( fvf );
	assert( vsize == vsize2 );
#endif*/
	return fvf;
}

int Dx9GraphicsDevice::getDeviceVertexSize( const VertexFormat& format ) const
{
	DWORD fvf = getDeviceFVF( format );
	int vsize = D3DXGetFVFVertexSize( fvf );
	return vsize;
}
