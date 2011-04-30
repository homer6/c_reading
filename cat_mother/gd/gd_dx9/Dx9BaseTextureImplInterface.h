#ifndef _DX8BASETEXTUREIMPLINTERFACE_H
#define _DX8BASETEXTUREIMPLINTERFACE_H


#include <gd/BaseTextureImplInterface.h>


class Dx9BaseTextureImplInterface :
	public gd::BaseTextureImplInterface
{
public:
	/** 
	 * Returns Direct3D texture. 
	 * Doesn't increment returned object reference count.
	 */
	virtual IDirect3DBaseTexture9*	getDx9Texture( Dx9GraphicsDevice* dev ) = 0;
};


#endif // _DX8BASETEXTUREIMPLINTERFACE_H
