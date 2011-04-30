#include "StdAfx.h"
#include "SceneExportUtil.h"
#include "TmUtil.h"
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <math/Matrix3x3.h>
#include <algorithm>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

/** Predicate for comparing material names. */
class MtlNameLess
{
public:
	bool operator()( Mtl* a, Mtl* b ) const
	{
		return strcmp(a->GetName(),b->GetName()) < 0;
	}
};

//-----------------------------------------------------------------------------

/** Lists all nodes recursively. */
static void listNodes( INode* parent, Vector<INode*>& nodes )
{
	nodes.add( parent );

	for ( int k = 0 ; k < parent->NumberOfChildren() ; ++k )
	{
		INode* child = parent->GetChildNode( k );
		if ( child )
			listNodes( child, nodes );
	}
}

//-----------------------------------------------------------------------------

void SceneExportUtil::getNodesToExport( Interface* i, DWORD options, Vector<INode*>& nodes )
{
	nodes.clear();
	
	if ( SCENE_EXPORT_SELECTED & options )
	{
		for ( int k = 0 ; k < i->GetSelNodeCount() ; ++k )
		{
			INode* node = i->GetSelNode(k);
			if ( node )
			{
				nodes.add( node );
			}
		}
	}
	else
	{
		INode* root = i->GetRootNode();
		if ( root )
			listNodes( root, nodes );
	}
}

void SceneExportUtil::getUsedMaterials( INode* node, Vector<Mtl*>& materials )
{
	require( node );

	Mtl* mat = node->GetMtl();
	if ( mat )
	{
		materials.add( mat );

		// get sub-materials (Multi/Sub-Object material)
		for ( int k = 0 ; k < mat->NumSubMtls() ; ++k )
		{
			Mtl* submat = mat->GetSubMtl(k);
			if ( submat )
				materials.add( submat );
		}
	}

	// remove duplicates
	std::sort( materials.begin(), materials.end() );
	std::unique( materials.begin(), materials.end() );

	// sort to abc order
	std::sort( materials.begin(), materials.end(), MtlNameLess() );
}

void SceneExportUtil::getUsedMaterials( const Vector<INode*>& nodes, Vector<Mtl*>& materials )
{
	for ( int j = 0 ; j < nodes.size() ; ++j )
	{
		INode* node = nodes[j];

		if ( node )
			getUsedMaterials( node, materials );
	}
}

void SceneExportUtil::getTriObjectFromNode( INode *node, TimeValue t, bool* deleteIt, TriObject** triobj )
{
	TriObject* tri = 0;
	bool needDelete = false;

	if ( node )
	{
		::Object* obj = node->EvalWorldState(t).obj;

		if ( obj )
		{
			if ( obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)) ) 
			{ 
				tri = static_cast<TriObject*>( obj->ConvertToType( t, Class_ID(TRIOBJ_CLASS_ID,0) ) );

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if ( obj != tri ) 
					needDelete = true;
			}
		}
	}

	*deleteIt = needDelete;
	*triobj = tri;
}

BitmapTex* SceneExportUtil::getStdMatBitmapTex( StdMat* stdmat, int id )
{
	StdMat2* stdmat2 = 0;
	int channel = id;
	if ( stdmat->SupportsShaders() )
	{
		stdmat2 = static_cast<StdMat2*>( stdmat );
		channel = stdmat2->StdIDToChannel( id );
	}

	if ( stdmat->MapEnabled(channel) )
	{
		Texmap*	tex	= stdmat->GetSubTexmap(channel);
		if ( tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID,0) &&
			(!stdmat2 || 2 == stdmat2->GetMapState(channel)) )
		{
			BitmapTex* bmptex = static_cast<BitmapTex*>(tex);
			if ( bmptex->GetMapName() )
			{
				return bmptex;
			}
		}
	}
	return 0;
}

void SceneExportUtil::copyFile( const lang::String& target, const lang::String& source )
{
	Array<char,1000> oldFile( source.length()+1 );
	Array<char,1000> newFile( target.length()+1 );
	source.getBytes( oldFile.begin(), oldFile.size(), "ASCII-7" );
	target.getBytes( newFile.begin(), newFile.size(), "ASCII-7" );
	CopyFile( oldFile.begin(), newFile.begin(), FALSE );
}

String SceneExportUtil::stripPath( const String& str )
{
	int i1 = str.lastIndexOf("/");
	int i2 = str.lastIndexOf("\\");

	int i = i1;
	if ( i < i2 )
		i = i2;

	return str.substring( i+1 );
}

String SceneExportUtil::getPath( const String& str )
{
	int i1 = str.lastIndexOf("/");
	int i2 = str.lastIndexOf("\\");

	int i = i1;
	if ( i < i2 )
		i = i2;

	return str.substring( 0, i );
}
