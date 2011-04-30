#ifndef _GD_RENDERTOTEXTURE_H
#define _GD_RENDERTOTEXTURE_H


#include <gd/GraphicsDevice.h>


namespace pix {
	SurfaceFormat;}


namespace gd
{


class Texture;
class GraphicsDevice;


/** 
 * Interface for rendering scene to texture. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class RenderToTexture
{
public:
	/** Increments reference count by one. */
	virtual void		addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void		release() = 0;

	/** 
	 * Creates render-to-texture object. 
	 * @return 0 if no error, error code otherwise.
	 */
	virtual int			create( gd::GraphicsDevice* device, int width, int height, const pix::SurfaceFormat& format, 
							bool depthstencil, const pix::SurfaceFormat& depthformat ) = 0;

	/** Destroys render-to-texture object explicitly. */
	virtual void		destroy() = 0;

	/** 
	 * Call before doing any rendering. Pair always with endScene() call. 
	 * @param texture Render target texture.
	 * @param x Viewport top left x
	 * @param x Viewport top left y
	 * @param width Viewport width
	 * @param height Viewport height
	 */
	virtual void		beginScene( gd::Texture* texture, int x, int y, int width, int height ) = 0;

	/** 
	 * Call after done all rendering in single rendered frame. 
	 * Pair always with beginScene() call. Never throws an exception 
	 * so you can always call this after rendering
	 * even if an exception was thrown during rendering.
	 * @param mipfilter Filter used for mipmap generation.
	 */
	virtual void		endScene( gd::GraphicsDevice::TextureFilterType mipfilter ) = 0;
};


} // gd


#endif // _GD_RENDERTOTEXTURE_H
