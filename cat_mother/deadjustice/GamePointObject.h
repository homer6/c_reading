#ifndef _GAMEPOINTOBJECT_H
#define _GAMEPOINTOBJECT_H


#include "GameObject.h"


/** 
 * Helper class for doing point collisions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GamePointObject :
	public GameObject
{
public:
	/** 
	 * Create point collision object.
	 * @param collideFlags Selects which objects to check. See CollisionInfo::CollisionType.
	 */
	explicit GamePointObject( int collideFlags );

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

	GamePointObject( const GamePointObject& );
	GamePointObject& operator=( const GamePointObject& );
};


#endif // _GAMEPOINTOBJECT_H
