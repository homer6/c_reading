#ifndef DX8BASETEXTURE_H
#define DX8BASETEXTURE_H


#include "Dx9GraphicsDevice.h"
#include "DrvObject.h"


/**
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9BaseTexture :
	public DrvObject
{
public:
	Dx9BaseTexture();
	~Dx9BaseTexture();

	/** Increments reference count by one. */
	virtual void	addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void	release() = 0;

	/** Deinitializes the texture explicitly. */
	virtual void	destroy() = 0;

	/** 
	 * Returns Direct3D texture. 
	 * Doesn't increment returned object reference count.
	 */
	virtual IDirect3DBaseTexture9*	getDx9Texture( Dx9GraphicsDevice* dev ) = 0;

private:

	Dx9BaseTexture( const Dx9BaseTexture& );
	Dx9BaseTexture& operator=( const Dx9BaseTexture& );
};

#endif