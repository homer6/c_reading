#ifndef _DX9RENDERTOTEXTURE_H
#define _DX9RENDERTOTEXTURE_H


#include "DrvObject.h"
#include <gd/RenderToTexture.h>


class Dx9GraphicsDevice;


/**
 * UNFINISHED.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9RenderToTexture :
	public gd::RenderToTexture,
	public DrvObject
{
public:
	Dx9RenderToTexture();
	~Dx9RenderToTexture();

	void		addReference();
	void		release();
	int			create( gd::GraphicsDevice* device, int width, int height, const pix::SurfaceFormat& format, 
					bool depthstencil, const pix::SurfaceFormat& depthformat );
	void		destroy();
	void		beginScene( gd::Texture* texture, int x, int y, int width, int height );
	void		endScene( gd::GraphicsDevice::TextureFilterType mipfilter );

private:
	long					m_refs;
	ID3DXRenderToSurface*	m_rts;
	Dx9GraphicsDevice*		m_dev;

	void		resetDeviceObject();
	void		destroyDeviceObject();

	Dx9RenderToTexture( const Dx9RenderToTexture& );
	Dx9RenderToTexture& operator=( const Dx9RenderToTexture& );
};


#endif // _DX9RENDERTOTEXTURE_H
