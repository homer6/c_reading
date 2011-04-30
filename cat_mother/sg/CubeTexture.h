#ifndef _SG_CUBETEXTURE_H
#define _SG_CUBETEXTURE_H


#include <sg/BaseTexture.h>


namespace gd {
	class CubeTexture;}

namespace io {
	class InputStream;}

namespace pix {
	class Surface;}


namespace sg
{


class Material;
class TextureCache;


/** 
 * Texture to be used in surface rendering.
 * Texture can only be used in rendering by setting it to material.
 * Supported texture image file formats are TGA, JPG and BMP.
 *
 * All coordinates used are expressed in pixels.
 * Origin is top left, X grows right and Y grows down.
 *
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class CubeTexture :
	public BaseTexture
{
public:
	/**
	 * Creates an empty texture of specified size.
	 *
	 * @param width Preferred width of the texture.
	 * @param height Preferred height of the texture.
	 * @param format Preferred pixel format of the texture surface.
	 */
	CubeTexture( int edgelength, const pix::SurfaceFormat& format );

	/** Creates a texture from specified stream. */
	explicit CubeTexture( io::InputStream* in );

	/** Creates a texture from specified stream. */
	CubeTexture( io::InputStream* in, const lang::String& filename );

	///
	~CubeTexture();

	/** Destroys the object. */
	void	destroy();

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** 
	 * Copies an image to the texture surface.
	 */
	void	blt( const pix::Surface* img, int subsurface );

	/** Returns width of the surface. */
	int		width() const;
	
	/** Returns height of the surface. */
	int		height() const;
	
	/** Returns pixel format of the surface. */
	const pix::SurfaceFormat&	format() const;

	/** Returns distance (in bytes) to the start of next line. */
	int							pitch() const;

	/** Returns (approximate) number of bytes texture memory used by the texture. */
	long						textureMemoryUsed() const;

	/** Returns low level texture object. */
	gd::CubeTexture*			texture() const;

	/** Returns low level texture object. */
	gd::BaseTexture*			baseTexture() const;

	/** Returns texture map type. */
	MapType						mapType() const;

	/** Flushes loaded textures from the cache. */
	static void					flushTextures();

	/** Enables/disables loaded texture downscaling. */
	static void					setDownScaling( bool enabled );

	/** Sets default bit depth for loaded textures. */
	static void					setDefaultBitDepth( int bits );

	/** Returns (approximate) number of bytes texture memory used by all cached textures. */
	static long					totalTextureMemoryUsed();

private:
	friend class Material;
	P(gd::CubeTexture)	m_tex;

	CubeTexture();
	CubeTexture( const CubeTexture& other );
	CubeTexture& operator=( const CubeTexture& other );
};


} // sg


#endif // _SG_CUBETEXTURE_H
