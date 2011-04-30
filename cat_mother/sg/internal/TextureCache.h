#ifndef _SG_TEXTURECACHE_H
#define _SG_TEXTURECACHE_H


#include <gd/Texture.h>
#include <gd/CubeTexture.h>
#include <lang/String.h>
#include <util/Hashtable.h>


namespace gd {
	class GraphicsDriver;}

namespace io {
	class InputStream;}

namespace lang {
	class String;}


namespace sg
{


/** 
 * Class for caching texture loading. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class TextureCache :
	public lang::Object
{
public:
	///
	TextureCache();

	/** Releases cached textures. */
	void				flushTextures();

	/** Loads a texture from stream if not in cache. */
	gd::Texture*		loadTexture( io::InputStream* in, const lang::String& name, 
							gd::GraphicsDriver* drv, gd::GraphicsDevice* dev );

	/** Loads a cube texture from stream if not in cache. */
	gd::CubeTexture*	loadCubeTexture( io::InputStream* in, const lang::String& name, 
							gd::GraphicsDriver* drv, gd::GraphicsDevice* dev );

	/** Loads multi-face cubetexture (as above). */
	gd::CubeTexture*	loadCubeTexture( io::InputStream* in1, io::InputStream* in2, 
						io::InputStream* in3, io::InputStream* in4, 
						io::InputStream* in5, io::InputStream* in6,
						const lang::String& name, 
						gd::GraphicsDriver* drv, gd::GraphicsDevice* dev );
	
	/** Enables/disables texture downscaling. */
	void				setDownScaling( bool enabled );

	/** Sets default bit depth for loaded textures. */
	void				setDefaultBitDepth( int bits );

	/** Returns (approximate) number of bytes device memory used by the textures. */
	long				textureMemoryUsed() const;

private:
	util::Hashtable<lang::String,P(gd::BaseTexture)>	m_textures;
	bool												m_downScaling;
	int													m_bitDepth;

	gd::Texture*		loadTextureImpl( io::InputStream* in, const lang::String& name, 
							gd::GraphicsDriver* drv, gd::GraphicsDevice* dev );
	gd::CubeTexture*	loadCubeTextureImpl( io::InputStream* in, const lang::String& name, 
							gd::GraphicsDriver* drv, gd::GraphicsDevice* dev );

	TextureCache( const TextureCache& );
	TextureCache& operator=( const TextureCache& );
};


} // sg


#endif // _SG_TEXTURECACHE_H
