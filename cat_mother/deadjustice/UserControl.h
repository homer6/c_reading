#ifndef _USERCONTROL_H
#define _USERCONTROL_H


#include "ControlBase.h"
#include "GameCharacter.h"
#include <math/Vector3.h>
#include <math/Matrix3x3.h>


namespace script {
	class VM; }

namespace io {
	class InputStreamArchive; }


class GameCharacter;


/** 
 * Provides interface for controlling game character by user input. 
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class UserControl :
	public ControlBase
{
public:
	UserControl( script::VM* vm, io::InputStreamArchive* arch, GameCharacter* owner );

	/** Player acceleration. */
	void		accelerate( float value );

	/** Player acceleration backwards. */
	void		accelerateBackwards( float value );

	/** Rotate player right.  */
	void		turnRight( float value );

	/** Rotate player left. */
	void		turnLeft( float value );

	/** Jump (if on ground). */
	void		jump( bool active );

	/** Attack. */
	void		attack( bool active );

	/** Strafe Left. */
	void		strafeLeft( float value );

	/** Strafe Right. */
	void		strafeRight( float value );

	/** Roll. */
	void		roll( float value );

	/** Crouch down. */
	void		crouch( bool active );

	/** Peek left. */
	void		peekLeft( float value );

	/** Peek left. */
	void		peekRight( float value );

	/** Attack physically by striking with weapon/fists. */
	void		physicalAttackStrike( bool active );

	/** Attack physically by kicking. */
	void		physicalAttackKick( bool active );

	/** Forced change clip. */
	void		changeClip( bool active );

	/** Cycle Weapon. */
	void		cycleWeapon( bool active );

	/** Create diversion by throwing an empty shell. */
	void		throwEmptyShell( bool active );

	/** Force running. */
	void		runModifier( bool active );

	/** Force sneaking. */
	void		sneakModifier( bool active );

	/** Reset input data. */
	void		resetInputState();

	/** Evaluates next player primary state. */
	GameCharacter::PrimaryState		evaluatePrimaryState( GameCharacter::PrimaryState lastState );

	/** Evaluates next player secondary state. */
	GameCharacter::SecondaryState	evaluateSecondaryState( GameCharacter::SecondaryState lastState );

	/** Adjusts velocity according to input. */
	void		adjustVelocity( float dt, const math::Vector3& forward, const math::Vector3& right,
					math::Vector3* velocity, math::Vector3* movement, math::Vector3* movementControl );

	/** Adjusts rotation according to input. */
	void		adjustRotation( math::Matrix3x3* rot, float angleDelta );

	/** Shortcut to input */
	float		accelerating() const; 

	/** Shortcut to input */
	float		acceleratingBackwards() const; 

	/** Shortcut to input */
	float		turningRight() const; 

	/** Shortcut to input */
	float		turningLeft() const; 

	/** Shortcut to input */
	float		strafingRight() const; 

	/** Shortcut to input */
	float		strafingLeft() const; 

	/** Shortcut to input */
	float		attacking() const; 

	/** Shortcut to input */
	bool		cycleWeapon() const; 
		
	/** Returns vector computed from movement controller input state. */
	math::Vector3	movementControlVector() const;

private:
	int									m_methodBase;
	static ScriptMethod<UserControl>	sm_methods[];

	float		m_accelerating;
	float		m_acceleratingBackwards;
	float		m_turningRight;
	float		m_turningLeft;
	bool		m_jumping;
	bool		m_attacking;
	float		m_strafingLeft;
	float		m_strafingRight;
	float		m_rolling;
	bool		m_crouching;
	bool		m_physicalAttackingStrike;
	bool		m_physicalAttackingKick;
	float		m_peekingLeft;
	float		m_peekingRight;
	bool		m_readyToSingleShot;
	bool		m_readyToChangeClip;
	bool		m_readyToPhysicalAttackStrike;
	bool		m_readyToPhysicalAttackKick;
	bool		m_readyToRoll;
	bool		m_changeClip;
	bool		m_cycleWeapon;
	bool		m_throwEmptyShell;
	bool		m_runModifier;
	bool		m_sneakModifier;

	GameCharacter*	m_owner;
	math::Vector3	m_movementControlVector;

	/** Movement control vector computed directly from input state. */
	math::Vector3	inputMovementControlVector() const;

	int		methodCall( script::VM* vm, int i );
	int		script_isTurning( script::VM* vm, const char* funcName );

	UserControl( const UserControl& other );
	UserControl& operator= ( const UserControl& other );
};


#endif
