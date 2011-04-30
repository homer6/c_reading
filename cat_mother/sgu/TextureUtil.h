#ifndef _SGU_TEXTUREUTIL_H
#define _SGU_TEXTUREUTIL_H


#include <sg/CubeTexture.h>


namespace sgu
{


/** 
 * Texture creation utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class TextureUtil
{
	/** Creates cube map for normalizing vectors. */
	P(sg::CubeTexture) createNormalizerCubeTexture( int cubeSide );
};


} // sgu


#endif // _SGU_TEXTUREUTIL_H
