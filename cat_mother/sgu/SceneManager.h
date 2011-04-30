#ifndef _SGU_SCENEMANAGER_H
#define _SGU_SCENEMANAGER_H


#include <sgu/SceneFile.h>
#include <sgu/ModelFile.h>


namespace io {
	class InputStreamArchive;}

namespace sg {
	class Node;}

namespace pix {
	class Colorf;}

namespace lang {
	class String;}


namespace sgu
{


class ModelFile;


/** 
 * Class for managing scene loading. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SceneManager :
	public lang::Object
{
public:
	/** Prepares to load scenes from specified archive. */
	explicit SceneManager( io::InputStreamArchive* arch );

	/** Releases loaded scenes. */
	~SceneManager();

	/** Loads scene. */
	sg::Node*	getScene( const lang::String& file, int flags=SceneFile::LOAD_ALL );

	/** 
	 * Gets model file by name. 
	 * @param name Name of the model file.
	 * @param boneNames Names of the available mesh bones.
	 * @param bones Number of bones available in the mesh.
	 * @param ambient Ambient vertex color for unlit objects.
	 * @param loadFlags See ModelFile::LoadFlags.
	 */
	ModelFile*	getModelFile( const lang::String& name,
					const lang::String* boneNames, int bones,
					const pix::Colorf& ambient, int loadFlags=ModelFile::LOAD_ALL );

private:
	class SceneManagerImpl;
	P(SceneManagerImpl) m_this;

	SceneManager( const SceneManager& );
	SceneManager& operator=( const SceneManager& );
};


} // sgu


#endif // _SGU_SCENEMANAGER_H
