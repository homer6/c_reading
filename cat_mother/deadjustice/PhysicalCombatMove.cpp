#include "PhysicalCombatMove.h"
#include <lang/Math.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

PhysicalCombatMove::PhysicalCombatMove() :
	m_anim(""),
	m_flags(PHYSICAL_MOVE_STRIKE|PHYSICAL_MOVE_SECONDARY),
	m_startTime(0),
	m_endTime(0),
	m_height(0),
	m_force(Vector3(0,0,0)),
	m_hit(false),
	m_attackSectorStartAngle(0.f),
	m_attackSectorEndAngle(0.f),
	m_attackReachDistance(0.f)
{
}

PhysicalCombatMove::PhysicalCombatMove( const String& anim, int flags, float start, float end, float height, 
	const Vector3& force, float attackSectorStartAngle, float attackSectorEndAngle, float attackReachDistance ) :
	m_anim(anim),
	m_flags(flags),
	m_startTime(start),
	m_endTime(end),
	m_height(height),
	m_force(force),
	m_hit(false),
	m_attackSectorStartAngle(attackSectorStartAngle),
	m_attackSectorEndAngle(attackSectorEndAngle),
	m_attackReachDistance(attackReachDistance)
{
}

void PhysicalCombatMove::setHit( bool value )					
{ 
	m_hit = value; 
}

const String&	PhysicalCombatMove::animName() const
{ 
	return m_anim; 
}

float PhysicalCombatMove::start() const
{ 
	return m_startTime; 
}

float PhysicalCombatMove::end() const
{ 
	return m_endTime;
}

float PhysicalCombatMove::attackHeight() const
{ 
	return m_height;
}

const Vector3&	PhysicalCombatMove::forceVector() const
{ 
	return m_force;
}

bool PhysicalCombatMove::hasHit() const	
{ 
	return m_hit; 
}

bool PhysicalCombatMove::isStrike() const		
{ 
	return (m_flags & PHYSICAL_MOVE_STRIKE) != 0;
}

bool PhysicalCombatMove::isLock() const
{ 
	return (m_flags & PHYSICAL_MOVE_LOCK) != 0; 
}

bool PhysicalCombatMove::isPrimary() const
{ 
	return (m_flags & PHYSICAL_MOVE_PRIMARY) != 0; 
}

bool PhysicalCombatMove::isSecondary() const
{ 
	return (m_flags & PHYSICAL_MOVE_SECONDARY) != 0; 
}

bool PhysicalCombatMove::isContact() const		
{ 
	return (m_flags & PHYSICAL_MOVE_CONTACT) != 0; 
}

float PhysicalCombatMove::attackSectorStartAngle() const
{
	return m_attackSectorStartAngle;
}

float PhysicalCombatMove::attackSectorEndAngle() const
{
	return m_attackSectorEndAngle;
}

float PhysicalCombatMove::attackReachDistance() const
{
	return m_attackReachDistance;
}

bool PhysicalCombatMove::isInAttackSector( const Vector3& enemyPositionInAttackerSpace ) const
{
	// compute direction of enemy
	Vector3 dirv = enemyPositionInAttackerSpace.normalize();
	float dirAngle = Math::atan2( dirv.z, dirv.x );
	if ( dirAngle < 0.f )
		dirAngle += Math::PI*2.f;
	
	return dirAngle >= m_attackSectorStartAngle && dirAngle <= m_attackSectorEndAngle;
}
