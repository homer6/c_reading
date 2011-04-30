#include "SceneManager.h"
#include <sg/Node.h>
#include <io/InputStreamArchive.h>
#include <sgu/SceneFile.h>
#include <sgu/ModelFileCache.h>
#include <util/Hashtable.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace util;
using namespace lang;

//-----------------------------------------------------------------------------

namespace sgu
{


class SceneManager::SceneManagerImpl :
	public Object
{
public:
	P(InputStreamArchive)		arch;
	P(ModelFileCache)			modelCache;
	Hashtable<String,P(Node)>	nodes;

	SceneManagerImpl() :
		arch(0),
		modelCache(0),
		nodes( Allocator< HashtablePair<String,P(Node)> >(__FILE__,__LINE__) )
	{
	}
};

//-----------------------------------------------------------------------------

SceneManager::SceneManager( InputStreamArchive* arch )
{
	m_this = new SceneManagerImpl;
	m_this->arch = arch;
	m_this->modelCache = new ModelFileCache( arch );
}

SceneManager::~SceneManager()
{
}

Node* SceneManager::getScene( const String& file, int flags )
{
	P(Node) node = m_this->nodes[file];
	if ( !node )
	{
		SceneFile playerSceneFile( file, m_this->modelCache, m_this->arch, flags );
		node = playerSceneFile.scene();
		m_this->nodes[file] = node;
	}
	return node;
}

ModelFile* SceneManager::getModelFile( const String& file,
	const lang::String* boneNames, int bones,
	const pix::Colorf& ambient, int loadFlags )
{
	return m_this->modelCache->getByName( file, boneNames, bones, ambient, loadFlags );
}


} // sgu
