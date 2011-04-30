#include "CollisionInfo.h"
#include <bsp/BSPPolygon.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

CollisionInfo::CollisionInfo() :
	m_collisionMask( COLLIDE_NONE ),
	m_pos( 0,0,0 ),
	m_positionCell( 0 ),
	m_collisionCell( 0 ),
	m_poly( 0 ),
	m_normal( 0,0,0 ),
	m_point( 0,0,0 ),
	m_obj( 0 )
{
}

CollisionInfo::CollisionInfo( const Vector3& pos, GameCell* cell ) :
	m_collisionMask( COLLIDE_NONE ),
	m_pos( pos ),
	m_positionCell( cell ),
	m_collisionCell( 0 ),
	m_poly( 0 ),
	m_normal( 0,0,0 ),
	m_point( 0,0,0 ),
	m_obj( 0 )
{
}

CollisionInfo::CollisionInfo( const Vector3& pos, 
	GameCell* positionCell, GameCell* collisionCell,
	const bsp::BSPPolygon* poly, const math::Vector3& normal, const math::Vector3& point,
	GameObject* obj, GameBSPTree* bsptree ) :
	m_collisionMask( COLLIDE_NONE ),
	m_pos( pos ),
	m_positionCell( positionCell ),
	m_collisionCell( collisionCell ),
	m_poly( poly ),
	m_normal( normal ),
	m_point( point ),
	m_obj( obj ),
	m_bsptree( bsptree )
{
	if ( obj )
		m_collisionMask |= COLLIDE_OBJECT;
	if ( poly )
		m_collisionMask |= poly->collisionMask();
	if ( bsptree )
		m_collisionMask |= COLLIDE_BSP;

	assert( !(m_collisionMask & COLLIDE_OBJECT) || m_obj );
}

const Vector3& CollisionInfo::position() const 
{
	return m_pos;
}

GameCell* CollisionInfo::positionCell() const 
{
	return m_positionCell;
}

GameCell* CollisionInfo::collisionCell() const 
{
	assert( m_collisionMask != COLLIDE_NONE );
	return m_collisionCell;
}

const Vector3& CollisionInfo::normal() const 
{
	assert( m_collisionMask != COLLIDE_NONE );
	return m_normal;
}

const Vector3& CollisionInfo::point() const 
{
	assert( m_collisionMask != COLLIDE_NONE );
	return m_point;
}

const bsp::BSPPolygon* CollisionInfo::polygon() const
{
	assert( (m_collisionMask & COLLIDE_GEOMETRY_SEETHROUGH) || (m_collisionMask & COLLIDE_GEOMETRY_SOLID) );
	assert( m_poly );
	return m_poly;
}

GameObject* CollisionInfo::object() const
{
	assert( m_collisionMask & COLLIDE_OBJECT );
	assert( m_obj );
	return m_obj;
}

bool CollisionInfo::isCollision( CollisionType type ) const
{
	return 0 != (type & m_collisionMask);
}

GameBSPTree* CollisionInfo::bspTree() const
{
	return m_bsptree;
}
