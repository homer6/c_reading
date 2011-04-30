#include "StdAfx.h"
#ifdef SGEXPORT_PHYSIQUE
#include "PhyExportUtil.h"
#include "DebugUtil.h"
#include <mb/VertexMap.h>
#include <dev/Profile.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <util/Vector.h>
#include <PhyExp.h>
#include <BipExp.h>
#include <modstack.h>

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

/** Adds a weight to the vertex weight map. */
static inline void addWeight( mb::VertexMap* vmap, int vertex, float weight )
{
	//weight = 0.0f;
	vmap->addValue( vertex, &weight, 1 );
}

//-----------------------------------------------------------------------------

Modifier* PhyExportUtil::findPhysiqueModifier( INode* node )
{
	// Get object from node. Abort if no object.
	::Object* obj = node->GetObjectRef();
	if ( !obj ) 
		return 0;

	// Is derived object ?
	while ( obj->SuperClassID() == GEN_DERIVOB_CLASS_ID && obj )
	{
		// Yes -> Cast.
		IDerivedObject* derivedObj = static_cast<IDerivedObject*>( obj );
						
		// Iterate over all entries of the modifier stack.
		for ( int modStackIndex = 0 ; modStackIndex < derivedObj->NumModifiers() ; ++modStackIndex )
		{
			// Get current modifier.
			Modifier* modPtr = derivedObj->GetModifier( modStackIndex );

			// Is this Physique ?
			if ( modPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B) )
			{
				// Yes -> Exit.
				return modPtr;
			}
		}
		obj = derivedObj->GetObjRef();
	}

	// Not found.
	return 0;
}

void PhyExportUtil::listBones( IPhyContextExport* mcExport, Vector<INode*>& bones )
{
	int vertices = mcExport->GetNumberVertices();
	for ( int i = 0 ; i < vertices ; ++i )
	{
		// rigid vertex / rigid blended vertex?
		IPhyVertexExport* vi = mcExport->GetVertexInterface( i );
		IPhyFloatingVertex* fv = static_cast<IPhyFloatingVertex*>( mcExport->GetFloatingVertexInterface(i) );
		
		if ( vi && !fv )
		{
			int type = vi->GetVertexType();

			switch ( type )
			{
			case RIGID_TYPE:{
				IPhyRigidVertex* rv = static_cast<IPhyRigidVertex*>( vi );
				INode* bone = rv->GetNode();
				if ( -1 == bones.indexOf(bone) ) 
					bones.add(bone);
				break;}

			case (RIGID_TYPE|BLENDED_TYPE):{
				IPhyBlendedRigidVertex* rbv = static_cast<IPhyBlendedRigidVertex*>( vi );
				for ( int x = 0 ; x < rbv->GetNumberNodes() ; ++x ) 
				{
					INode* bone = rbv->GetNode( x );
					if ( -1 == bones.indexOf(bone) ) 
						bones.add(bone);
				}
				break;}

			default:{
				require( false );
				break;}
			}
		}
		else if ( fv ) // floating vertex
		{
			Debug::println( "Floating vertex found!" );
			for ( int x = 0; x < fv->GetNumberNodes() ; ++x )
			{
				INode* bone = fv->GetNode( x );
				if ( -1 == bones.indexOf(bone) ) 
					bones.add(bone);
			}
		}
		else
		{
			require( false );
		}

		if ( fv )
			mcExport->ReleaseVertexInterface( fv );
		if ( vi )
			mcExport->ReleaseVertexInterface( vi );
	}

	// print list of used bones
	Debug::println( "    Used bones ({0}):", bones.size() );
	for ( int i = 0 ; i < bones.size() ; ++i )
		Debug::println( "      bone {0,#}: {1}", i, (String)bones[i]->GetName() );
}

/** Inserts all vertices affected by the node to the vertex map. */
void PhyExportUtil::addWeights( mb::VertexMap* vmap, INode* node, IPhyContextExport* mcExport )
{
	int vertices = mcExport->GetNumberVertices();
	for ( int i = 0 ; i < vertices ; ++i )
	{
		// rigid vertex / rigid blended vertex?
		IPhyVertexExport* vi = mcExport->GetVertexInterface( i /*,HIERARCHIAL_VERTEX*/ );
		IPhyFloatingVertex* fv = static_cast<IPhyFloatingVertex*>( mcExport->GetFloatingVertexInterface(i) );

		if ( vi && !fv )
		{
			int type = vi->GetVertexType();

			switch ( type )
			{
			case RIGID_TYPE:{
				IPhyRigidVertex* rv = static_cast<IPhyRigidVertex*>( vi );
				INode* bone = rv->GetNode();
				if ( bone == node )
					addWeight( vmap, i, 1.f );
				break;}

			case (RIGID_TYPE|BLENDED_TYPE):{
				IPhyBlendedRigidVertex* rbv = static_cast<IPhyBlendedRigidVertex*>( vi );
				for ( int x = 0 ; x < rbv->GetNumberNodes() ; ++x ) 
				{
					INode* bone = rbv->GetNode( x );
					if ( bone == node )
					{
						float weight = rbv->GetWeight( x );
						addWeight( vmap, i, weight );
					}
				}
				break;}

			default:{
				require( false );
				break;}
			}
		}
		else if ( fv )
		{
			Debug::println( "Floating vertex found!" );
			for ( int x = 0; x < fv->GetNumberNodes() ; ++x )
			{
				INode* bone = fv->GetNode( x );
				if ( bone == node )
				{
					float total;
					float weight = fv->GetWeight( x, total );
					addWeight( vmap, i, weight );
				}
			}
		}
		else
		{
			require( false );
		}

		if ( fv )
			mcExport->ReleaseVertexInterface( fv );
		if ( vi )
			mcExport->ReleaseVertexInterface( vi );
	}

	// print list of mapped weights
	/*Debug::println( "    Weight map {0}:", vmap->name() );
	for ( int i = 0 ; i < vertices ; ++i )
	{
		float w = 0.f;
		if ( vmap->getValue(i, &w, 1) )
			Debug::println( "      Vertex {0,#}: {1,#.###}", i, w );
	}*/
}

Matrix3 PhyExportUtil::getInitBoneTM( INode* node, IPhysiqueExport* phyExport )
{
	return node->GetNodeTM(0);
}

Matrix3 PhyExportUtil::getInitSkinTM( INode* node, IPhysiqueExport* phyExport )
{
	return node->GetNodeTM(0);
}

#endif // SGEXPORT_PHYSIQUE
