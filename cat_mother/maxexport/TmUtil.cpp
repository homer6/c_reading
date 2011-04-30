#include "StdAfx.h"
#include "TmUtil.h"
#include "BipedUtil.h"
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Exception.h>

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

/** 3DS to left handed coordinate system conversion matrix. */
static const Matrix3	s_convtm( Point3(1,0,0), Point3(0,1,0), Point3(0,0,-1), Point3(0,0,0) );

//-----------------------------------------------------------------------------

/** Returns unprocessed model-to-world transform. */
static Matrix3 getNodeTM( INode* node, TimeValue t )
{
	Matrix3 tm = node->GetNodeTM( t );

	if ( node->IsRootNode() )
	{
		const float HALF_PI = 1.570796327f;
		Matrix3 rotx90 = RotateXMatrix( HALF_PI );
		tm *= rotx90;
	}

	return tm;
}

//-----------------------------------------------------------------------------

Matrix4x4 TmUtil::getModelToParentLH( INode* node, TimeValue t )
{
	Matrix3 tm = getNodeTM( node, t );
	
	INode* parent = node->GetParentNode();
	if ( parent )
	{
		// WORKAROUND: re-parent BipXX Spine to root instead of 
		// BipXX Pelvis to break dependency between lower body and upper body
		if ( BipedUtil::isBipedName(node->GetName(),"Spine") && parent &&
			BipedUtil::isBipedName(parent->GetName(),"Pelvis") )
		{
			parent = parent->GetParentNode();
			if ( !parent )
				throw Exception( Format("{0} must have two parent levels", node->GetName()) );

			if ( parent->IsRootNode() )
			{
				// hierarchy: (BipXX->BipXX Pelvis)->BipXX Spine
				const float HALF_PI = 1.570796327f;
				Matrix3 rotx90 = RotateXMatrix( -HALF_PI );
				tm *= rotx90;
				return toLH( tm );
			}
			else
			{
				// hierarchy: MASTER_CTRL->(BipXX->BipXX Pelvis)->BipXX Spine
				parent = parent->GetParentNode();
			}
		}

		tm = tm * Inverse( getNodeTM(parent,t) );
	}

	return toLH( tm );
}

Matrix3 TmUtil::getPivotTransform( INode* node )
{
	Matrix3 pivot(1);
	pivot.PreTranslate( node->GetObjOffsetPos() );
	PreRotateMatrix( pivot, node->GetObjOffsetRot() );
	ApplyScaling( pivot, node->GetObjOffsetScale() );
	return pivot;
}

bool TmUtil::hasNegativeParity( const Matrix3& m )
{
	return DotProd( CrossProd(m.GetRow(0), m.GetRow(1)), m.GetRow(2) ) < 0.f;
}

Matrix4x4 TmUtil::toLH( const Matrix3& tm )
{
	Matrix3 m0 = s_convtm * tm * s_convtm;
	Matrix4x4 m;
	m.setColumn( 0, Vector4(m0.GetRow(0).x,m0.GetRow(0).y,m0.GetRow(0).z,0.f) );
	m.setColumn( 1, Vector4(m0.GetRow(1).x,m0.GetRow(1).y,m0.GetRow(1).z,0.f) );
	m.setColumn( 2, Vector4(m0.GetRow(2).x,m0.GetRow(2).y,m0.GetRow(2).z,0.f) );
	m.setColumn( 3, Vector4(m0.GetRow(3).x,m0.GetRow(3).y,m0.GetRow(3).z,1.f) );
	return m;
}

void TmUtil::println( const Matrix3& tm, int margin )
{
	String str;
	for ( int i = 0 ; i < margin ; ++i )
		str = str + " ";
	for ( int k = 0 ; k < 4 ; ++k )
		Debug::println( "{3}{0,#.###} {1,#.###} {2,#.###}", tm.GetRow(k).x, tm.GetRow(k).y, tm.GetRow(k).z, str );
}
