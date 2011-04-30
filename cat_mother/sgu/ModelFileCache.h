#ifndef _SGU_MODELFILECACHE_H
#define _SGU_MODELFILECACHE_H


#include <sgu/ModelFile.h>
#include <lang/Object.h>


namespace io {
	class InputStreamArchive;}

namespace pix {
	class Colorf;}

namespace lang {
	class String;}

namespace math {
	class Vector3;}


namespace sgu
{


class ModelFile;


/** 
 * Model file cache. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ModelFileCache :
	public lang::Object
{
public:
	/** 
	 * Creates a model file cache loading files from archive. 
	 */
	explicit ModelFileCache( io::InputStreamArchive* zip );

	///
	~ModelFileCache();

	/** 
	 * Get model file by name. 
	 * @param name Name of the model file.
	 * @param boneNames Names of the available mesh bones.
	 * @param bones Number of bones available in the mesh.
	 * @param ambient Ambient vertex color for unlit objects.
	 * @param loadFlags See ModelFile::LoadFlags.
	 */
	ModelFile*	getByName( const lang::String& name,
					const lang::String* boneNames, int bones,
					const pix::Colorf& ambient, int loadFlags=ModelFile::LOAD_ALL );

	/** Removes all loaded model files from the cache. */
	void		clear();

private:
	class ModelFileCacheImpl;
	P(ModelFileCacheImpl) m_this;

	ModelFileCache( const ModelFileCache& );
	ModelFileCache& operator=( const ModelFileCache& );
};


} // sgu


#endif // _SGU_MODELFILECACHE_H
