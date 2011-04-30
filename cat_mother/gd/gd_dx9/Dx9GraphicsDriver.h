#include <gd/GraphicsDriver.h>


/**
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9GraphicsDriver :
	public gd::GraphicsDriver
{
public:
	Dx9GraphicsDriver();
	~Dx9GraphicsDriver();

	void						addReference();
	void						release();
	void						destroy();

	gd::Texture*				createTexture();
	gd::CubeTexture*			createCubeTexture();
	gd::Material*				createMaterial();
	gd::GraphicsDevice*			createGraphicsDevice();
	gd::Primitive*				createPrimitive();
	gd::Effect*					createEffect();

private:
	long							m_refs;

	Dx9GraphicsDriver( const Dx9GraphicsDriver& );
	Dx9GraphicsDriver& operator=( const Dx9GraphicsDriver& );
};
