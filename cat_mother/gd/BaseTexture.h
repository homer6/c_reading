#ifndef _GD_BASETEXTURE_H
#define _GD_BASETEXTURE_H



namespace gd
{


class BaseTextureImplInterface;


/** 
 * Base interface for all textured used in surface shading.
 * All coordinates used are expressed in pixels.
 * Origin is top left, X grows right and Y grows down.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BaseTexture
{
public:
	/** Increments reference count by one. */
	virtual void	addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void	release() = 0;

	/** Deinitializes the texture explicitly. */
	virtual void	destroy() = 0;

	/** Returns (approximate) number of bytes texture memory used by the texture. */
	virtual long	textureMemoryUsed() const = 0;	

	/** Returns driver specific implementation interface. */
	virtual BaseTextureImplInterface*	impl() = 0;

protected:
	BaseTexture() {}
	virtual ~BaseTexture() {}

private:
	BaseTexture( const BaseTexture& );
	BaseTexture& operator=( const BaseTexture& );

};

}

#endif // _GD_BASETEXTURE_H
