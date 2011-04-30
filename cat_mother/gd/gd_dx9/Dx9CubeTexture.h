#include "Dx9BaseTexture.h"
#include "Dx9GraphicsDevice.h"
#include "Dx9BaseTextureImplInterface.h"
#include <gd/CubeTexture.h>
#include <pix/Surface.h>


/**
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9CubeTexture : 
	public Dx9BaseTexture,
	public gd::CubeTexture,
	public Dx9BaseTextureImplInterface
{
public:
	Dx9CubeTexture();
	~Dx9CubeTexture();

	void		addReference();
	void		release();
	int			create( int edgelength, const pix::SurfaceFormat& format );
	int			create( pix::Surface* surfaces, int mipmaplevels );
	void		destroy();
	void		load( gd::GraphicsDevice* device );
	void		unload();
	bool		lock( const gd::LockMode& mode, int subsurface );
	void		unlock( int subsurface);
	void*		data(int subsurface);
	
	gd::BaseTextureImplInterface* impl();

	int							width() const;
	int							height() const;
	const pix::SurfaceFormat&	format() const;
	bool						locked(int subsurface) const;
	const void*					data(int subsurface) const;
	int							pitch() const;
	long						textureMemoryUsed() const;
	bool						dirty() const;

	/** 
	 * Returns Direct3D texture. 
	 * Doesn't increment returned object reference count.
	 */
	IDirect3DBaseTexture9*	getDx9Texture( Dx9GraphicsDevice* dev );

private:
	long						m_refs;
	Array<pix::Surface>			m_imgs;
	IDirect3DCubeTexture9*		m_tex;
	bool						m_dirty;
	bool						m_locked;
	int							m_mipMapLevels;

	void	destroyDeviceObject();

	static int			roundUpPow2( int x );
	static D3DFORMAT	getNextBestFormat( D3DFORMAT format, int index );
	static D3DFORMAT	useCompressionFormat( const pix::SurfaceFormat& textureformat, D3DFORMAT defaultformat, const Dx9GraphicsDevice::TCFormatsSupported& supported );
	static bool			isCompressed( D3DFORMAT fmt) { return ((fmt==D3DFMT_DXT1) || (fmt==D3DFMT_DXT2) || (fmt==D3DFMT_DXT3) || (fmt==D3DFMT_DXT4) || (fmt==D3DFMT_DXT5)); }

	Dx9CubeTexture( const Dx9CubeTexture& );
	Dx9CubeTexture& operator=( const Dx9CubeTexture& );
};
