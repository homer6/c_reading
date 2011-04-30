#ifndef _GMUTIL_H
#define _GMUTIL_H


#include <lang/String.h>
#include <util/Vector.h>


class GmModel;
class GmMaterial;


/**
 * Export object creation util.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */ 
class GmUtil
{
public:
	static P(GmModel)		createGmModel( INode* node, Mesh* mesh, PatchMesh* patchmesh, ShapeObject* shape, Mtl* material );
	static P(GmMaterial)	createGmMaterial( Mtl* material, Mtl* bakedmaterial = 0 );
	static lang::String		getGmFilename( const lang::String& nodeName );
};


#endif // _GMUTIL_H
