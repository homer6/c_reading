#include "UserControl.h"
#include "GameCharacter.h"
#include "GameWeapon.h"
#include "ScriptUtil.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <script/ScriptException.h>
#include <math/lerp.h>
#include <math.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<UserControl> UserControl::sm_methods[] =
{
	ScriptMethod<UserControl>( "isTurning", script_isTurning ),
};

//-----------------------------------------------------------------------------

/** Returns difference of two angles in radians. */
static float getAngleDifference( float a, float b )
{
	const float PI2 = Math::PI*2.f;
	while ( a < 0.f )
		a += PI2;
	while ( b < 0.f )
		b += PI2;
	a = fmodf( a, PI2 );
	b = fmodf( b, PI2 );

	if ( a > b )
	{
		float tmp = a;
		a = b;
		b = tmp;
	}

	// a <= b
	float d1 = b - a;
	float d2 = a + (PI2-b);
	if ( d1 < d2 )
		return d1;
	else
		return d2;
}

//-----------------------------------------------------------------------------

UserControl::UserControl(script::VM* vm, io::InputStreamArchive* arch, GameCharacter* owner ) :
	ControlBase( vm, arch, owner ),
	m_owner( owner ),
	m_movementControlVector( 0, 0, 0 )
{
	m_methodBase = ScriptUtil<UserControl,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
	resetInputState();
}

int UserControl::methodCall( VM* vm, int i )
{
	return ScriptUtil<UserControl,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

void UserControl::accelerate( float value )
{
	m_accelerating = value;
}

void UserControl::accelerateBackwards( float value ) 
{
	m_acceleratingBackwards = value;
}

void UserControl::turnRight( float value )
{
	m_turningRight = value;
}

void UserControl::turnLeft( float value )
{
	m_turningLeft = value;
}

void UserControl::jump( bool active )
{
	m_jumping = active;
}

void UserControl::attack( bool active )
{
	m_attacking = active;
}

void UserControl::strafeLeft( float value ) 
{
	m_strafingLeft = value;
}

void UserControl::strafeRight( float value ) 
{
	m_strafingRight = value;
}

void UserControl::roll( float value )
{
	m_rolling = value;
}

void UserControl::crouch( bool active )
{
	m_crouching = active;
}

void UserControl::peekLeft( float value ) 
{
	m_peekingLeft = value;
}

void UserControl::peekRight( float value ) 
{
	m_peekingRight = value;
}

void UserControl::physicalAttackStrike( bool active )
{
	m_physicalAttackingStrike = active;
}

void UserControl::physicalAttackKick( bool active )
{
	m_physicalAttackingKick = active;
}

void UserControl::changeClip( bool active )
{
	m_changeClip = active;
}

void UserControl::cycleWeapon( bool active )
{
	m_cycleWeapon = active;
}

void UserControl::throwEmptyShell( bool active )
{
	m_throwEmptyShell = active;
}

void UserControl::runModifier( bool active )
{
	m_runModifier = active;
}

void UserControl::sneakModifier( bool active )
{
	m_sneakModifier = active;
}

void UserControl::resetInputState()
{
	m_accelerating = 0.f;
	m_acceleratingBackwards = 0.f;
	m_turningRight = 0.f;
	m_turningLeft = 0.f;
	m_jumping = false;
	m_attacking = false;
	m_strafingLeft = 0.f;
	m_strafingRight = 0.f;
	m_rolling = 0.f;
	m_crouching = false;
	m_physicalAttackingStrike  = false;
	m_physicalAttackingKick = false;
	m_peekingLeft = 0.f;
	m_peekingRight = 0.f;
	m_readyToChangeClip = true;
	m_readyToSingleShot = true;
	m_readyToPhysicalAttackStrike = true;
	m_readyToPhysicalAttackKick = true;
	m_readyToRoll = true;
	m_changeClip = false;
	m_cycleWeapon = false;
	m_throwEmptyShell = false;
	m_runModifier = false;
	m_sneakModifier = false;
	m_movementControlVector = Vector3(0,0,0);
}

GameCharacter::PrimaryState UserControl::evaluatePrimaryState( GameCharacter::PrimaryState lastState )
{
	GameCharacter::PrimaryState state = GameCharacter::PRIMARY_STATE_STANDING;

	if ( m_accelerating > ZERO_CONTROL_VALUE )
		state = GameCharacter::PRIMARY_STATE_WALKING;

	if ( m_acceleratingBackwards > ZERO_CONTROL_VALUE )
		state = GameCharacter::PRIMARY_STATE_WALKING_BACKWARD;

	if ( m_strafingLeft > ZERO_CONTROL_VALUE || m_strafingRight > ZERO_CONTROL_VALUE )
		state = GameCharacter::PRIMARY_STATE_STRAFING;

	if ( m_jumping && 
		lastState != GameCharacter::PRIMARY_STATE_JUMPING && 
		lastState != GameCharacter::PRIMARY_STATE_INAIR )
		state = GameCharacter::PRIMARY_STATE_JUMPING;

	if ( m_peekingLeft > ZERO_CONTROL_VALUE && m_owner->canMove(-m_owner->right() * m_owner->peekMoveCheckDistance() ) && 
		lastState != GameCharacter::PRIMARY_STATE_PEEKING_RIGHT)
	{
		state = GameCharacter::PRIMARY_STATE_PEEKING_LEFT;
	}
	else if ( m_peekingRight > ZERO_CONTROL_VALUE && m_owner->canMove(m_owner->right() * m_owner->peekMoveCheckDistance() ) && 
		lastState != GameCharacter::PRIMARY_STATE_PEEKING_LEFT)
	{
		state = GameCharacter::PRIMARY_STATE_PEEKING_RIGHT;
	}
	else if ( lastState == GameCharacter::PRIMARY_STATE_PEEKING_LEFT )
	{
		state = GameCharacter::PRIMARY_STATE_UNPEEKING_LEFT;
	}
	else if ( lastState == GameCharacter::PRIMARY_STATE_PEEKING_RIGHT )
	{
		state = GameCharacter::PRIMARY_STATE_UNPEEKING_RIGHT;
	}
	
	if ( m_crouching )
	{
		if ( lastState != GameCharacter::PRIMARY_STATE_CROUCHED && 
			lastState != GameCharacter::PRIMARY_STATE_CROUCHING_DOWN &&
			lastState != GameCharacter::PRIMARY_STATE_CROUCHED_WALKING )
		{
			state = GameCharacter::PRIMARY_STATE_CROUCHING_DOWN;
		}
		else if ( lastState == GameCharacter::PRIMARY_STATE_CROUCHING_DOWN || 
			lastState == GameCharacter::PRIMARY_STATE_CROUCHED ||
			lastState == GameCharacter::PRIMARY_STATE_CROUCHED_WALKING )
		{
			state = GameCharacter::PRIMARY_STATE_CROUCHED;
			if ( movementControlVector().length() > ZERO_CONTROL_VALUE )
				state = GameCharacter::PRIMARY_STATE_CROUCHED_WALKING;
		}
	}
	else
	{
		if ( lastState == GameCharacter::PRIMARY_STATE_CROUCHED )
			state = GameCharacter::PRIMARY_STATE_UNCROUCHING;
	}

	if ( m_rolling && m_readyToRoll && !GameCharacter::isRolling( lastState ) && 
	/*	m_owner->secondaryState() != GameCharacter::SECONDARY_STATE_CHANGING_CLIP && */
		m_owner->secondaryState() != GameCharacter::SECONDARY_STATE_PHYSICAL_STRIKE )
	{
		float roll = m_accelerating;
		state = GameCharacter::PRIMARY_STATE_ROLLING_FORWARD;
		if ( m_acceleratingBackwards > roll )
		{
			roll = m_acceleratingBackwards;
			state = GameCharacter::PRIMARY_STATE_ROLLING_BACKWARD;
		}
		if ( m_strafingLeft > roll )
		{
			roll = m_strafingLeft;
			state = GameCharacter::PRIMARY_STATE_ROLLING_LEFT;
		}
		if ( m_strafingRight > roll )
		{
			roll = m_strafingRight;
			state = GameCharacter::PRIMARY_STATE_ROLLING_RIGHT;
		}
	}
	m_readyToRoll = ( m_rolling == 0.f && !GameCharacter::isRolling(m_owner->primaryState()) );

	if ( m_readyToPhysicalAttackKick && m_physicalAttackingKick )
		state = GameCharacter::PRIMARY_STATE_PHYSICAL_KICK;

	m_readyToPhysicalAttackKick = (state != GameCharacter::PRIMARY_STATE_PHYSICAL_KICK && !m_physicalAttackingKick && 
		state != GameCharacter::PRIMARY_STATE_PEEKING_LEFT &&
		state != GameCharacter::PRIMARY_STATE_PEEKING_RIGHT &&
		(m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_IDLING || 
		m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_LOOKING || 
		m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_AIMING || 
		m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_HOLDING_AIM) );

	return state;
}

GameCharacter::SecondaryState UserControl::evaluateSecondaryState( GameCharacter::SecondaryState lastState )
{
	GameCharacter::SecondaryState state = lastState;
	
	if ( !GameCharacter::isRolling( m_owner->primaryState() ) && !GameCharacter::isHurting( m_owner->primaryState() ) 
		&& m_owner->primaryState() != GameCharacter::PRIMARY_STATE_PHYSICAL_KICK )
	{
		state = GameCharacter::SECONDARY_STATE_LOOKING;

		if ( m_owner->hasWeapon() )
		{
			if ( ( lastState == GameCharacter::SECONDARY_STATE_AIMING || 
				lastState == GameCharacter::SECONDARY_STATE_HOLDING_AIM || 
				lastState == GameCharacter::SECONDARY_STATE_ATTACKING ) && 
				m_owner->secondaryStateTime() < m_owner->aimingTimeAfterShooting() )
			{
				if ( lastState == GameCharacter::SECONDARY_STATE_AIMING && !m_owner->readyToShoot() )
					state = GameCharacter::SECONDARY_STATE_AIMING;
				else
					state = GameCharacter::SECONDARY_STATE_HOLDING_AIM;
			}

			if ( m_throwEmptyShell )
				state = GameCharacter::SECONDARY_STATE_THROWING_EMPTY_SHELL;

			if ( m_readyToChangeClip && m_changeClip )
				state = GameCharacter::SECONDARY_STATE_CHANGING_CLIP;

			if ( lastState == GameCharacter::SECONDARY_STATE_ATTACKING && !m_owner->weapon()->ready() )
			{
				state = GameCharacter::SECONDARY_STATE_ATTACKING;
			}
			else if ( m_attacking )
			{
				if ( m_readyToChangeClip && m_owner->weapon()->clipEmpty() )
				{
					state = GameCharacter::SECONDARY_STATE_CHANGING_CLIP;
				}
				else
				{
					if ( lastState == GameCharacter::SECONDARY_STATE_ATTACKING && m_owner->weapon()->ready() )
					{
						state = GameCharacter::SECONDARY_STATE_HOLDING_AIM;
					}
					else
					{
						if ( m_owner->readyToShoot() && 
							!m_owner->weapon()->clipEmpty() &&
							(m_readyToSingleShot || m_owner->weapon()->fireMode() == GameWeapon::FIRE_AUTO) )
						{
							state = GameCharacter::SECONDARY_STATE_ATTACKING;
							m_readyToChangeClip = false;
							m_readyToSingleShot = false;
						}
						else
						{
							state = GameCharacter::SECONDARY_STATE_AIMING;
						}
					}
				}
			}
			else
			{
				m_readyToChangeClip = true;
				m_readyToSingleShot = m_owner->weapon()->ready();
			}

			if ( m_cycleWeapon )
			{
				state = GameCharacter::SECONDARY_STATE_CYCLING_WEAPON;
			}
		}

		if ( m_readyToPhysicalAttackStrike && m_physicalAttackingStrike && state != GameCharacter::SECONDARY_STATE_ATTACKING )
			state = GameCharacter::SECONDARY_STATE_PHYSICAL_STRIKE;

		m_readyToPhysicalAttackStrike = (state != GameCharacter::SECONDARY_STATE_PHYSICAL_STRIKE 
			&& !m_physicalAttackingStrike && !GameCharacter::isPhysicalAttacking( m_owner->primaryState() ) );
	}

	return state;
}

void UserControl::adjustVelocity( float dt, const Vector3& /*forward*/, const Vector3& /*right*/,
	Vector3* velocity, Vector3* movement, Vector3* movementControl )
{	
	Vector3 vec0 = m_movementControlVector;
	Vector3 vec1 = scaleMovementControlVector( inputMovementControlVector() );

	const float PI2			= Math::PI * 2.f;
	const float angleSpeed	= Math::toRadians( m_owner->getNumber("inputTurnSpeed") ); //  Math::toRadians(180.f);
	const float lengthSpeed = m_owner->getNumber("inputControlSpeed"); // 2.f;

	// source and target length
	float len0 = vec0.length();
	float len1 = vec1.length();

	// source and target angle
	float ang0 = Math::abs(vec0.x)+Math::abs(vec0.z) > Float::MIN_VALUE ? Math::atan2( vec0.z, vec0.x ) : Math::PI*.5f;
	if ( ang0 < 0.f )
		ang0 += PI2;
	float ang1 = Math::abs(vec1.x)+Math::abs(vec1.z) > Float::MIN_VALUE ? Math::atan2( vec1.z, vec1.x ) : Math::PI*.5f;
	if ( ang1 < 0.f )
		ang1 += PI2;

	// new control vector length and angle
	float len = len0;
	float ang = ang0;

	// update length
	if ( len0 <= ZERO_CONTROL_VALUE )
	{
		len = len1;
	}
	else
	{
		len = len0;
		if ( len0 < len1 )
		{
			len = len0 + lengthSpeed*dt;
			if ( len > len1 )
				len = len1;
		}
		else if ( len0 > len1 )
		{
			len = len0 - lengthSpeed*dt;
			if ( len < len1 )
				len = len1;
		}
	}

	// update angle
	if ( ang0 > Math::toRadians(180.1f) && ang1 < Math::toRadians(179.9f) || 
		ang1 > Math::toRadians(180.1f) && ang0 < Math::toRadians(179.9f) || 
		len0 <= ZERO_CONTROL_VALUE )
	{
		ang = ang1;
	}
	else
	{
		ang = ang0;
		float angdiff0 = getAngleDifference(ang0,ang1);
		if ( angdiff0 > Math::toRadians(1.f) )
		{
			//Debug::println( "ang0={0,#}, ang1={1,#}, da={2,#}", Math::toDegrees(ang0), Math::toDegrees(ang1), Math::toDegrees(angdiff0) );
			float da = angleSpeed*dt;
			float ad1 = getAngleDifference(ang+da,ang1);
			float ad2 = getAngleDifference(ang-da,ang1);
			if ( Math::abs(getAngleDifference(ang,ang1)-Math::PI) < da*.5f ) // limit must be less than delta angle
			{
				// prefer front turn
				if ( ang >= Math::PI*.5f && ang <= Math::PI*1.5f )
					ang -= da;
				else
					ang += da;
			}
			else
			{
				float ad0 = getAngleDifference(ang,ang1);
				if ( ad1 < ad2 )
					ang += da;
				else
					ang -= da;
				float ad1 = getAngleDifference(ang,ang1);
				if ( ad0 < ad1 )
					ang = ang1;
			}
		}
		if ( len0 <= ZERO_CONTROL_VALUE )
			ang = ang1;
	}

	// create new control vector from length and angle
	m_movementControlVector = Matrix3x3( Vector3(0,1,0), -ang ) * Vector3( len, 0, 0 );


	getVelocityFromScaledMovementControlVector( m_movementControlVector, movement, velocity );
	*movementControl = m_movementControlVector;
}

void UserControl::adjustRotation( math::Matrix3x3* rot, float angleDelta ) 
{
	if ( m_owner->primaryState() != GameCharacter::PRIMARY_STATE_PEEKING_LEFT &&
		m_owner->primaryState() != GameCharacter::PRIMARY_STATE_PEEKING_RIGHT &&
		m_owner->primaryState() != GameCharacter::PRIMARY_STATE_PROJECTILE_HURT )
	{
		float angle = 0.f;
		if ( m_turningRight > ZERO_CONTROL_VALUE && GameCharacter::isControllable( m_owner->primaryState() ) )
			angle += angleDelta * m_turningRight;
		if ( m_turningLeft > ZERO_CONTROL_VALUE && GameCharacter::isControllable( m_owner->primaryState() ) )
			angle -= angleDelta * m_turningLeft;
		*rot = *rot * Matrix3x3(Vector3(0,1,0),angle);
	}
}

Vector3 UserControl::inputMovementControlVector() const
{
	Vector3 vec( 0, 0, 0 );

	// moving
	if ( m_accelerating > ZERO_CONTROL_VALUE )
		vec.z += m_accelerating;
	else if ( m_acceleratingBackwards > ZERO_CONTROL_VALUE )
		vec.z -= m_acceleratingBackwards;
	
	// strafing
	if ( m_strafingRight > ZERO_CONTROL_VALUE )
		vec.x += m_strafingRight;
	else if ( m_strafingLeft > ZERO_CONTROL_VALUE )
		vec.x -= m_strafingLeft;

	float lengthLimit = m_owner->maxWalkingControl() - 0.01f;

	if ( m_runModifier )
		lengthLimit = 1.f;

	if ( m_sneakModifier )
		lengthLimit = m_owner->maxSneakingControl() - 0.01f;

	if ( GameCharacter::isCrouched( m_owner->primaryState() ) || GameCharacter::isRolling( m_owner->primaryState() ) )
		lengthLimit = 1.f;

	// limit movement control vector length
	if ( vec.length() > lengthLimit )
		vec = vec.normalize() * lengthLimit;

	return vec;
}

Vector3 UserControl::movementControlVector() const
{
	return m_movementControlVector;
}

float UserControl::accelerating() const			
{ 
	return m_accelerating;
}

float UserControl::acceleratingBackwards() const	
{ 
	return m_acceleratingBackwards; 
}

float UserControl::turningRight() const			
{ 
	return m_turningRight; 
}

float UserControl::turningLeft() const			
{ 
	return m_turningLeft; 
}

float UserControl::strafingRight() const			
{ 
	return m_strafingRight; 
}

float UserControl::strafingLeft() const			
{ 
	return m_strafingLeft; 
}

float UserControl::attacking() const				
{ 
	return m_attacking; 
}

bool UserControl::cycleWeapon() const				
{ 
	return m_cycleWeapon; 
}

int UserControl::script_isTurning( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true if character is turning", funcName) );

	vm->pushBoolean( m_turningLeft>ZERO_CONTROL_VALUE || m_turningRight>ZERO_CONTROL_VALUE );
	return 1;
}
