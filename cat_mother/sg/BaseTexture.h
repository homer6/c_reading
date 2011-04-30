#ifndef _SG_BASETEXTURE_H
#define _SG_BASETEXTURE_H


#include <sg/ContextObject.h>
#include <lang/String.h>


namespace gd {
	class BaseTexture;}

namespace io {
	class InputStream; }

namespace pix {
	class SurfaceFormat;}


namespace sg 
{


class TextureCache;


/** 
 * Base class for cubemaps and regular bitmap textures. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BaseTexture :
	public ContextObject
{
public:
	/** Type of texture map. */
	enum MapType
	{
		/** Texture is normal 2D bitmap. */
		MAP_BITMAP = 0,
		/** Texture is cubemap with 6 faces. */
		MAP_CUBEMAP = 1
	};

	BaseTexture();
	virtual ~BaseTexture();
	
	/** Sets the name of the texture. */
	void								setName( const lang::String& name );
	
	/** Returns the name of the texture. */
	const lang::String&					name() const;

	/** Returns pixel format of the surface, extending implementations should overload this function. */
	virtual const pix::SurfaceFormat&	format() const = 0;

	/** Returns low level texture object. */
	virtual gd::BaseTexture*			baseTexture() const = 0;

	/** Returns texture object type. */
	virtual MapType						mapType() const = 0;

protected:
	/** Returns texture cache. */
	static TextureCache&				getTextureCache();

private:
	lang::String		m_name;

	BaseTexture( const BaseTexture& other );
	BaseTexture& operator=( const BaseTexture& other );		
};


} // sg


#endif
