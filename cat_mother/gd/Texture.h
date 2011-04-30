#ifndef _GD_TEXTURE_H
#define _GD_TEXTURE_H


#include <gd/BaseTexture.h>


namespace pix {
	class Surface;
	class SurfaceFormat;}


namespace gd
{


class Material;
class LockMode;
class GraphicsDevice;


/** 
 * Interface to texture used in surface shading.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Texture :
	public BaseTexture
{
public:
	/** Texture usage type. */
	enum UsageType
	{
		/** Texture is used as normal texture resource. */
		USAGE_NORMAL,
		/** Texture is used as render target. */
		USAGE_RENDERTARGET
	};

	/** Increments reference count by one. */
	virtual void	addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void	release() = 0;

	/**
	 * Creates an empty texture of specified size.
	 * @param device Rendering device.
	 * @param width Preferred width of the texture.
	 * @param height Preferred height of the texture.
	 * @param format Preferred pixel format of the texture surface.
	 * @return Error code or 0 if ok.
	 */
	virtual int		create( gd::GraphicsDevice* device, int width, int height, const pix::SurfaceFormat& format, UsageType usage ) = 0;

	/**
	 * Creates a texture from the temporary surface.
	 * <em>Note</em> that surface data is transferred by <em>swapping</em>,
	 * so original surface data passed in to the function is destroyed.
	 * @param surfaces Source surface array which is destroyed by the function.
	 * @param mipmaplevels mipmap levels in array, pass 0 to generate mipmaps.
	 * @return Error code or 0 if ok.
	 */
	virtual int		create( gd::GraphicsDevice* device, pix::Surface* surfaces, int mipmaplevels ) = 0;

	/** Deinitializes the texture explicitly. */
	virtual void	destroy() = 0;

	/** Uploads object to the rendering device. */
	virtual void	load( gd::GraphicsDevice* device ) = 0;

	/** Unloads object from the rendering device. */
	virtual void	unload() = 0;

	/** 
	 * Gets access to surface data.
	 * IMPORTANT : Only locks the first mipmap level.
	 * Call unlock() to release the access.
	 * data() can be used after this. 
	 * @return false if lock failed.
	 */
	virtual bool	lock( const gd::LockMode& mode ) = 0;
	
	/** 
	 * Release access to the surface data.
	 * @see lock
	 */
	virtual void	unlock() = 0;

	/** Returns pointer to surface data. Can be called only when locked. */
	virtual void*	data() = 0;

	/** Returns width of the surface. */
	virtual int		width() const = 0;
	
	/** Returns height of the surface. */
	virtual int		height() const = 0;
	
	/** Returns pixel format of the surface. */
	virtual const pix::SurfaceFormat&	format() const = 0;

	/** Returns true if buffer is already locked. */
	virtual bool						locked() const = 0;
	
	/** Returns pointer to surface data. Can be called only when locked. */
	virtual const void*					data() const = 0;

	/** Returns distance (in bytes) to the start of next line. */
	virtual int							pitch() const = 0;

	/** Returns (approximate) number of bytes texture memory used by the texture. */
	virtual long						textureMemoryUsed() const = 0;

protected:
	Texture() {}
	virtual ~Texture() {}

private:
	Texture( const Texture& );
	Texture& operator=( const Texture& );
};


} // gd


#endif // _GD_TEXTURE_H
