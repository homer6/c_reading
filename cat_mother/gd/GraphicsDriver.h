#ifndef _GD_GRAPHICSDRIVER_H
#define _GD_GRAPHICSDRIVER_H


namespace gd
{


class Effect;
class Texture;
class CubeTexture;
class Material;
class LineList;
class Primitive;
class TriangleList;
class GraphicsDevice;
class IndexedTriangleList;


/**
 * Interface to platform dependent graphics driver. 
 * The driver is used for creating other platform-dependent objects.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GraphicsDriver
{
public:
	/** Constants related to the graphics driver. */
	enum Constants
	{
		/** Version number of the current graphics engine drivers. */
		VERSION = 228
	};

	/** Increments reference count by one. */
	virtual void						addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void						release() = 0;

	/** Releases all objects allocated by the driver. */
	virtual void						destroy() = 0;

	/** Allocates a texture object. Initial reference count is 0. Thread safe. */
	virtual gd::Texture*				createTexture() = 0;

	/** Allocates a cube texture object. Initial reference count is 0. Thread safe. */
	virtual gd::CubeTexture*			createCubeTexture() = 0;

	/** Allocates a material object. Initial reference count is 0. Thread safe. */
	virtual gd::Material*				createMaterial() = 0;

	/** Allocates a device object. Initial reference count is 0. Thread safe. */
	virtual gd::GraphicsDevice*			createGraphicsDevice() = 0;

	/** Allocates a visual primitive object. Initial reference count is 0. Thread safe. */
	virtual gd::Primitive*				createPrimitive() = 0;

	/** Allocates a surface effect object. Initial reference count is 0. Thread safe. */
	virtual gd::Effect*					createEffect() = 0;

protected:
	GraphicsDriver() {}
	virtual ~GraphicsDriver() {}

private:
	GraphicsDriver( const GraphicsDriver& );
	GraphicsDriver& operator=( const GraphicsDriver& );
};


typedef GraphicsDriver*	(*createGraphicsDriverFunc)();
typedef int	(*getGraphicsDriverVersionFunc)();


} // gd


#endif // _GD_GRAPHICSDRIVER_H
