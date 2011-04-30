#ifndef _COMPUTERCONTROL_H
#define _COMPUTERCONTROL_H


#include "ControlBase.h"
#include "GameCharacter.h"
#include "ScriptUtil.h"
#include <math/Vector3.h>
#include <math/Matrix3x3.h>


namespace fsm {
	class StateMachine; }

namespace script {
	class VM; }

namespace io {
	class InputStreamArchive; }


class GameCharacter;
class GameObject;


/** 
 * Provides interface for controlling game character by computer generated input. 
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class ComputerControl :
	public ControlBase
{
public:
	/** Mode of moving */
	enum MoveMode {
		MOVE_SNEAKING,
		MOVE_WALKING,
		MOVE_RUNNING
	};

	ComputerControl( script::VM* vm, io::InputStreamArchive* arch, GameCharacter* owner );

	~ComputerControl();
	
	/** Reset input data. */
	void		resetInputState();

	/** Evaluates next player primary state. */
	GameCharacter::PrimaryState	evaluatePrimaryState( GameCharacter::PrimaryState lastState );

	/** Evaluates next player secondary state. */
	GameCharacter::SecondaryState	evaluateSecondaryState( GameCharacter::SecondaryState lastState );

	/** Adjusts velocity according to input. */
	void		adjustVelocity( math::Vector3* velocity, math::Vector3* movement, math::Vector3* movementControl );

	/** Adjusts rotation according to input. */
	void		adjustRotation( math::Matrix3x3* rot, float maxRotationSpeed, float dt );

	/** Adjusts aim vector for realistic inaccuracy. */
	void		adjustAim( math::Vector3* aimVector, float dt );

	/** Adjusts look vector for smooth transitions between head turning. */
	void		adjustLook( math::Vector3* lookVector, float dt );

	/** Attacks physically */
	void		physicalAttackStrike( bool active );
	
	/** Attacks physically */
	void		physicalAttackKick( bool active );

	/** Updates aim delay. */
	void		updateAimDelay( float dt );

	/** Returns true if character is ordered to fire its' weapon */
	bool	fire() const						{ return m_fire; }

	/** Returns moving target position. */
	const math::Vector3& goTo()	const			{ return m_goTo; }

	/** Returns fire target. */
	GameCharacter* fireTarget()	const			{ return m_fireTarget; }

	/** Returns firing accuracy. */
	float aimAccuracy()	const					{ return m_aimAccuracy; }

	/** Returns movement "sluggishness". */
	float movementSluggishness() const			{ return m_movementSluggishness; }

	/** Returns turning "sluggishness". */
	float turnSluggishness() const				{ return m_turnSluggishness; }

	/** Returns pointer to state machine. */
	fsm::StateMachine* stateMachine() const		{ return m_stateMachine; }

	/** Returns true if already looking at certain direction, with anglespeed used as a guide to accuracy of measurement. */
	bool	isLookingAt( float xangle, float yangle, float anglespeed ) const;

private:
	class LookTo {
	public:
		LookTo( float x, float y, float v )
		{
			xAngle = x;
			yAngle = y;
			angleSpeed = v;
		}

		float xAngle;
		float yAngle;
		float angleSpeed;
	};

	math::Vector3			m_goTo;
	math::Vector3			m_turnTo;
	float					m_aimChangeMaxDelta;
	float					m_movementSluggishness;
	float					m_turnSluggishness;
	float					m_aimAccuracy;
	GameCharacter*			m_fireTarget;
	int						m_jammedCount; // detect if computer player can't move to requested position
	math::Vector3			m_jammedPos;
	math::Vector3			m_lastMovementVector;
	float					m_aimingTimer;
	float					m_aimingDelay;

	// actions
	MoveMode				m_moveMode;
	bool					m_fire;
	bool					m_crouching;
	bool					m_rollBackward;
	bool					m_rollForward;
	bool					m_rollLeft;
	bool					m_rollRight;
	bool					m_physicalAttackStrike;
	bool					m_physicalAttackKick;
	bool					m_readyToPhysicalAttackStrike;
	bool					m_readyToPhysicalAttackKick;
	LookTo					m_lookTo;
	bool					m_fireOnce;

	// owner 
	GameCharacter*			m_owner;

	// state machine
	P(fsm::StateMachine)	m_stateMachine;

	// helpers
	/** Cancels goTo command if we can't move. */
	void					updateJammedMovementDetection();
	/** Use this instead of setting m_goTo directly -- resets jammed movement counter also. */
	void					goTo( const math::Vector3& pos );
	math::Vector3			perturbVector( const math::Vector3& src, float limit, const math::Vector3& up, const math::Vector3& right );
	static float			getAbsoluteAngleXZ( const math::Vector3& vec );
	static float			getAbsoluteAngleYZ( const math::Vector3& vec );

	// scripting 
	int										m_methodBase;
	static ScriptMethod<ComputerControl>	sm_methods[];

	// script methods
	int		methodCall( script::VM* vm, int i );
	int		script_crouch( script::VM* vm, const char* funcName );
	int		script_crouching( script::VM* vm, const char* funcName );
	int		script_fire( script::VM* vm, const char* funcName );
	int		script_fireOnce( script::VM* vm, const char* funcName );
	int		script_getFireTarget( script::VM* vm, const char* funcName );
	int		script_getFireState( script::VM* vm, const char* funcName );
	int		script_getStateMachine( script::VM* vm, const char* funcName );
	int		script_goTo( script::VM* vm, const char* funcName );
	int		script_goToOffset( script::VM* vm, const char* funcName );
	int		script_lookToAngle( script::VM* vm, const char* funcName );
	int		script_physicalAttackStrike( script::VM* vm, const char* funcName );
	int		script_physicalAttackKick( script::VM* vm, const char* funcName );
	int		script_rollBackward( script::VM* vm, const char* funcName );
	int		script_rollForward( script::VM* vm, const char* funcName );
	int		script_rollLeft( script::VM* vm, const char* funcName );
	int		script_rollRight( script::VM* vm, const char* funcName );
	int		script_setFireTarget( script::VM* vm, const char* funcName );
	int		script_setAimInaccuracy( script::VM* vm, const char* funcName );
	int		script_setAimChangeSlewRate( script::VM* vm, const char* funcName );
	int		script_setShootDelay( script::VM* vm, const char* funcName );
	int		script_setMovementSluggishness( script::VM* vm, const char* funcName );
	int		script_setMovingRunning( script::VM* vm, const char* funcName );
	int		script_setMovingSneaking( script::VM* vm, const char* funcName );
	int		script_setMovingWalking( script::VM* vm, const char* funcName );
	int		script_setTurningSluggishness( script::VM* vm, const char* funcName );
	int		script_standHere( script::VM* vm, const char* funcName );
	int		script_stop( script::VM* vm, const char* funcName );
	int		script_turnTo( script::VM* vm, const char* funcName );
	int		script_turnToMovement( script::VM* vm, const char* funcName );
	
	// Unallowed 
	ComputerControl( const ComputerControl& other );
	ComputerControl& operator= ( const ComputerControl& other );
};


#endif