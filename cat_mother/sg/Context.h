#ifndef _SG_CONTEXT_H
#define _SG_CONTEXT_H


#include <lang/Object.h>


namespace gd {
	class GraphicsDriver;
	class GraphicsDevice;}

namespace lang {
	class String;}


namespace sg
{


/** 
 * The graphics library context. 
 * The context must be created before any other graphics library objects.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Context :
	public lang::Object
{
public:
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
		VERTEXP_HW
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

	/** Texture Compression type. */
	enum TextureCompressionType
	{
		/** Device does not use compressed textures unless DXT pixel format is used. */
		TC_NONE,
		/** Device automatically compresses all textures unless prohibited by pixel format description. */
		TC_COMPRESSED
	};

	/** 
	 * Initializes the graphics library context. 
	 * Must be the first call to the graphics library.
	 * @param name Name of the library driver without debug build identifier (d) and file extension.
	 * @exception Exception
	 */
	explicit Context( const lang::String& name );

	///
	~Context();

	/** 
	 * Destroys the graphics library context.
	 * All objects created by the graphics library driver will be destroyed.
	 * Calling destroy() must always be the last call to the graphics library.
	 */
	void		destroy();

	/** 
	 * Initializes rendering device window with default parameters.
	 * @exception Exception
	 */
	void		open();

	/** 
	 * Initializes rendering device window.  
	 * @param width Width of the back buffer.
	 * @param height Height of the back buffer.
	 * @param bitsPerPixel Number of bits per pixel in fullscreen mode. Pass 0 for desktop window mode.
	 * @param refreshRate Refresh rate in Hz if fullscreen mode.
	 * @param surfaceFlags Back buffer surfaces. See SurfaceFlags.
	 * @param rz Type of used rasterizer.
	 * @param vp Type of used vertex processing.
	 * @exception Exception
	 */
	void		open( int width, int height, int bitsPerPixel, 
					int refreshRate, int surfaceFlags, 
					RasterizerType rz, VertexProcessingType vp,
					TextureCompressionType textureCompression);

	/** Deinitializes the rendering device window. */
	void		close();

	/** Flushes objects from rendering device memory. */
	void		flushDeviceObjects();

	/**  
	 * Restores device context after losing device.
	 * @return true if validation succeed.
	 */
	bool		restore();

	/** 
	 * Call before doing any rendering. Pair always with endScene() call. 
	 */
	void		beginScene();

	/** 
	 * Call after done all rendering in single rendered frame. 
	 * Pair always with beginScene() call. Never throws an exception 
	 * so you can always call this after rendering
	 * even if an exception was thrown during rendering.
	 */
	void		endScene();

	/** 
	 * Updates back buffer contents to the screen. 
	 * The rendering device must be initialized with open() first.
	 */
	void		present();

	/**
	 * Clears specified buffers of the viewport.
	 */
	void		clear( int surfaces=SURFACE_TARGET|SURFACE_DEPTH|SURFACE_STENCIL );

	/** Sets texture mipmap filter. Default is TEXF_NONE. */
	void		setMipMapFilter( TextureFilterType mode );

	/** 
	 * Sets texture mipmap level of detail bias.
	 * Each +-1 bias alters the selection by one mipmap level. 
	 * Negative bias causes the use of larger mipmaps,
	 * positive bias causes the use of smaller mipmaps. 
	 */
	void		setMipMapLODBias( float bias );

	/** Returns true if context is ready for rendering. */
	bool		ready() const;

	/** Returns width of the render target. */
	int			width() const;

	/** Returns height of the render target. */
	int			height() const;

	/** Returns true if the device is in fullscreen mode. */
	bool		fullscreen() const;

	/* Returns true if rendering device has been initialized. */
	static bool					initialized();

	/* Returns active graphics driver. */
	static gd::GraphicsDriver*	driver();

	/* Returns active graphics device. */
	static gd::GraphicsDevice*	device();

private:
	class ContextImpl;
	P(ContextImpl) m_this;
	static ContextImpl* sm_active;

	Context();
	Context( const Context& );
	Context& operator=( const Context& );
};


} // sg


#endif // _SG_CONTEXT_H
