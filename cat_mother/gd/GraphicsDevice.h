#ifndef _GD_GRAPHICSDEVICE_H
#define _GD_GRAPHICSDEVICE_H


namespace pix {
	class SurfaceFormat;
	class Color;}

namespace math {
	class Matrix4x4;}


namespace gd
{


class LightState;
class Material;


/** 
 * Interface to rendering device.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GraphicsDevice
{
public:
	/** Device window type. */
	enum WindowType
	{
		/** Render to desktop-window. */
		WINDOW_DESKTOP,
		/** Render to full-screen window. */
		WINDOW_FULLSCREEN
	};

	/**
	 * Device rasterizer type.
	 */
	enum RasterizerType
	{
		/** Software rasterizer. */
		RASTERIZER_SW,
		/** Hardware rasterizer. */
		RASTERIZER_HW
	};

	/**
	 * Device vertex processing type.
	 */
	enum VertexProcessingType
	{
		/** Software vertex processing. */
		VERTEXP_SW,
		/** Hardware vertex processing. */
		VERTEXP_HW,
		/** Hardware/software vertex processing. */
		VERTEXP_MIXED
	};

	/** 
	 * Device frame buffer surface flags. 
	 */
	enum SurfaceFlags
	{
		/** Frame	buffer color channel. */
		SURFACE_TARGET		= 1,
		/** Frame buffer depth channel. */
		SURFACE_DEPTH		= 2,
		/** Frame buffer stencil channel. */
		SURFACE_STENCIL		= 4,
	};

	/** 
	 * Operation modes for traditional fog effect.
	 */
	enum FogMode
	{
		/** No fogging */
		FOG_NONE,
		/** Linear fogging */
		FOG_LINEAR,
		/** Exponential fogging */
		FOG_EXP,
		/** Squared exponential fogging */
		FOG_EXP2
	};

	/** Filter to be used when rendering textures. */
	enum TextureFilterType
	{
		/** Mipmapping disabled. */
		TEXF_NONE,
		/** Point filtering. */
		TEXF_POINT,
		/** Bilinear interpolation filtering. */
		TEXF_LINEAR,
		/** Anisotropic texture filtering. */
		TEXF_ANISOTROPIC
	};

	/** Texture compression support. */
	enum TextureCompressionSupport
	{
		/** Device does not support texture compression. */
		TCSUPPORT_NONE,
		/** Device supports texture compression. */
		TCSUPPORT_SUPPORTED
	};

	/** Texture compression used. */
	enum TextureCompression
	{
		/** Device does not use compressed textures unless DXT pixel format is used. */
		TC_NONE,
		/** Device automatically compresses all textures unless prohibited by pixel format description. */
		TC_COMPRESSED
	};

	/** 
	 * Initializes the device object that renders to current active window.
	 * @param width Width of the frame buffer.
	 * @param height Height of the frame buffer.
	 * @param bitsPerPixel Requested bits per pixel.
	 * @param refreshRate Refresh rate in Hz if fullscreen mode.
	 * @param win Window type, fullscreen or desktop.
	 * @param rz Rasterizer type.
	 * @param vp Vertex processing type.
	 * @param buffers Frame buffer surface type. See SurfaceFlags.
	 * @return Error code, or 0 if initialization ok.
	 * @see DeviceFlags
	 */
	virtual int			create( int width, int height, 
							int bitsPerPixel, int refreshRate, 
							WindowType win, RasterizerType rz, 
							VertexProcessingType vp, int buffers, 
							TextureCompression tc ) = 0;

	/** Deinitializes the rendering device explicitly. */
	virtual void		destroy() = 0;

	/**  
	 * Restores device context after losing device.
	 * @return true if validation succeed.
	 */
	virtual bool		restore() = 0;

	/** Flushes all device dependent objects. */
	virtual void		flushDeviceObjects() = 0;

	/** Increments reference count. */
	virtual void		addReference() = 0;

	/** Decrements reference count and deletes the object if no more references left. */
	virtual void		release() = 0;

	/** 
	 * Sets ambient light used in rendering of following objects. 
	 */
	virtual void		setAmbient( const pix::Color& ambientLight ) = 0;

	/** 
	 * Adds a light to be used in rendering of following objects. 
	 */
	virtual void		addLight( const gd::LightState& lightDesc ) = 0;

	/** 
	 * Removes all lights from the device. 
	 * If objects get rendered before adding any new lights with addLight() they get lit only by ambient() light. 
	 */
	virtual void		removeLights() = 0;
	
	/** 
	 * Call before doing any rendering. Pair always with endScene() call. 
	 */
	virtual void		beginScene() = 0;

	/** 
	 * Call after done all rendering in single rendered frame. 
	 * Pair always with beginScene() call. Never throws an exception 
	 * so you can always call this after rendering
	 * even if an exception was thrown during rendering.
	 */
	virtual void		endScene() = 0;

	/** 
	 * Flips/blits back buffer to screen. 
	 * Call after last endScene() inside single frame to show back buffer contents. 
	 */
	virtual void		present() = 0;

	/** 
	 * Sets transform from local/model to world space. 
	 */
	virtual void		setWorldTransform( const math::Matrix4x4& modelToWorld ) = 0;

	/** 
	 * Sets transform from local/model to world space for a bone.
	 * @param modelToWorld Transform from object space to world space.
	 */
	virtual void		setWorldTransform( int index, const math::Matrix4x4& modelToWorld ) = 0;

	/** 
	 * Sets transform from world space to camera space. 
	 * @param worldToView Transform from world space to camera space.
	 */
	virtual void		setViewTransform( const math::Matrix4x4& worldToView ) = 0;
	
	/** 
	 * Sets transform from camera space to screen space. 
	 * @param toScreen Transform from camera space to screen space.
	 */
	virtual void		setProjectionTransform( const math::Matrix4x4& cameraToScreen ) = 0;

	/** 
	 * Sets viewport. Origin is top left corner, x grows left, y grows down. 
	 */
	virtual void		setViewport( int x, int y, int width, int height ) = 0;

	/** 
	 * Clears specified buffers of the viewport.
	 * @see SurfaceFlags
	 */
	virtual void		clear( int flags, const pix::Color& color, int stencil=0 ) = 0;

	/** 
	 * Enable/disable clipping. The device may have better performance if 
	 * clipping is disabled when it's not needed. 
	 */
	virtual void		setClipping( bool enabled ) = 0;

	/** 
	 * Sets fogging mode.  
	 */
	virtual void		setFog( FogMode mode ) = 0;

	/** 
	 * Sets color of the fog. 
	 */
	virtual void		setFogColor( const pix::Color& color ) = 0;
	
	/** 
	 * Sets fog start distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	virtual void		setFogStart( float start ) = 0;
	
	/** 
	 * Sets fog end distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	virtual void		setFogEnd( float end ) = 0;
	
	/** 
	 * Sets fog start density. Affects when fog mode is exp or exp^2. 
	 * Valid value range is between [0,1]. 
	 */
	virtual void		setFogDensity( float density ) = 0;

	/** Sets texture mipmap filter. Default is TEXF_NONE. */
	virtual void		setMipMapFilter( TextureFilterType mode ) = 0;

	/** 
	 * Sets texture mipmap level of detail bias.
	 * Each +-1 bias alters the selection by one mipmap level. 
	 * Negative bias causes the use of larger mipmaps,
	 * positive bias causes the use of smaller mipmaps. 
	 */
	virtual void		setMipMapLODBias( float bias ) = 0;

	/**
	 * Locks the back buffer for reading.
	 * @return true if lock ok.
	 */ 
	virtual bool		lockBackBuffer( void** surface, int* pitch ) = 0;

	/** Unlocks the back buffer. */ 
	virtual void		unlockBackBuffer() = 0;

	/** Returns true if the render states are valid. */
	virtual bool		validate() = 0;

	/** 
	 * Resets following statistics to 0:
	 * - renderedPrimitives() <br>
	 * - renderedTriangles() <br>
	 * - lockedVertices() <br>
	 * - lockedIndices() <br>
	 */
	virtual void		resetStatistics() = 0;

	/** Returns true if context is ready for rendering. */
	virtual bool		ready() const = 0;

	/** 
	 * Returns transform from local/model to world space. 
	 */
	virtual void		getWorldTransform( math::Matrix4x4* modelToWorld ) const = 0;

	/** 
	 * Returns all transforms from local/model to world space. 
	 * Next call to setWorldTransform can invalidate returned pointer.
	 */
	virtual const math::Matrix4x4*	worldTransforms() const = 0;

	/** 
	 * Returns number of transforms from local/model to world space. 
	 * Next call to setWorldTransform can invalidate returned count.
	 */
	virtual int			worldTransformCount() const = 0;

	/** 
	 * Returns transform from world space to camera space. 
	 * @param worldToView [out] Transform from world space to camera space.
	 */
	virtual void		getViewTransform( math::Matrix4x4* worldToView ) const = 0;
	
	/** 
	 * Returns transform from camera space to screen space. 
	 * @param toScreen [out] Transform from camera space to screen space.
	 */
	virtual void		getProjectionTransform( math::Matrix4x4* cameraToScreen ) const = 0;

	/** Returns back buffer surface format. */
	virtual void		getFormat( pix::SurfaceFormat* fmt ) const = 0;

	/** Returns width of the frame buffer. */
	virtual int			width() const = 0;

	/** Returns height of the frame buffer. */
	virtual int			height() const = 0;

	/** 
	 * Returns true if a scene is being rendered.
	 * The return value is true only between 
	 * beginScene() and endScene() function calls.
	 */
	virtual bool		sceneInProgress() const = 0;

	/** Returns true if the device is in full-screen frame buffer mode. */
	virtual bool		fullscreen() const = 0;
	
	/** 
	 * Returns true if clipping is enabled. 
	 * @see setClipping
	 */
	virtual bool		clipping() const = 0;

	/** Returns fogging mode. */
	virtual FogMode		fog() const = 0;

	/** Returns color of the fog (if fog is enabled). */
	virtual const pix::Color&	fogColor() const = 0;
	
	/** 
	 * Returns fog start distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	virtual float		fogStart() const = 0;
	
	/** 
	 * Returns fog end distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	virtual float		fogEnd() const = 0;
	
	/** 
	 * Returns fog start density. Affects when fog mode is exp or exp^2. 
	 * Valid value range is between [0,1]. 
	 */
	virtual float		fogDensity() const = 0;

	/** Returns type of used mipmap filter. */
	virtual TextureFilterType	mipMapFilter() const = 0;

	/** Returns true if the device supports stencil operations. */
	virtual bool		stencil() const = 0;

	/** Returns number of bytes texture memory used. */
	virtual long		textureMemoryUsed() const = 0;

	/** Returns number of primitives rendered. */
	virtual int			renderedPrimitives() const = 0;

	/** Returns number of triangles rendered. */
	virtual int			renderedTriangles() const = 0;

	/** Returns number of material changes. */
	virtual int			materialChanges() const = 0;

	/** Returns number of locked vertices. */
	virtual int			lockedVertices() const = 0;

	/** Returns number of locked indices. */
	virtual int			lockedIndices() const = 0;

	/** Returns support for compressed textures. */
	virtual TextureCompressionSupport	textureCompressionSupported() const = 0;

protected:
	GraphicsDevice() {}
	virtual ~GraphicsDevice() {}

private:
	GraphicsDevice( const GraphicsDevice& );
	GraphicsDevice& operator=( const GraphicsDevice& );
};


} // gd


#endif // _GD_GRAPHICSDEVICE_H
