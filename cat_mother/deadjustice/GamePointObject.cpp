#include "GamePointObject.h"
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

GamePointObject::GamePointObject( int collideFlags ) :
	GameObject( 0, 0, 0, 0, 0 ),
	m_collideFlags( collideFlags ),
	m_objectToIgnore( 0 )
{
}

void GamePointObject::setObjectToIgnore( const GameObject* obj )
{
	m_objectToIgnore = obj;
}

void GamePointObject::checkCollisionsAgainstCell( const Vector3& start, const Vector3& delta, bsp::BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo )
{
	float t = 1.f;
	const BSPPolygon* cpoly = 0;
	BSPCollisionUtil::findLineIntersection( bsptree, start, delta, m_collideFlags, &t, &cpoly );

	if ( t < 1.f )
	{
		Vector3 end = start + delta * t;
		Vector4 pl = cpoly->plane();
		*cinfo = CollisionInfo( end, cinfo->positionCell(), collisionCell, cpoly, Vector3(pl.x,pl.y,pl.z), end, 0, collisionCell->bspTree() );
	}
}

void GamePointObject::checkCollisionsAgainstObjects( const Vector3& start, const Vector3& delta, const Vector<GameObjectDistance>& objects, CollisionInfo* cinfo )
{
	if ( m_collideFlags & CollisionInfo::COLLIDE_OBJECT )
	{
		for ( int i = 0 ; i < objects.size() ; ++i )
		{
			GameObject* obj = objects[i].object;
			if ( obj == m_objectToIgnore )
				continue;

			GameCharacter* character = dynamic_cast<GameCharacter*>( obj );
			if ( character )
			{
				if ( character->rootCollisionBox().findLineBoxIntersection(start,delta,0) )
				{
					for ( int k = 0 ; k < character->boneCollisionBoxes() ; ++k )
					{
						const BoneCollisionBox& box = character->getBoneCollisionBox(k);

						float t;
						if ( box.findLineBoxIntersection(start,delta,&t) )
						{
							Vector3 end = start + delta*t;
							Vector3 cpoint = end;
							*cinfo = CollisionInfo( end, obj->cell(), obj->cell(), 0, obj->forward(), cpoint, obj, 0 );
							return;
						}
					}
				}
				continue;
			}

			GameDynamicObject* dynamicObject = dynamic_cast<GameDynamicObject*>( obj );
			if ( dynamicObject )
			{
				if ( !dynamicObject->hidden() && 
					Intersection::findLineSphereIntersection(start, delta, dynamicObject->position(), dynamicObject->boundSphere(), 0) )
				{
					Matrix4x4 tm = dynamicObject->transform();
					Matrix4x4 invtm = tm.inverse();
					Vector3 bspStart = invtm.transform( start );
					Vector3 bspDelta = invtm.rotate( delta );

					float t = 1.f;
					const BSPPolygon* cpoly = 0;
					GameBSPTree* bsptree = dynamicObject->bspTree();
					BSPCollisionUtil::findLineIntersection( bsptree->root(), bspStart, bspDelta, CollisionInfo::COLLIDE_GEOMETRY_SOLID, &t, &cpoly );

					if ( t < 1.f )
					{
						Vector3 end = start + delta * t;
						Vector4 pl = cpoly->plane();
						Vector3 cnormal = tm.rotate( Vector3(pl.x,pl.y,pl.z) );
						*cinfo = CollisionInfo( end, dynamicObject->cell(), dynamicObject->cell(), cpoly, cnormal, end, dynamicObject, bsptree );
					}
				}
				continue;
			}
		}
	}
}

