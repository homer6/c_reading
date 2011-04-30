#include "Context.h"
#include "BaseTexture.h"
#include "CubeTexture.h"
#include "TextureCache.h"
#include "Texture.h"
#include <io/InputStream.h>
#include <pix/SurfaceFormat.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace sg
{


static TextureCache		s_textureCache;

//-----------------------------------------------------------------------------

BaseTexture::BaseTexture() :									
	m_name("") 
{
}

BaseTexture::~BaseTexture()
{
}

/** Sets the name of the texture. */
void BaseTexture::setName( const lang::String& name )		
{ 
	m_name = name; 
}
	
/** Returns the name of the texture. */
const lang::String&	BaseTexture::name() const		
{ 
	return m_name; 
}

TextureCache& BaseTexture::getTextureCache()
{
	return s_textureCache;
}


} // sg
