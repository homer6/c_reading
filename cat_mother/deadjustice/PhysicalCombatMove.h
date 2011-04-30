#ifndef _PHYSICALCOMBATMOVE_H
#define _PHYSICALCOMBATMOVE_H

#include <lang/Object.h>
#include <lang/String.h>
#include <math/Vector3.h>
#include <util/Hashtable.h>

namespace sg {
	class Node; }

/**
 * @author Toni Aittoniemi
 */
class PhysicalCombatMove : 
	public lang::Object 
{
public:
	/** Physical combat flags */
	enum CombatMoveFlags
	{
		PHYSICAL_MOVE_STRIKE = 1,		// Punch/kick type of attack (explicitly excludes MOVE_HOLD)
		PHYSICAL_MOVE_LOCK = 2,			// Grab & hold type attack (explicitly excludes MOVE_STRIKE)
		PHYSICAL_MOVE_PRIMARY = 4,		// Whole body motion (explicitly excludes MOVE_SECONDARY)
		PHYSICAL_MOVE_SECONDARY = 8,	// Masked body motion (explicitly excludes MOVE_PRIMARY)
		PHYSICAL_MOVE_CONTACT = 16		// Contact move, can not be performed unless enemy in physical contact
	};

	/* Construct default move */
	PhysicalCombatMove();

	/**
	 * Initialize move 
	 * @param anim Name of animation
	 * @param flags move type, a combination of CombatMoveFlags
	 * @param start time in seconds when effect starts
	 * @param end time in seconds when effect ends
	 * @param height height from user's feet in meters at which the effect occurs
	 * @param force hit force vector in local character space. Length is amount damage, orientation is the direction of force
	 * @param attackSectorStartAngle Angle of attack sector start. (0=right, 90=forward, 180=left, 270=behind)
	 * @param attackSectorEndAngle Angle of attack sector end. (0=right, 90=forward, 180=left, 270=behind)
	 * @param attackReachDistance How far attack is effective.
	 */
	PhysicalCombatMove( const lang::String& anim, int flags, float start, float end, float height, const math::Vector3& force,
		float attackSectorStartAngle, float attackSectorEndAngle, float attackReachDistance );

	/* Set to true if move has hit it's target, false otherwise */
	void					setHit( bool value );
	
	/** Returns true if enemy is in valid sector for attack to cause any harm .*/
	bool					isInAttackSector( const math::Vector3& enemyPositionInAttackerSpace ) const;

	const lang::String&		animName() const;
	float					start() const;
	float					end() const;
	float					attackHeight() const;
	const math::Vector3&	forceVector() const;
	bool					hasHit() const;
	float					attackSectorStartAngle() const;
	float					attackSectorEndAngle() const;
	float					attackReachDistance() const;

	/**
	 * Shortcuts to flags
	 */
	bool			isStrike() const;
	bool			isLock() const;
	bool			isPrimary() const;
	bool			isSecondary() const;
	bool			isContact() const;

private:
	lang::String	m_anim;
	int				m_flags;
	float			m_startTime;
	float			m_endTime;
	float			m_height;
	math::Vector3	m_force;
	bool			m_hit;
	float			m_attackSectorStartAngle;
	float			m_attackSectorEndAngle;
	float			m_attackReachDistance;

	// Unallowed
	PhysicalCombatMove( const PhysicalCombatMove& other );
	PhysicalCombatMove& operator=( const PhysicalCombatMove& other );
};


#endif