#ifndef _COLLISIONINFO_H
#define _COLLISIONINFO_H


#include <math/Vector3.h>
#include <math/Vector4.h>


namespace bsp {
	class BSPPolygon;}

class GameCell;
class GameObject;
class GameBSPTree;


/** 
 * Information about collision event. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class CollisionInfo
{
public:
	/** Flags for collision mask. */
	enum CollisionType
	{
		/** Collide nothing. */
		COLLIDE_NONE				= 0,
		/** Collision against geometry. */
		COLLIDE_GEOMETRY_SOLID		= (1<<0),
		/** Collision against geometry which can be seen through. */
		COLLIDE_GEOMETRY_SEETHROUGH	= (1<<1),
		/** Collision against dynamic object. */
		COLLIDE_OBJECT				= (1<<2),
		/** Collision against BSP tree (dynamic object or cell). */
		COLLIDE_BSP					= (1<<3),
		/** Collide anything. */
		COLLIDE_ALL					= -1
	};

	/** No collision information. */
	CollisionInfo();

	/** No collision. */
	explicit CollisionInfo( const math::Vector3& pos, GameCell* cell );

	/** 
	 * Collision at specified node. 
	 * @param pos Object position at the end of movement.
	 * @param positionCell Cell at the end of movement.
	 * @param collisionCell Cell at the collision point.
	 * @param poly Collision polygon.
	 * @param normal Collision plane normal.
	 * @param point Collision plane point.
	 * @param obj Collision object if any.
	 * @param bsptree Collision BSP tree if any.
	 */
	CollisionInfo( const math::Vector3& pos, 
		GameCell* positionCell, GameCell* collisionCell,
		const bsp::BSPPolygon* poly, const math::Vector3& normal, const math::Vector3& point,
		GameObject* obj, GameBSPTree* bsptree );

	/** Returns true if specified kind of collision occured. Pass COLLIDE_ALL for any kind of collision. */
	bool					isCollision( CollisionType type ) const;

	/** Returns position at the end of movement. */
	const math::Vector3&	position() const;

	/** Returns cell at the end of movement. */
	GameCell*				positionCell() const;

	/** Returns cell at the collision point if collision() != COLLIDE_NONE. */
	GameCell*				collisionCell() const;

	/** Returns collision normal collided against if collision() != COLLIDE_NONE. */
	const math::Vector3&	normal() const;

	/** Returns collision point collided against if collision() != COLLIDE_NONE. */
	const math::Vector3&	point() const;

	/** Returns collision polygon if collision() == COLLIDE_GEOMETRY. */
	const bsp::BSPPolygon*	polygon() const;

	/** Returns collision object if collision() == COLLIDE_OBJECT. */
	GameObject*				object() const;

	/** Returns collision BSP tree if collision() == COLLIDE_BSP. */
	GameBSPTree*			bspTree() const;

private:
	int						m_collisionMask;
	math::Vector3			m_pos;
	GameCell*				m_positionCell;
	GameCell*				m_collisionCell;
	const bsp::BSPPolygon*	m_poly;
	math::Vector3			m_normal;
	math::Vector3			m_point;
	GameObject*				m_obj;
	GameBSPTree*			m_bsptree;
};


#endif // _COLLISIONINFO_H
