#include "StdAfx.h"
#include "SgUtil.h"
#include "SgNode.h"
#include "SgMesh.h"
#include "SgLight.h"
#include "SgCamera.h"
#include "GmUtil.h"
#include "TmUtil.h"
#include "LODUtil.h"
#include "PhyExportUtil.h"
#include "SkinExportUtil.h"
#include "SceneExportUtil.h"
#include "DebugUtil.h"
#include <io/IOException.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/Format.h>
#include <util/Vector.h>
#include <math/Matrix4x4.h>
#include <memory>
#include <dummy.h>
#include <plugapi.h>
#ifdef SGEXPORT_PHYSIQUE
#include <BipExp.h>
#endif // SGEXPORT_PHYSIQUE

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

static const Matrix3	s_convtm	= Matrix3( Point3(1,0,0), Point3(0,1,0), Point3(0,0,-1), Point3(0,0,0) );

//-----------------------------------------------------------------------------

static void initNode( SgNode* obj, INode* node, Interval animRange )
{
	require( node );
	require( node->GetName() );

	// simple properties
	obj->name = node->GetName();
	obj->castShadows = (0 != node->CastShadows());
	obj->recvShadows = (0 != node->RcvShadows());
	obj->renderable = (0 != node->Renderable());
}

static pix::Colorf toColorf( const Color& c )
{
	return pix::Colorf( c.r, c.g, c.b );
}

//-----------------------------------------------------------------------------

P(SgMesh) SgUtil::createMesh( INode* node, Interval animRange )
{
	P(SgMesh) obj = new SgMesh;
	initNode( obj, node, animRange );

	LODUtil::getLODMemberInfo( node, &obj->lodID, &obj->lodMin, &obj->lodMax );
	if ( obj->lodID != "" )
		Debug::println( "Mesh {0} (size {2,#} pixels) is member of LOD group {1}", obj->name, obj->lodID, obj->lodMax );
	
	return obj;
}

P(SgLight) SgUtil::createLight( INode* node, Interval animRange )
{
	P(SgLight) obj = new SgLight;
	initNode( obj, node, animRange );

	TimeValue time = 0;
	ObjectState os = node->EvalWorldState( time );
	require( os.obj );
	require( os.obj->SuperClassID() == LIGHT_CLASS_ID );
	GenLight* lob = static_cast<GenLight*>( os.obj );

	Interval valid( time, time );
	LightState ls;
	int evalret = lob->EvalLightState( time, valid, &ls );
	require( REF_SUCCEED == evalret );

	switch ( ls.type )
	{
	case OMNI_LGT:		obj->type = SgLight::LIGHT_POINT; break;
	case DIRECT_LGT:	obj->type = SgLight::LIGHT_DIRECT; break;
	case SPOT_LGT:		obj->type = SgLight::LIGHT_SPOT; break;
	case AMBIENT_LGT:	obj->type = SgLight::LIGHT_AMBIENT; break;
	default:			throw Exception( Format("Unknown light type: {0}", lob->Type()) );
	}

	obj->on				= (0 != ls.on);
	obj->color			= toColorf( ls.color );
	obj->intensity		= ls.intens;
	if ( ls.type == SPOT_LGT )
	{
		obj->hotsize		= ls.hotsize;
		obj->fallsize		= ls.fallsize;
	}
	obj->farAtten		= (0 != ls.useAtten);
	obj->farAttenStart	= ls.attenStart;
	obj->farAttenEnd	= ls.attenEnd;
	obj->nearAtten		= (0 != ls.useNearAtten);
	obj->nearAttenStart	= ls.nearAttenStart;
	obj->nearAttenEnd	= ls.nearAttenEnd;
	obj->decay			= lob->GetDecayType();
	obj->decayRadius	= lob->GetDecayRadius( time );

	return obj;
}

P(SgCamera) SgUtil::createCamera( INode* node, Interval animRange )
{
	P(SgCamera) obj = new SgCamera;
	initNode( obj, node, animRange );

	TimeValue time = 0;
	ObjectState os = node->EvalWorldState( time );
	require( os.obj );
	require( os.obj->SuperClassID() == CAMERA_CLASS_ID );
	GenCamera* cob = static_cast<GenCamera*>( os.obj );

	Interval valid( time, time );
	CameraState cs;
	int evalret = cob->EvalCameraState( time, valid, &cs );
	require( REF_SUCCEED == evalret );
	
	obj->fov = cs.fov;
	return obj;
}

P(SgLOD) SgUtil::createLOD( INode* node, Interval animRange )
{
	P(SgLOD) obj = new SgLOD;
	initNode( obj, node, animRange );

	obj->lodID = LODUtil::getLODHeadID( node );

	if ( obj->lodID != "" )
		Debug::println( "Found LOD group: {0} (group head is {1})", obj->lodID, obj->name );

	return obj;
	
}

P(SgDummy) SgUtil::createDummy( INode* node, Interval animRange )
{
	P(SgDummy) obj = new SgDummy;
	initNode( obj, node, animRange );

	// get dummy box size
	ObjectState os = node->EvalWorldState( 0 );
	if ( os.obj && os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
	{
		DummyObject* dummy = static_cast<DummyObject*>(os.obj);

		Point3 boxMin = dummy->GetBox().pmin;
		Point3 boxMax = dummy->GetBox().pmax;
		Matrix3 vertexTM = TmUtil::getPivotTransform(node) * s_convtm;
		boxMin = vertexTM * boxMin;
		boxMax = vertexTM * boxMax;

		for ( int i = 0 ; i < 3 ; ++i )
		{
			float minv = Math::min( boxMin[i], boxMax[i] );
			float maxv = Math::max( boxMin[i], boxMax[i] );
			boxMin[i] = minv;
			boxMax[i] = maxv;
		}

		Debug::println( "Dummy object {0} boxMin/boxMax: {1} {2} {3} / {4} {5} {6}", (String)node->GetName(), boxMin.x, boxMin.y, boxMin.z, boxMax.x, boxMax.y, boxMax.z );

		obj->boxMin = Vector3(boxMin.x,boxMin.y,boxMin.z);
		obj->boxMax = Vector3(boxMax.x,boxMax.y,boxMax.z);
	}

	return obj;
}

/*P(SgShape) SgUtil::createShape( INode* node, Interval animRange )
{
	P(SgShape) obj = new SgShape;
	initNode( obj, node, animRange );

	// get shape points
	ObjectState os = node->EvalWorldState( 0 );
	if ( os.obj && os.obj->ClassID() == Class_ID(SHAPE_CLASS_ID,0) )
	{
		ShapeObject* shape = static_cast<ShapeObject*>( os.obj );
		
		PolyShape poly;
		shape->MakePolyShape( 0, poly, PSHAPE_BUILTIN_STEPS, FALSE );

		for ( int i = 0 ; i < shape->numLines ; ++i )
		{
			PolyLine& 
		}
	}

	return obj;
}*/

P(SgNode) SgUtil::createUnknown( INode* node, Interval animRange )
{
	P(SgNode) obj = new SgNode;
	initNode( obj, node, animRange );
	return obj;
}

void SgUtil::addBones( SgMesh* mesh, INode* node, const Vector<INode*>& allnodes )
{
	mesh->bones.clear();

	// add bones (from Physique modifier)
#ifdef SGEXPORT_PHYSIQUE
	Modifier* phyMod = PhyExportUtil::findPhysiqueModifier( node );
	if ( phyMod )
	{
		// get (possibly shared) Physique export interface
		IPhysiqueExport* phyExport = (IPhysiqueExport*)phyMod->GetInterface( I_PHYINTERFACE );
		require( phyExport );

		// export from initial pose?
		phyExport->SetInitialPose( false );

		// get (unique) context dependent export inteface
		IPhyContextExport* mcExport = (IPhyContextExport*)phyExport->GetContextInterface( node );
		if( !mcExport )
			throw Exception( Format("No Physique modifier context export interface") );

		// convert to rigid for time independent vertex assignment
		mcExport->ConvertToRigid( true );

		// allow blending to export multi-link assignments
		mcExport->AllowBlending( true );

		// list bone nodes
		Vector<INode*> bones( Allocator<INode*>(__FILE__,__LINE__) );
		PhyExportUtil::listBones( mcExport, bones );

		// add bone indices and rest transform to the mesh
		//Debug::println( "    Mesh {0} bones:", mesh->name );
		Matrix3 toSkin = Inverse( PhyExportUtil::getInitSkinTM(node,phyExport) );
		for ( int i = 0 ; i < bones.size() ; ++i )
		{
			INode* boneNode = bones[i];

			// rest = (bone -> skin) in non-deforming pose
			Matrix3 initTM = PhyExportUtil::getInitBoneTM( boneNode, phyExport );
			Matrix3 restTM = initTM * toSkin;
			Matrix4x4 rest = TmUtil::toLH( restTM );
			
			SgMesh::Bone bone;
			bone.index = allnodes.indexOf( boneNode );
			bone.rest = rest;
			require( -1 != bone.index );
			mesh->bones.add( bone );
			//Debug::println( "      bone {0,#}: {1,#}, {2}", i, bone.index, (String)boneNode->GetName() );
		}

		// cleanup Physique modifier export
		if ( mcExport )
		{
			require( phyExport );
			phyExport->ReleaseContextInterface( mcExport );
			mcExport = 0;
		}
		if ( phyExport )
		{
			require( phyMod );
			phyMod->ReleaseInterface( I_PHYINTERFACE, phyExport );
			phyExport = 0;
		}
	}
#endif // SGEXPORT_PHYSIQUE

	// add bones (from Skin modifier)
	Modifier* skinMod = SkinExportUtil::findSkinModifier( node );
	if ( skinMod )
	{
		// get (possibly shared) Skin interface
		ISkin* skin = (ISkin*)skinMod->GetInterface( I_SKIN );
		require( skin );

		// get skin context
		ISkinContextData* skincx = skin->GetContextInterface( node );
		require( skincx );

		// list bone nodes
		Vector<INode*> bones( Allocator<INode*>(__FILE__,__LINE__) );
		SkinExportUtil::listBones( skin, bones );

		// add bone indices and rest transform to the mesh
		Matrix3 toSkin = Inverse( SkinExportUtil::getInitSkinTM(node,skin) );
		for ( int i = 0 ; i < bones.size() ; ++i )
		{
			INode* boneNode = bones[i];

			// rest = (bone -> skin) in non-deforming pose
			Matrix3 initTM = SkinExportUtil::getInitBoneTM( boneNode, skin );
			Matrix3 restTM = initTM * toSkin;
			Matrix4x4 rest = TmUtil::toLH( restTM );
			
			SgMesh::Bone bone;
			bone.index = allnodes.indexOf( boneNode );
			bone.rest = rest;
			require( -1 != bone.index );
			mesh->bones.add( bone );
		}

		// cleanup Skin modifier export
		if ( skin )
		{
			require( skinMod );
			skinMod->ReleaseInterface( I_SKIN, skin );
			skin = 0;
		}
	}
}
