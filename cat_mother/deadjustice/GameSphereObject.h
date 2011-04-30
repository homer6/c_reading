#ifndef _GAMESPHEREOBJECT_H
#define _GAMESPHEREOBJECT_H


#include "GameObject.h"


/** 
 * Helper class for doing point collisions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameSphereObject :
	public GameObject
{
public:
	/** 
	 * Create point collision object.
	 * @param collideFlags Selects which objects to check. See CollisionInfo::CollisionType.
	 */
	explicit GameSphereObject( int collideFlags );

	/** Sets object to be ignored in collision checks. */
	void	setObjectToIgnore( const GameObject* obj );

	/** Checks collision of this object against BSP tree. */
	void	checkCollisionsAgainstCell( const math::Vector3& start, const math::Vector3& delta, bsp::BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo );

	/** 
	 * Checks collisions against dynamic objects. 
	 * Updates CollisionInfo only if collision happens.
	 */
	void	checkCollisionsAgainstObjects( const math::Vector3& start, const math::Vector3& delta, const util::Vector<GameObjectDistance>& objects, CollisionInfo* cinfo );

private:
	int						m_collideFlags;
	const GameObject*		m_objectToIgnore;

	GameSphereObject( const GameSphereObject& );
	GameSphereObject& operator=( const GameSphereObject& );
};


#endif // _GAMESPHEREOBJECT_H
