#ifndef _CONTROLBASE_H
#define _CONTROLBASE_H


#include "GameScriptable.h"


namespace math {
	class Vector3;}

class GameCharacter;


/** 
 * Base class for computer and user control. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ControlBase :
	public GameScriptable
{
public:
	/** Value of control which is considered to be zero. */
	static const float ZERO_CONTROL_VALUE;

	ControlBase( script::VM* vm, io::InputStreamArchive* arch, GameCharacter* owner );

	/**
	 * Returns character velocity based on control vector direction and length, X=right, Z=forward.
	 * @param movementControl Control vector inside unit circle, X=right, Z=forward.
	 * @param movement [out] Receives velocity in character space.
	 * @param velocity [out] Receives velocity in world space.
	 * @param adjustedMovementControl [out] Receives movement control vector adjusted by strafing/backward movement limits.
	 */
	void	getVelocityFromMovementControlVector( const math::Vector3& movementControl,
				math::Vector3* movement, math::Vector3* velocity, math::Vector3* adjustedMovementControl ) const;

	/**
	 * Returns character velocity based on scaled control vector direction and length, X=right, Z=forward.
	 * Use scaleMovementControlVector for scaling.
	 * @param movementControlVector Scaled control vector inside unit circle, X=right, Z=forward.
	 * @param movement [out] Receives velocity in character space.
	 * @param velocity [out] Receives velocity in world space.
	 */
	void	getVelocityFromScaledMovementControlVector( const math::Vector3& movementControlVector,
				math::Vector3* movement, math::Vector3* velocity ) const;

	/** 
	 * Scales movement (sneak/walk/run/crouched) control vector by strafing/backward control limits.
	 * @param movementControlVector Vector inside unit circle. Forward is (0,0,1), right (1,0,0).
	 */
	math::Vector3	scaleMovementControlVector( const math::Vector3& movementControlVector ) const;

private:
	GameCharacter*	m_owner;

	ControlBase( const ControlBase& );
	ControlBase& operator=( const ControlBase& );
};


#endif // _CONTROLBASE_H
