#include "CameraUtil.h"
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/Camera.h>
#include <sg/VertexAndIndexLock.h>
#include <sg/VertexFormat.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <math/Vector3.h>
#include <math/Intersection.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace sgu
{


void CameraUtil::setFrontAndBackPlanes( Camera* camera, Node* scene )
{
	Vector3 camWt = camera->worldTransform().translation();
	float maxr = 0.f;
	for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh && mesh->enabled() )
		{
			float dist = (camWt - mesh->worldTransform().translation()).length();
			float primr = 0.f;
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				float r = mesh->getPrimitive(i)->boundSphere();
				assert( r < 1e9f );
				if ( r > primr )
					primr = r;
			}
			float r = dist + mesh->worldTransform().rotate( Vector3(primr,0.f,0.f) ).length();
			if ( r > maxr )
				maxr = r;
		}
	}

	float back = maxr*1.5f;
	if ( back < 1.f )
		back = 1.f;
	float front = back / 10000.f;
	if ( front < 1e-3f )
		front = 1e-3f;
	camera->setFront( front );
	camera->setBack( back );
}

void CameraUtil::convertZoomToFov( float zoom, float width, float height,
	float* hfov, float* vfov )
{
	assert( width > 1 );
	assert( height > 1 );
	assert( zoom > Float::MIN_VALUE );
	assert( hfov );
	assert( vfov );

	float frameAspect = width / height;
	*hfov = 2.f * Math::atan( frameAspect / zoom );
	*vfov = 2.f * Math::atan( 1.f / zoom );
}

sg::Node* CameraUtil::pick( Node* root, const Vector3& rayPos, 
	const Vector3& rayDir, float* distance )
{
	//Profile pr( "pick" );

	float minLen = Float::MAX_VALUE;
	Node* minLenNode = 0;

	Node* node = 0;
	for ( node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );

		if ( mesh && mesh->enabled() )
		{
			Matrix4x4	tminv = mesh->worldTransform().inverse();
			Vector3		dir = tminv.rotate( rayDir );
			Vector3		pos = tminv.transform( rayPos );
			Vector3		distv = pos - dir * pos.dot( dir );
			float		dist = distv.length();
			
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive( i );
				Model* model = dynamic_cast<Model*>( prim );

				if ( model && dist < model->boundSphere() &&
					0 == model->vertexFormat().weights() )
				{
					VertexAndIndexLock<Model> lock( model, Model::LOCK_READ );
					
					int face[3];
					Vector3 v[3];
					for ( int j = 0 ; j < model->indices() ; j += 3 )
					{
						// get face vertices
						model->getIndices( j, face, 3 );
						for ( int k = 0 ; k < 3 ; ++k ) 
							model->getVertexPositions( face[k], &v[k], 1 );

						// facing against ray?
						bool facing = false;
						bool valid = true;
						Vector3 edge1 = v[1] - v[0];
						Vector3 edge2 = v[2] - v[0];
						Vector3 normal = edge1.cross( edge2 );
						float n2 = normal.lengthSquared();
						if ( n2 > 1e-8f )
							facing = ( normal.dot(dir) < 0.f );
						else
							valid = false;
						
						if ( facing && valid )
						{
							float t = 0.f;
							if ( Intersection::findRayTriangleIntersection(pos,dir,v[0],v[1],v[2],&t) )
							{
								if ( t < minLen )
								{
									minLen = t;
									minLenNode = mesh;
								}
							}
						}
					} // for ( int j = 0 ; j < model->indices() ; j += 3 )
				}
			} // for ( int i = 0 ; i < mesh->primitives() ; ++i )
		}
	}

	if ( distance )
		*distance = minLen;
	return minLenNode;
}


} // sgu
