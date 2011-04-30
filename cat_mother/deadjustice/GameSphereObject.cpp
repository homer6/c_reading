#include "GameSphereObject.h"
#include "GameLevel.h"
#include "GameBSPTree.h"
#include "CollisionInfo.h"
#include "GameCharacter.h"
#include <bsp/BSPCollisionUtil.h>
#include <bsp/BSPPolygon.h>
#include <math/Intersection.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace bsp;
using namespace math;
using namespace util;

//-----------------------------------------------------------------------------

GameSphereObject::GameSphereObject( int collideFlags ) :
	GameObject( 0, 0, 0, 0, 0 ),
	m_collideFlags( collideFlags ),
	m_objectToIgnore( 0 )
{
}

void GameSphereObject::setObjectToIgnore( const GameObject* obj )
{
	m_objectToIgnore = obj;
}

void GameSphereObject::checkCollisionsAgainstCell( const Vector3& start, const Vector3& delta, BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo )
{
	float margin = 0.01f;
	float r = boundSphere();
	
	float t = 1.f;
	const BSPPolygon* cpoly = 0;
	Vector3 cnormal( 0, 0, 0 );
	Vector3 cpoint( 0, 0, 0 );
	Vector3 shiftedStart = start;
	BSPCollisionUtil::findMovingSphereIntersection( bsptree, shiftedStart, delta, r, CollisionInfo::COLLIDE_ALL, &t, &cpoly, &cnormal, &cpoint );

	if ( t < 1.f )
	{
		Vector3 shiftedEnd = shiftedStart + delta*t;
		Vector3 end = start + delta*t;

		float inside = r + margin - (shiftedEnd - cpoint).dot( cnormal );
		if ( inside > 0.f )
		{
			end += cnormal * inside;
		}

		*cinfo = CollisionInfo( end, cinfo->positionCell(), collisionCell, cpoly, cnormal, cpoint, 0, collisionCell->bspTree() );
	}
}

void GameSphereObject::checkCollisionsAgainstObjects( const Vector3& start, const Vector3& delta, const Vector<GameObjectDistance>& objects, CollisionInfo* cinfo )
{
	float margin = 0.01f;
	Vector3 shiftedStart = start;

	for ( int i = 0 ; i < objects.size() ; ++i )
	{
		GameObject* obj = objects[i].object;

		GameCharacter* character = dynamic_cast<GameCharacter*>( obj );
		if ( character && character != m_objectToIgnore )
		{
			Vector3 distv = start - character->position();
			float t;
			if ( !GameCharacter::isDead(character->primaryState()) &&
				delta.dot(distv) < 0.f && // moving towards each other?
				Intersection::findLineSphereIntersection( start, delta, character->position(), boundSphere()+character->characterCollisionRadius(), &t ) )
			{
				Vector3 cpos = start + delta*t;
				Vector3 cnormal = (cpos - character->position()).normalize();
				*cinfo = CollisionInfo( cpos+cnormal*0.01f, cinfo->positionCell(), character->cell(), 0, cnormal, cpos-cnormal*boundSphere(), character, 0 );
			}
			continue;
		}

		GameDynamicObject* dynamicObject = dynamic_cast<GameDynamicObject*>( obj );
		if ( dynamicObject && !dynamicObject->hidden() &&
			Intersection::findLineSphereIntersection(shiftedStart, delta, dynamicObject->position(), boundSphere()+dynamicObject->boundSphere(), 0) )
		{
			Matrix4x4 tm = dynamicObject->transform();
			Matrix4x4 invtm = tm.inverse();
			Vector3 bspStart = invtm.transform( shiftedStart );
			Vector3 bspDelta = invtm.rotate( delta );

			float t = 1.f;
			float r = boundSphere();
			const BSPPolygon* cpoly = 0;
			Vector3 cnormal( 0, 0, 0 );
			Vector3 cpoint( 0, 0, 0 );
			GameBSPTree* bsptree = dynamicObject->bspTree();
			BSPCollisionUtil::findMovingSphereIntersection( bsptree->root(), bspStart, bspDelta, r, CollisionInfo::COLLIDE_ALL, &t, &cpoly, &cnormal, &cpoint );

			if ( t < 1.f )
			{
				cnormal = tm.rotate( cnormal );
				cpoint = tm.transform( cpoint );
				Vector3 shiftedEnd = shiftedStart + delta*t;
				Vector3 end = start + delta*t;

				float inside = r + margin - (shiftedEnd - cpoint).dot( cnormal );
				if ( inside > 0.f )
				{
					end += cnormal * inside;
				}

				*cinfo = CollisionInfo( end, dynamicObject->cell(), dynamicObject->cell(), cpoly, cnormal, cpoint, dynamicObject, bsptree );
			}

			continue;
		}
	}
}
