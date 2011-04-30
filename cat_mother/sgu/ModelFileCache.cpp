#include "ModelFileCache.h"
#include <io/FileInputStream.h>
#include <io/InputStreamArchive.h>
#include <sgu/ModelFile.h>
#include <lang/String.h>
#include <math/Vector3.h>
#include <util/Hashtable.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace math;
using namespace util;

//-----------------------------------------------------------------------------

namespace sgu
{


class ModelFileCache::ModelFileCacheImpl :
	public lang::Object
{
public:
	Hashtable< String, P(ModelFile) >	files;
	P(InputStreamArchive)				zip;

	ModelFileCacheImpl() :
		files( Allocator< HashtablePair<String,P(ModelFile)> >(__FILE__,__LINE__) ),
		zip(0)
	{
	}
};

//-----------------------------------------------------------------------------

ModelFileCache::ModelFileCache( InputStreamArchive* zip )
{
	assert( zip );

	m_this			= new ModelFileCacheImpl;
	m_this->zip		= zip;
}

ModelFileCache::~ModelFileCache()
{
}

ModelFile* ModelFileCache::getByName( const String& name,
	const String* boneNames, int bones, const pix::Colorf& ambient, int loadFlags )
{
	String fname = name.toLowerCase();
	P(ModelFile) model = m_this->files[fname];
	if ( !model )
	{
		model = new ModelFile( name, boneNames, bones, ambient, m_this->zip, this, loadFlags );
		m_this->files[fname] = model;
	}
	return model;
}

void ModelFileCache::clear()
{
	m_this->files.clear();
}


} // sgu
