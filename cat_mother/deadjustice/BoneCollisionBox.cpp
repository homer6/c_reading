#include "BoneCollisionBox.h"
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/VertexAndIndexLock.h>
#include <sgu/NodeUtil.h>
#include <lang/Math.h>
#include <lang/Exception.h>
#include <math/Intersection.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace sgu;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

BoneCollisionBox::BoneCollisionBox() :
	m_bone(0),
	m_min(0,0,0),
	m_max(0,0,0),
	m_damageMultiplier(1.f)
{
}

BoneCollisionBox::BoneCollisionBox( sg::Node* root, sg::Node* bone, float damageMultiplier ) :
	m_bone(bone),
	m_min(0,0,0),
	m_max(0,0,0),
	m_damageMultiplier(damageMultiplier)
{
	// find all vertices which are most influenced by this bone
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		
		if ( mesh && mesh->bones() > 0 )
		{
			for ( int k = 0 ; k < mesh->bones() ; ++k )
			{
				if ( mesh->getBone(k) == m_bone )
				{
					const Matrix4x4& skinToBoneTm = mesh->getBoneInverseRestTransform( k );

					for ( int i = 0 ; i < mesh->primitives() ; ++i )
					{
						Model* model = dynamic_cast<Model*>( mesh->getPrimitive(i) );
						
						if ( model && model->vertexFormat().weights() > 0 )
						{
							VertexAndIndexLock<Model> lk( model, Model::LOCK_READ );
							
							int boneIndex = -1;
							for ( int j = 0 ; j < model->usedBones() ; ++j )
							{
								if ( model->usedBoneArray()[j]-1 == k )
								{
									boneIndex = j;
									break;
								}
							}

							if ( boneIndex >= 0 )
							{
								for ( int j = 0 ; j < model->vertices() ; ++j )
								{
									Vector3 pos;
									model->getVertexPositions( j, &pos );

									int bones[4];
									float weights[4];
									model->getVertexWeights( j, bones, weights, 4 );

									if ( bones[0] == boneIndex )
									{
										Vector3 bonePos;
										skinToBoneTm.transform( pos, &bonePos );
										mergePoint( bonePos );
									}
								}
							}
						}
					}
					break;
				}
			}
		}
	}
}

Node* BoneCollisionBox::bone() const
{
	assert( m_bone );
	return m_bone;
}

const Vector3& BoneCollisionBox::boxMin() const
{
	return m_min;
}

const Vector3& BoneCollisionBox::boxMax() const
{
	return m_max;
}

void BoneCollisionBox::setBox( const Vector3& boxMin, const Vector3& boxMax )
{
	m_min = boxMin;
	m_max = boxMax;
}

void BoneCollisionBox::mergePoint( const math::Vector3& point )
{
	for ( int n = 0 ; n < 3 ; ++n )
	{
		float d = point[n];
		if ( d > m_max[n] )
			m_max[n] = d;
		if ( d < m_min[n] )
			m_min[n] = d;
	}
}

bool BoneCollisionBox::findLineBoxIntersection( const math::Vector3& start, const math::Vector3& delta, float* t ) const
{
	assert( m_bone );

	const Matrix4x4& boxtm = m_bone->worldTransform();
	return Intersection::findLineBoxIntersection( start, delta, boxtm, m_min, m_max, t, 0 );
}

float BoneCollisionBox::damageMultiplier() const
{
	return m_damageMultiplier;
}

bool BoneCollisionBox::hasBone() const
{
	return m_bone != 0;
}
