#include "Dx9BaseTexture.h"
#include "Dx9GraphicsDevice.h"
#include "Dx9BaseTextureImplInterface.h"
#include <gd/Texture.h>
#include <pix/Surface.h>


/** 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9Texture : 
	public Dx9BaseTexture,
	public gd::Texture,
	public Dx9BaseTextureImplInterface
{
public:
	Dx9Texture();
	~Dx9Texture();

	void		addReference();
	void		release();
	int			create( gd::GraphicsDevice* device, int width, int height, const pix::SurfaceFormat& format, UsageType usage );
	int			create( gd::GraphicsDevice* device, pix::Surface* surfaces, int mipmaplevels );
	void		destroy();
	void		load( gd::GraphicsDevice* device );
	void		load( gd::GraphicsDevice* device, pix::Surface* surfaces, int mipmaplevels, UsageType usage );
	void		unload();
	bool		lock( const gd::LockMode& mode );
	void		unlock();
	void*		data();

	gd::BaseTextureImplInterface* impl();

	int							width() const;
	int							height() const;
	const pix::SurfaceFormat&	format() const;
	bool						locked() const;
	const void*					data() const;
	int							pitch() const;
	long						textureMemoryUsed() const;

	IDirect3DTexture9*			d3dTexture() const									{return m_tex;}

	/** 
	 * Returns Direct3D texture. 
	 * Doesn't increment returned object reference count.
	 */
	IDirect3DBaseTexture9*	getDx9Texture( Dx9GraphicsDevice* dev );

private:
	long						m_refs;
	IDirect3DTexture9*			m_tex;
	int							m_width;
	int							m_height;
	pix::SurfaceFormat			m_format;

	// locked data
	void*	m_data;
	int		m_pitch;

	void	destroyDeviceObject();

	static int			roundUpPow2( int x );
	static D3DFORMAT	getNextBestFormat( D3DFORMAT format, int index );
	static D3DFORMAT	useCompressionFormat( const pix::SurfaceFormat& textureformat, D3DFORMAT defaultformat, const Dx9GraphicsDevice::TCFormatsSupported& supported );
	static bool			isCompressed( D3DFORMAT fmt) { return ((fmt==D3DFMT_DXT1) || (fmt==D3DFMT_DXT2) || (fmt==D3DFMT_DXT3) || (fmt==D3DFMT_DXT4) || (fmt==D3DFMT_DXT5)); }

	Dx9Texture( const Dx9Texture& );
	Dx9Texture& operator=( const Dx9Texture& );
};
