#ifndef _SGU_SCENEFILE_H
#define _SGU_SCENEFILE_H


#include <lang/Object.h>


namespace lang {
	class String;}

namespace io {
	class InputStream;
	class InputStreamArchive;}

namespace sg {
	class Node;
	class Camera;}


namespace sgu
{


class ModelFileCache;


/**
 * 3D scene file loader.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SceneFile :
	public lang::Object
{
public:
	/** Loading options. */
	enum LoadFlags
	{
		/** Load key-framed animations. */
		LOAD_ANIMATIONS	= 1,
		/** Load geometry and textures. */
		LOAD_GEOMETRY	= 2,
		/** Load everything. */
		LOAD_ALL		= LOAD_ANIMATIONS+LOAD_GEOMETRY
	};

	/** 
	 * Loads scene file. 
	 *
	 * @param name Name of the scene file.
	 * @param modelcache Cache of geometry objects.
	 * @param zip File archive to use.
	 * @param loadFlags Loading options. See LoadFlags.
	 * @exception IOException
	 * @exception GraphicsDeviceException
	 */
	SceneFile( const lang::String& name, ModelFileCache* modelcache,
		io::InputStreamArchive* zip, int loadFlags=LOAD_ALL );

	///
	~SceneFile();

	/** Returns the root node of the loaded scene. */
	sg::Node*				scene() const;

	/** Returns a camera (if any) of the loaded scene. */
	sg::Camera*				camera() const;

	/** Returns name of the loaded scene file. */
	const lang::String&		name() const;

private:
	class SceneFileImpl;
	P(SceneFileImpl) m_this;

	SceneFile();
	SceneFile( const SceneFile& );
	SceneFile& operator=( const SceneFile& );
};


} // sgu


#endif // _SGU_SCENEFILE_H
