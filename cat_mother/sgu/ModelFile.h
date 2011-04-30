#ifndef _SGU_MODELFILE_H
#define _SGU_MODELFILE_H


#include <lang/Object.h>


namespace io {
	class InputStream;
	class InputStreamArchive;}

namespace sg {
	class Primitive;}

namespace pix {
	class Colorf;}

namespace lang {
	class String;}


namespace sgu
{


class ModelFileCache;


/** 
 * 3D geometry file loader.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ModelFile :
	public lang::Object
{
public:
	/** Loading options. */
	enum LoadFlags
	{
		/** Load geometry and materials. */
		LOAD_GEOMETRY	= 2,
		/** Load morpher and morph targets only. */
		LOAD_MORPH		= 4,
		/** Load everything. */
		LOAD_ALL		= LOAD_GEOMETRY+LOAD_MORPH,
	};

	/** 
	 * Reads the models from the stream.
	 * @param name Name of the file.
	 * @param boneNames Name of the bones in the mesh.
	 * @param bones Number of the bones in the mesh.
	 * @param ambient Ambient vertex color for unlit objects.
	 * @param arch Archive for the files.
	 * @param modelCache Cache of models. Used for loading models (morph targets) connected to this one.
	 * @param loadFlags Loading options. See LoadFlags.
	 * @exception IOException
	 * @exception GraphicsDeviceException
	 */
	explicit ModelFile( const lang::String& name,
		const lang::String* boneNames, int bones,
		const pix::Colorf& ambient,
		io::InputStreamArchive* arch, ModelFileCache* modelCache, int loadFlags=LOAD_ALL );

	/** Returns ith primitive in the file. */
	sg::Primitive*	getPrimitive( int index ) const;

	/** Returns number of primitives in the file. */
	int				primitives() const;

	/** Returns name of the primitive file. */
	const lang::String&		name() const;

private:
	class ModelFileImpl;
	P(ModelFileImpl) m_this;

	ModelFile( const ModelFile& );
	ModelFile& operator=( const ModelFile& );
};


} // sgu


#endif // _SGU_MODELFILE_H
