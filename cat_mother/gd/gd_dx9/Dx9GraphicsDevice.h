#ifndef DXGRAPHICSDEVICE_H
#define DXGRAPHICSDEVICE_H


#include "Array.h"
#include "DrvObject.h"
#include "Dx9RenderingState.h"
#include <gd/GraphicsDevice.h>
#include <pix/Color.h>


class Dx9Texture;
class Dx9Material;


/**
 * Device implementation.
 * @author Jani Kajala (jani.kajala@helsinki.fi), Toni Aittoniemi
 */
class Dx9GraphicsDevice : 
	public gd::GraphicsDevice,
	public DrvObject
{
public:
	/** Supported texture compression formats. */
	class TCFormatsSupported
	{
	public:
		TCFormatsSupported() : uyvy(false), yuy2(false), dxt1(false), dxt2(false), dxt3(false), dxt4(false), dxt5(false) {}

		bool anyFormatSupported() { return ( uyvy || yuy2 || dxt1 || dxt2 || dxt3 || dxt4 || dxt5 ); }

		bool	uyvy;
		bool	yuy2;
		bool	dxt1;
		bool	dxt2;
		bool    dxt3;
		bool	dxt4;
		bool	dxt5;
	};

	Dx9GraphicsDevice();
	~Dx9GraphicsDevice();

	int			create( int width, int height, int bitsPerPixel, int refreshRate, WindowType win, RasterizerType rz, VertexProcessingType vp, int buffers, TextureCompression tc );
	void		destroy();
	void		flushDeviceObjects();
	void		addReference();
	void		release();
	void		setAmbient( const pix::Color& ambientLight );
	void		addLight( const gd::LightState& lightDesc );
	void		removeLights();
	void		beginScene();
	void		endScene();
	void		present();
	void		setWorldTransform( const math::Matrix4x4& modelToWorld );
	void		setWorldTransform( int index, const math::Matrix4x4& modelToWorld );
	void		setViewTransform( const math::Matrix4x4& worldToView );
	void		setProjectionTransform( const math::Matrix4x4& cameraToScreen );
	void		setViewport( int x, int y, int width, int height );
	void		clear( int flags, const pix::Color& color, int stencil );
	void		setClipping( bool enabled );
	void		setFog( FogMode mode );
	void		setFogColor( const pix::Color& color );
	void		setFogStart( float start );
	void		setFogEnd( float end );
	void		setFogDensity( float density );
	void		setMipMapFilter( TextureFilterType mode );
	void		setMipMapLODBias( float bias );
	bool		lockBackBuffer( void** surface, int* pitch );
	void		unlockBackBuffer();
	void		resetStatistics();
	bool		validate();
	bool		restore();

	/** Adds one locked primitive and n triangles to statistics. */
	void		updateLockStatistics( int vertices, int indices );

	/** 
	 * Call after rendering any geometry with D3D objects. 
	 * @param triangles Number of triangles in the primitive (for statistics).
	 */
	void		updateStatistics( int triangles );

	/** 
	 * Toggles rendering on/off. 
	 * Used by materials which can't be rendered on the device.
	 */
	void		setRendering( bool enabled ) 										{m_renderEnabled = enabled;}

	/** 
	 * Returns temporary buffer of integers. Use with care!
	 * Do not store the returned buffer as some other caller
	 * might request the buffer. Also do not call any functions 
	 * while using the buffer unless you are absolutely sure that 
	 * the called function doesn't request another buffer.
	 */
	int*		getTemporaryIntegerBuffer( int capacity );

	/** Sets whole rendering state at once. */
	void		setRenderState( const Dx9RenderingState& rs );

	/** Restores rendering state to default. */
	void		setDefaultRenderState();

	/** Restores rendering state to default by forcing state changes. */
	void		resetRenderState();

	Dx9RenderingState&	renderingState()											{return m_rs;}

	const math::Matrix4x4*	worldTransforms() const;
	int			worldTransformCount() const;
	void		getWorldTransform( math::Matrix4x4* modelToWorld ) const;
	void		getViewTransform( math::Matrix4x4* worldToView ) const;
	void		getProjectionTransform( math::Matrix4x4* cameraToScreen ) const;
	void		getFormat( pix::SurfaceFormat* fmt ) const;
	int			width() const;
	int			height() const;
	bool		sceneInProgress() const;
	bool		fullscreen() const;
	bool		clipping() const;
	FogMode		fog() const;
	const pix::Color&	fogColor() const;
	float		fogStart() const;
	float		fogEnd() const;
	float		fogDensity() const;
	TextureFilterType mipMapFilter() const;
	bool		stencil() const;
	long		textureMemoryUsed() const;
	int			renderedPrimitives() const;
	int			renderedTriangles() const;
	int			materialChanges() const;
	int			lockedVertices() const;
	int			lockedIndices() const;
	bool		ready() const;

	/** Returns true if rendering is enabled. */
	bool		rendering() const													{return m_renderEnabled;}

	/** Returns device compatible vertex size. */
	int			getDeviceVertexSize( const gd::VertexFormat& vf ) const;

	/** Returns device compatible FVF. */
	DWORD		getDeviceFVF( const gd::VertexFormat& vf ) const;

	/** Returns Direct3D object. */
	IDirect3D9*				d3d() const												{return m_d3d;}

	/** Returns Direct3D device object. */
	IDirect3DDevice9*		d3dDevice() const										{return m_device;}

	/** Returns device caps. */
	const D3DCAPS9&			caps() const											{return m_caps;}

	/** Returns current display mode info. */
	const D3DDISPLAYMODE&	displayMode() const										{return m_displaymode;}

	/** Returns current rendering state. */
	const Dx9RenderingState&	renderingState() const								{return m_rs;}

	/** Returns support for compressed textures. */
	TextureCompressionSupport	textureCompressionSupported() const					{return m_textureCompressionSupport;}

	/** Returns true if texture compression is in use. */
	bool		textureCompressionEnabled() const									{return m_useTextureCompression == TC_COMPRESSED;}

	/** Returns reference to supported texture compression formats. */
	const TCFormatsSupported&	tcFormatsSupported() const							{return m_tcFormatsSupported;}

	/** Returns device vertex processing type. */
	VertexProcessingType		vertexProcessing() const							{return m_vp;}

private:
	long						m_refs;
	IDirect3D9*					m_d3d;
	IDirect3DDevice9*			m_device;
	int							m_lights;
	bool						m_sceneInProgress;
	D3DDISPLAYMODE				m_displaymode;
	D3DCAPS9					m_caps;
	D3DPRESENT_PARAMETERS		m_present;
	int							m_buffers;
	int							m_renderedTriangles;
	int							m_renderedPrimitives;
	int							m_materialChanges;
	int							m_lockedIndices;
	int							m_lockedVertices;
	int							m_renderWidth;
	int							m_renderHeight;
	IDirect3DSurface9*			m_lockedBackBuffer;
	bool						m_renderEnabled;
	VertexProcessingType		m_vp;

	// Direct3D device dependent
	Dx9RenderingState			m_rs;
	Dx9RenderingState			m_defaultRS;
	pix::Color					m_fogColor;
	float						m_fogStart;
	float						m_fogEnd;
	float						m_fogDensity;
	FogMode						m_fog;
	TextureFilterType			m_mipMapFilter;
	float						m_mipMapLODBias;
	TextureCompressionSupport	m_textureCompressionSupport;
	TextureCompression			m_useTextureCompression;
	TCFormatsSupported			m_tcFormatsSupported;

	Array<int>					m_intBuffer;
	Array<math::Matrix4x4>		m_worldTM;

	void				defaults();
	void				destroyDeviceObject();
	void				setRenderState( D3DRENDERSTATETYPE state, DWORD value );
	DWORD				getRenderState( D3DRENDERSTATETYPE state ) const;
	void				setMixedVertexProcessing( bool enabled );

	bool				fogSupported() const										{return 0 != (D3DPRASTERCAPS_FOGVERTEX & m_caps.RasterCaps);} 
	bool				textureFormatSupported( D3DFORMAT format ) const;

	Dx9GraphicsDevice( const Dx9GraphicsDevice& );
	Dx9GraphicsDevice& operator=( const Dx9GraphicsDevice& );
};


#endif
