#include "SgNode.h"
#include "SgMesh.h"
#include "SgLight.h"
#include "SgCamera.h"
#include "SgLOD.h"
#include "SgDummy.h"
#include <util/Vector.h>


/**
 * Scene graph utilities.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgUtil
{
public:
	static P(SgMesh)	createMesh( INode* node, Interval animRange );
	static P(SgLight)	createLight( INode* node, Interval animRange );
	static P(SgCamera)	createCamera( INode* node, Interval animRange );
	static P(SgLOD)		createLOD( INode* node, Interval animRange );
	static P(SgDummy)	createDummy( INode* node, Interval animRange );
	static P(SgNode)	createUnknown( INode* node, Interval animRange );

	static void			addBones( SgMesh* mesh, INode* node, const util::Vector<INode*>& allnodes );
};
