#include "StdAfx.h"
#include "SkinExportUtil.h"
#include <mb/VertexMap.h>
#include <lang/Debug.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

Modifier* SkinExportUtil::findSkinModifier( INode* node )
{
	// Get object from node. Abort if no object.
	::Object* obj = node->GetObjectRef();
	if ( !obj )
		return 0;

	// print modifier stack
	//Debug::println( "Trying to find Skin modifier from {0}", String(node->GetName()) );
	while (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* derivedObj = static_cast<IDerivedObject*>(obj);
		//Debug::println( "  ---- derived object ({0} modifiers)", derivedObj->NumModifiers() );
		for ( int modStack = 0 ; modStack < derivedObj->NumModifiers() ; ++modStack )
		{
			Modifier* mod = derivedObj->GetModifier(modStack);
			//Debug::println( "  modifier {0}", String(mod->GetName()) );
		}
		obj = derivedObj->GetObjRef();
	}

	// Is derived object ?
	obj = node->GetObjectRef();
	while (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		// Yes -> Cast.
		IDerivedObject* derivedObj = static_cast<IDerivedObject*>(obj);

		// Iterate over all entries of the modifier stack.
		for ( int modStack = 0 ; modStack < derivedObj->NumModifiers() ; ++modStack )
		{
			// Get current modifier.
			Modifier* mod = derivedObj->GetModifier(modStack);

			// Is this Skin ?
			if ( mod->ClassID() == SKIN_CLASSID )
			{
				// Yes -> Exit.
				return mod;
			}
		}
		obj = derivedObj->GetObjRef();
	}
	return 0;
}

void SkinExportUtil::listBones( ISkin* skin, util::Vector<INode*>& bones )
{
	bones.clear();
	for ( int i = 0 ; i < skin->GetNumBones() ; ++i )
		bones.add( skin->GetBone(i) );
}

void SkinExportUtil::addWeights( mb::VertexMap* vmap, INode* node, ISkin* skin, ISkinContextData* skincx )
{
	for ( int i = 0 ; i < skincx->GetNumPoints() ; ++i )
	{
		int bonec = skincx->GetNumAssignedBones(i);
		for ( int k = 0 ; k < bonec ; ++k )
		{
			int boneidx = skincx->GetAssignedBone( i, k );
			INode* bone = skin->GetBone( boneidx );
			require( bone );
			if ( bone == node )
			{
				float w = skincx->GetBoneWeight( i, k );
				if ( w > 0.f )
				{
					int oldsize = vmap->size();
					//Debug::println( "    found vertex using bone {0}", String(node->GetName()) );
					vmap->addValue( i, &w, 1 );
					require( vmap->size() == oldsize+1 );
				}
			}
		}
	}
}

Matrix3 SkinExportUtil::getInitBoneTM( INode* node, ISkin* skin )
{
	Matrix3 tm;
	int ok = skin->GetBoneInitTM( node, tm );
	require( ok == SKIN_OK );
	return tm;
	//return node->GetNodeTM(0);
}

Matrix3 SkinExportUtil::getInitSkinTM( INode* node, ISkin* skin )
{
	Matrix3 tm;
	int ok = skin->GetSkinInitTM( node, tm );
	require( ok == SKIN_OK );
	return tm;
	//return node->GetNodeTM(0);
}
