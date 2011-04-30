#include "ComputerControl.h"
#include "GameCharacter.h"
#include "BellCurve.h"
#include "GameWeapon.h"
#include "GameObject.h"
#include "GameLevel.h"
#include "GameCell.h"
#include "ScriptUtil.h"
#include <sg/Node.h>
#include <script/VM.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <math/lerp.h>
#include <sgu/NodeUtil.h>
#include <fsm/StateMachine.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace sgu;
using namespace fsm;
using namespace lang;
using namespace math;
using namespace script;

//-----------------------------------------------------------------------------

static const float	MIN_MOVEMENT_DISTANCE	= 0.25f;
static const float	JAMMED_DISTANCE			= 0.10f;
static const int	JAMMED_UPDATE_COUNT		= 15;

//-----------------------------------------------------------------------------

ScriptMethod<ComputerControl> ComputerControl::sm_methods[] =
{
	ScriptMethod<ComputerControl>( "crouch", script_crouch ),
	ScriptMethod<ComputerControl>( "crouching", script_crouching ),
	ScriptMethod<ComputerControl>( "fire", script_fire ),
	ScriptMethod<ComputerControl>( "fireOnce", script_fireOnce ),
	ScriptMethod<ComputerControl>( "getFireTarget", script_getFireTarget ),
	ScriptMethod<ComputerControl>( "getFireState", script_getFireState ),
	ScriptMethod<ComputerControl>( "getStateMachine", script_getStateMachine ),
	ScriptMethod<ComputerControl>( "goTo", script_goTo ),
	ScriptMethod<ComputerControl>( "goToOffset", script_goToOffset ),
	ScriptMethod<ComputerControl>( "lookToAngle", script_lookToAngle ),
	ScriptMethod<ComputerControl>( "physicalAttackStrike", script_physicalAttackStrike ),
	ScriptMethod<ComputerControl>( "physicalAttackKick", script_physicalAttackKick ),
	ScriptMethod<ComputerControl>( "rollBackward", script_rollBackward ),
	ScriptMethod<ComputerControl>( "rollForward", script_rollForward ),
	ScriptMethod<ComputerControl>( "rollLeft", script_rollLeft ),
	ScriptMethod<ComputerControl>( "rollRight", script_rollRight ),
	ScriptMethod<ComputerControl>( "setFireTarget", script_setFireTarget ),
	ScriptMethod<ComputerControl>( "setAimInaccuracy", script_setAimInaccuracy ),
	ScriptMethod<ComputerControl>( "setAimChangeSlewRate", script_setAimChangeSlewRate ),
	ScriptMethod<ComputerControl>( "setShootDelay", script_setShootDelay ),
	ScriptMethod<ComputerControl>( "setMovementSluggishness", script_setMovementSluggishness ),
	ScriptMethod<ComputerControl>( "setMovingRunning", script_setMovingRunning ),
	ScriptMethod<ComputerControl>( "setMovingSneaking", script_setMovingSneaking ),
	ScriptMethod<ComputerControl>( "setMovingWalking", script_setMovingWalking ),
	ScriptMethod<ComputerControl>( "setTurningSluggishness", script_setTurningSluggishness ),
	ScriptMethod<ComputerControl>( "standHere", script_standHere ),
	ScriptMethod<ComputerControl>( "stop", script_stop ),
	ScriptMethod<ComputerControl>( "turnTo", script_turnTo ),
	ScriptMethod<ComputerControl>( "turnToMovement", script_turnToMovement ),
};

//-----------------------------------------------------------------------------

ComputerControl::ComputerControl( script::VM* vm, io::InputStreamArchive* arch, GameCharacter* owner ) :
	ControlBase( vm, arch, owner ),
	m_goTo( Vector3(0,0,0) ),
	m_turnTo( Vector3(0,0,1) ),
	m_aimChangeMaxDelta( 100 ),
	m_movementSluggishness( 0 ),
	m_turnSluggishness( 0 ),
	m_aimAccuracy( 1 ),
	m_fireTarget( 0 ),
	m_moveMode( MOVE_WALKING ),
	m_crouching( false ),
	m_fire( false ),
	m_rollBackward( false ),
	m_rollForward( false ),
	m_rollLeft( false ),
	m_rollRight( false ),
	m_physicalAttackStrike( false ),
	m_physicalAttackKick( false ),
	m_readyToPhysicalAttackStrike( true ),
	m_readyToPhysicalAttackKick( true ),
	m_lookTo( 0, 0, Math::PI / 3.f ),
	m_fireOnce( false ),
	m_owner( owner ),
	m_aimingTimer( 0 ),
	m_aimingDelay( 0 ),
	m_stateMachine( new StateMachine( vm ) )
{
	m_methodBase = -1;
	if ( vm )
		m_methodBase = ScriptUtil<ComputerControl,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	resetInputState();
}

ComputerControl::~ComputerControl()
{
	m_stateMachine = 0;
}

void ComputerControl::resetInputState() 
{	
	m_goTo = m_owner->position();
	m_turnTo = m_owner->position() + m_owner->forward() * 5;
	m_lookTo.xAngle = 0;
	m_lookTo.yAngle = 0;
	m_lookTo.angleSpeed = Math::PI / 3.f;
	m_aimChangeMaxDelta = 30;
	m_movementSluggishness = 0.5f;
	m_turnSluggishness = 0.5f;
	m_aimAccuracy = 5;
	m_fireTarget = 0;
	m_crouching = false;
	m_fire = false;
	m_rollBackward = false;
	m_rollForward = false;
	m_rollLeft = false;
	m_rollRight = false;
	m_physicalAttackStrike = false;
	m_physicalAttackKick = false;
	m_readyToPhysicalAttackKick = true;
	m_readyToPhysicalAttackStrike = true;
	m_jammedCount = 0;
	m_lastMovementVector = Vector3(0,0,0);
	m_fireOnce = false;
}

void ComputerControl::physicalAttackStrike( bool active )
{
	m_physicalAttackStrike = active;
}

void ComputerControl::physicalAttackKick( bool active )
{
	m_physicalAttackKick = active;
}

void ComputerControl::updateJammedMovementDetection()
{
	float moveVecLen = m_lastMovementVector.length();
	if ( moveVecLen > Float::MIN_VALUE )
	{
		++m_jammedCount;
		
		float dist = ( m_owner->position() - m_jammedPos ).length();
		if ( moveVecLen < 1.f )
			dist /= moveVecLen;

		if ( dist >= JAMMED_DISTANCE )
		{
			m_jammedPos = m_owner->position();
			m_jammedCount = 0;
		}
		else if ( m_jammedCount > JAMMED_UPDATE_COUNT )
		{
			goTo( m_owner->position() );
		}
	}
}
void ComputerControl::updateAimDelay( float dt ) 
{
	m_aimingTimer -= dt;
}


GameCharacter::PrimaryState ComputerControl::evaluatePrimaryState( GameCharacter::PrimaryState lastState ) 
{
	GameCharacter::PrimaryState state = GameCharacter::PRIMARY_STATE_STANDING;

	if (( m_owner->position() - m_goTo ).length() > MIN_MOVEMENT_DISTANCE )	
	{
		state = GameCharacter::PRIMARY_STATE_WALKING;
		updateJammedMovementDetection();
	}

	if ( m_crouching )
	{
		if ( lastState != GameCharacter::PRIMARY_STATE_CROUCHED && 
			lastState != GameCharacter::PRIMARY_STATE_CROUCHING_DOWN )
			state = GameCharacter::PRIMARY_STATE_CROUCHING_DOWN;
		else if ( lastState == GameCharacter::PRIMARY_STATE_CROUCHING_DOWN || 
			lastState == GameCharacter::PRIMARY_STATE_CROUCHED )
			state = GameCharacter::PRIMARY_STATE_CROUCHED;
	}
	else
		if ( lastState == GameCharacter::PRIMARY_STATE_CROUCHED )
			state = GameCharacter::PRIMARY_STATE_UNCROUCHING;

	if ( m_owner->secondaryState() != GameCharacter::SECONDARY_STATE_CHANGING_CLIP &&
		 m_owner->secondaryState() != GameCharacter::SECONDARY_STATE_PHYSICAL_STRIKE )
	{
		if ( m_rollBackward )
		{
			state = GameCharacter::PRIMARY_STATE_ROLLING_BACKWARD;
			m_rollBackward = false;
		}
		if ( m_rollForward )
		{
			state = GameCharacter::PRIMARY_STATE_ROLLING_FORWARD;
			m_rollForward = false;
		}

		if ( m_rollLeft )
		{
			state = GameCharacter::PRIMARY_STATE_ROLLING_LEFT;
			m_rollLeft = false;
		}

		if ( m_rollRight )
		{
			state = GameCharacter::PRIMARY_STATE_ROLLING_RIGHT;
			m_rollRight = false;
		}
		if ( m_readyToPhysicalAttackKick && m_physicalAttackKick )
			state = GameCharacter::PRIMARY_STATE_PHYSICAL_KICK;

		m_readyToPhysicalAttackKick = (state != GameCharacter::PRIMARY_STATE_PHYSICAL_KICK && !m_physicalAttackKick && 
			state != GameCharacter::PRIMARY_STATE_PEEKING_LEFT &&
			state != GameCharacter::PRIMARY_STATE_PEEKING_RIGHT &&
			(m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_IDLING || 
			m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_LOOKING || 
			m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_AIMING || 
			m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_HOLDING_AIM) );

		m_physicalAttackKick = false;
	}

	return state;
}

GameCharacter::SecondaryState ComputerControl::evaluateSecondaryState( GameCharacter::SecondaryState lastState ) 
{
	GameCharacter::SecondaryState state = lastState;

	if ( !GameCharacter::isRolling( m_owner->primaryState() ) && !GameCharacter::isHurting( m_owner->primaryState() ) &&
		m_owner->primaryState() != GameCharacter::PRIMARY_STATE_PHYSICAL_KICK )
	{
		state = GameCharacter::SECONDARY_STATE_LOOKING;

		if ( ( lastState == GameCharacter::SECONDARY_STATE_AIMING || lastState == GameCharacter::SECONDARY_STATE_HOLDING_AIM 
			  || lastState == GameCharacter::SECONDARY_STATE_ATTACKING ) && m_owner->secondaryStateTime() < m_owner->aimingTimeAfterShooting() )
			state = GameCharacter::SECONDARY_STATE_HOLDING_AIM;

		if ( lastState == GameCharacter::SECONDARY_STATE_ATTACKING && !m_owner->weapon()->ready() )
			state = GameCharacter::SECONDARY_STATE_ATTACKING;

		if ( m_fireTarget && ( m_fire || m_fireOnce ) )
		{
			if ( lastState == GameCharacter::SECONDARY_STATE_ATTACKING && m_owner->weapon()->ready() )
			{
				state = GameCharacter::SECONDARY_STATE_HOLDING_AIM;
			}
			else
			{
				if ( m_owner->readyToShoot() && m_aimingTimer <= 0 )
				{
					state = GameCharacter::SECONDARY_STATE_ATTACKING;
					m_fireOnce = false;
					m_aimingTimer = m_aimingDelay;
				}
				else
				{
					state = GameCharacter::SECONDARY_STATE_AIMING;
				}
			}
			if ( m_owner->weapon()->clipEmpty() )
			{
				state = GameCharacter::SECONDARY_STATE_CHANGING_CLIP;
			}
		}
		if ( m_readyToPhysicalAttackStrike && m_physicalAttackStrike && state != GameCharacter::SECONDARY_STATE_ATTACKING )
			state = GameCharacter::SECONDARY_STATE_PHYSICAL_STRIKE;

		m_readyToPhysicalAttackStrike = (state != GameCharacter::SECONDARY_STATE_PHYSICAL_STRIKE 
			&& !m_physicalAttackStrike && !GameCharacter::isPhysicalAttacking( m_owner->primaryState() ) );

		m_physicalAttackStrike = false; 
	}

	return state;
}

void ComputerControl::adjustVelocity( math::Vector3* velocity, math::Vector3* movement, math::Vector3* movementControl ) 
{
	Vector3 delta = m_owner->position() - m_goTo;

	// don't move less than 25cm
	if ( delta.length() > MIN_MOVEMENT_DISTANCE )	
	{
		float angle = m_owner->getAngleTo( m_goTo );

		Vector3 mov = Vector3(0,0,1);
		switch ( m_moveMode )
		{
		case MOVE_SNEAKING:
			mov = Vector3(0,0, m_owner->maxSneakingControl() - 0.1f );
			break;
		case MOVE_WALKING:
			mov = Vector3(0,0, m_owner->maxWalkingControl() - 0.1f );
			break;
		case MOVE_RUNNING:
			mov = Vector3(0,0, m_owner->maxRunningControl() - 0.1f );
			break;
		default:
			throw Exception( Format("Invalid move mode for character {0}", m_owner->name() ) );
		}
		Matrix3x3( Vector3(0,1,0), -angle ).rotate( mov, &mov );

		getVelocityFromMovementControlVector( mov, movement, velocity, movementControl );
	}
	else
	{
		*velocity = Vector3(0,0,0);
		*movement = Vector3(0,0,0);	
		*movementControl = Vector3(0,0,0);	
	}

	m_lastMovementVector = *movement;
}

void ComputerControl::adjustRotation( math::Matrix3x3* rot, float maxRotationSpeed, float dt ) 
{
	Matrix3x3 rotation = *rot;
	
	Vector3 turnToTarget = m_turnTo - m_owner->up() * m_turnTo.dot( m_owner->up() );
	float angle = m_owner->getAngleTo( turnToTarget );

	if ( Math::abs(angle) > maxRotationSpeed * dt )
	{
		if ( angle > 0.f )
			angle = maxRotationSpeed * dt;
		if ( angle < 0.f )
			angle = -maxRotationSpeed * dt;
	}

	if ( Math::abs(angle) > Math::toRadians(1.f) )
	{
		rotation = rotation * Matrix3x3( Vector3(0,1,0), -angle );
	}
	
	// Prevent rotation if hurting
	if ( !GameCharacter::isHurting( m_owner->primaryState() ) )
	{
		*rot = rotation;
	}
}

Vector3 ComputerControl::perturbVector( const Vector3& src, float limit, const Vector3& up, const Vector3& right )
{
	Vector3 dir( src );		
	
	Vector3 leftLimit( dir.rotate( up, random() * -limit ) );
	Vector3 rightLimit( dir.rotate( up, random() * limit ) );
	Vector3 upLimit( dir.rotate( right, random() * -limit ) );
	Vector3 downLimit( dir.rotate( right, random() * limit ) );

	// Create 4 random numbers, normalize sum to one, take a weighted average of limit vectors with the values and assign it as direction
	float r1 = random();
	float r2 = random();
	float r3 = random();
	float r4 = random();
	float normalizer = 1.f / ( r1 + r2 + r3 + r4 );
	r1 *= normalizer;
	r2 *= normalizer;
	r3 *= normalizer;
	r4 *= normalizer;

	return leftLimit * r1 + rightLimit * r2 + upLimit * r3 + downLimit * r4;	
}

void ComputerControl::adjustAim( math::Vector3* aimVector, float /*dt*/ ) 
{
	if ( m_owner->isAiming( m_owner->secondaryState() ) || m_owner->isAttacking( m_owner->secondaryState() ) )
	{
	//	Debug::println( Format( "Aim before perturbation {0},{1},{2}", aimVector->x, aimVector->y, aimVector->z ).format() );

		Vector3 aim = perturbVector( *aimVector, m_aimAccuracy, m_owner->up(), m_owner->right() );
		
	//	Debug::println( Format( "Aim after perturbation by {3} degrees, {0},{1},{2}", aim.x, aim.y, aim.z, Math::toDegrees( m_aimAccuracy ) ).format() );
		*aimVector = aim;
	}

}

void ComputerControl::adjustLook( math::Vector3* lookVector, float dt )
{
	if ( lookVector->length() < 1.f )
	{
		*lookVector = lookVector->normalize();
	}

	if ( !isLookingAt( m_lookTo.xAngle, m_lookTo.yAngle, m_lookTo.angleSpeed ) )
	{
		Vector3 target( 0,0,1 );
		target = target.rotate( Vector3( 0, 1, 0 ), m_lookTo.xAngle );
		target = target.rotate( Vector3( 1, 0, 0 ), m_lookTo.yAngle );
		Vector3 look = *lookVector;

		// find rotation normal that gets look from current vec to lookTo
		Vector3 rotateNormal = look.cross( target ).normalize();
		look = look.rotate( rotateNormal, m_lookTo.angleSpeed * dt );
		*lookVector = look;
	}
}

void ComputerControl::goTo( const Vector3& pos )
{
	m_goTo = pos;
	m_jammedCount = 0;
	m_jammedPos = m_owner->position();
}

float ComputerControl::getAbsoluteAngleXZ( const Vector3& vec )
{	
	Vector3 planevec = Vector3( vec.x, 0, vec.z );

	if ( planevec.length() > Float::MIN_VALUE )
	{
		planevec = planevec.normalize();

		float turn = planevec.dot( Vector3(0.f, 0.f, 1.f) );
		bool onright = ( planevec.dot( Vector3(1.f, 0.f, 0.f) ) > 0.f );
		
		float angle = Math::acos( turn );
		if ( !onright ) 
			angle = -angle;
			
		return( angle );
	}
	else
		return 0.f;
}

float ComputerControl::getAbsoluteAngleYZ( const Vector3& vec )
{
	Vector3 planevec = Vector3( 0, vec.y, vec.z );

	if ( planevec.length() > Float::MIN_VALUE )
	{
		planevec = planevec.normalize();

		float turn = planevec.dot( Vector3(0.f, 0.f, 1.f) );
		bool ontop = ( planevec.dot( Vector3(0.f, 1.f, 0.f) ) > 0.f );
		
		float angle = Math::acos( turn );
		if ( ontop ) 
			angle = -angle;
			
		return( angle );
	}
	else
		return 0.f;
}

bool ComputerControl::isLookingAt( float xangle, float yangle, float anglespeed ) const
{
	Vector3 look = m_owner->lookVector();

	// use anglespeed / 100 as accuracy limit
	float maxVariation = anglespeed / 100.f;

	float lookAngleX = getAbsoluteAngleXZ( look );
	float lookAngleY = getAbsoluteAngleYZ( look );

	// if angle X and angle Y fall within target +- maxVariation, then looking angle will be viewed as being correct
	if ( ( xangle - maxVariation < lookAngleX && lookAngleX < xangle + maxVariation ) &&
		 ( yangle - maxVariation < lookAngleY && lookAngleY < yangle + maxVariation ) )
			return true;
		
	return false;
}

int ComputerControl::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<ComputerControl,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int ComputerControl::script_goTo( script::VM* vm, const char* funcName ) 
{
	int tags1[] = {VM::TYPE_TABLE};
	int tags2[] = {VM::TYPE_STRING};
	int tags3[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER,};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		 !hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) &&
		 !hasParams(tags3,sizeof(tags3)/sizeof(tags3[0])))
		throw ScriptException( Format("{0} expects GameObject table, level node name or Vector3 (x,y,z)", funcName) );

	if ( vm->isTable(1) )
	{
		GameObject* obj = dynamic_cast<GameObject*>( getThisPtr(vm,1) );
		if (!obj)
			throw ScriptException( Format("{0} expects (C++) GameObject, not regular table", funcName) );
		
		goTo( obj->position() );
	}
	else if ( vm->isString(1) )
	{
		Node* node = 0;
		
		int cell = 0;
		while ( !node && cell < m_owner->level()->cells() )
		{
			node = NodeUtil::findNodeByName( m_owner->level()->getCell( cell )->getRenderObject(0), vm->toString(1) );
			cell++;
		}
		
		if ( !node )
			throw ScriptException( Format("{0} expects a node in the current level, {1} not found", funcName, vm->toString(1) ) );
		
		goTo( node->position() );
	}
	else
	{
		Vector3 pos( vm->toNumber(1), vm->toNumber(2), vm->toNumber(3) );
		goTo( pos );
	}

	return 0;
}

int ComputerControl::script_goToOffset( script::VM* vm, const char* funcName ) 
{
	int tags1[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER,};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])))
		throw ScriptException( Format("{0} expects Vector3 (x,y,z)", funcName) );

	Vector3 pos( m_owner->position().x + vm->toNumber(1), m_owner->position().y + vm->toNumber(2), m_owner->position().z + vm->toNumber(3) );
	goTo( pos );
	return 0;
}

int ComputerControl::script_turnTo( script::VM* vm, const char* funcName ) 
{
	int tags1[] = {VM::TYPE_TABLE};
	int tags3[] = {VM::TYPE_STRING};
	int tags2[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER,};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		 !hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) &&
		 !hasParams(tags3,sizeof(tags3)/sizeof(tags3[0])))
		throw ScriptException( Format("{0} expects GameObject table, level node name or Vector3 (x,y,z)", funcName) );

	if ( vm->isTable(1) )
	{
		GameObject* obj = dynamic_cast<GameObject*>( getThisPtr(vm,1) );
		if (!obj)
			throw ScriptException( Format("{0} expects (C++) GameObject, not regular table", funcName) );
	
		m_turnTo = obj->position();
	}
	else if ( vm->isString(1) )
	{
		Node* node = 0;
		
		int cell = 0;
		while ( !node && cell < m_owner->level()->cells() )
		{
			node = NodeUtil::findNodeByName( m_owner->level()->getCell( cell )->getRenderObject(0), vm->toString(1) );
			cell++;
		}
		
		if ( !node )
			throw ScriptException( Format("{0} expects a node in the current level, {1} not found", funcName, vm->toString(1) ) );

		m_turnTo = node->position();
	}
	else
	{
		m_turnTo.x = vm->toNumber(1);
		m_turnTo.y = vm->toNumber(2);
		m_turnTo.z = vm->toNumber(3);
	}

	return 0;
}

int ComputerControl::script_crouch( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 ) 
		throw ScriptException( Format("{0} tells character to crouch, expects 1 boolean parameter", funcName ) );

	m_crouching = !vm->isNil(1);
	return 0;
}

int ComputerControl::script_crouching( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 ) 
		throw ScriptException( Format("{0} returns true if character should be crouching", funcName ) );

	vm->pushBoolean( m_crouching );
	return 1;
}


int ComputerControl::script_fire( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 ) 
		throw ScriptException( Format("{0} tells character to fire its' weapon, expects 1 boolean parameter", funcName ) );

	m_fire = !vm->isNil(1);
	return 0;
}


int ComputerControl::script_getFireTarget( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 ) 
		throw ScriptException( Format("{0} returns current fire target or nil if none", funcName ) );

	if ( m_fireTarget )
		vm->pushTable( m_fireTarget );
	else
		vm->pushNil();

	return 1;
}

int ComputerControl::script_setFireTarget( script::VM* vm, const char* funcName ) 
{
	int tags1[] = {VM::TYPE_TABLE};
	int tags2[] = {VM::TYPE_NIL};
	if ( !hasParams(tags1,sizeof(tags1)/sizeof(tags1[0])) &&
		 !hasParams(tags2,sizeof(tags2)/sizeof(tags2[0])) )
		throw ScriptException( Format("{0} expects GameCharacter table", funcName) );

	if ( !vm->isNil(1) )
		m_fireTarget = (GameCharacter*)( getThisPtr(vm,1) );
	else
		m_fireTarget = 0;

	return 0;
}

int ComputerControl::script_setAimInaccuracy( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number in range 0..1 ( 0 = stupid, 1 = perfect )", funcName) );
	
	m_aimAccuracy = Math::toRadians( vm->toNumber(1) );
	return 0;
}

int ComputerControl::script_setAimChangeSlewRate( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects max aim change (meters) per second", funcName) );
	
	m_aimChangeMaxDelta = vm->toNumber(1);
	return 0;
}

int ComputerControl::script_setMovementSluggishness( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number in range 0..1 ( 0 = stupid, 1 = perfect )", funcName) );
	
	m_movementSluggishness = vm->toNumber(1);
	return 0;
}

int ComputerControl::script_setTurningSluggishness( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects number in range 0..1 ( 0 = stupid, 1 = perfect )", funcName) );
	
	m_turnSluggishness = vm->toNumber(1);
	return 0;
}

int ComputerControl::script_getStateMachine( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() > 0 )
		throw ScriptException( Format("{0} returns state machine table, expects no parameters", funcName) );

	vm->pushTable( m_stateMachine );
	return 1;
}

int ComputerControl::script_standHere( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() > 0 )
		throw ScriptException( Format("{0} tells character stand where it is, expects no parameters", funcName) );

	goTo( m_owner->position() );
	m_turnTo = m_owner->position() + m_owner->forward() * 5;

	return 0;
}

int ComputerControl::script_stop( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() > 0 )
		throw ScriptException( Format("{0} tells character to stop, expects no parameters", funcName) );

	goTo( m_owner->position() );
	return 0;	
}


int ComputerControl::script_rollBackward( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to roll backward, expects no parameters", funcName) );

	m_rollBackward = !vm->isNil(1);
	return 0;
}

int ComputerControl::script_rollForward( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to roll forward, expects no parameters", funcName) );

	m_rollForward = !vm->isNil(1);
	return 0;
}

int ComputerControl::script_rollLeft( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to roll left, expects no parameters", funcName) );

	m_rollLeft = !vm->isNil(1);
	return 0;
}

int ComputerControl::script_rollRight( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to roll right, expects no parameters", funcName) );

	m_rollRight = !vm->isNil(1);
	return 0;
}

int ComputerControl::script_setMovingRunning( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to move running, expects no parameters", funcName) );

	m_moveMode = MOVE_RUNNING;
	return 0;
}

int ComputerControl::script_setMovingSneaking( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to move sneaking, expects no parameters", funcName) );

	m_moveMode = MOVE_SNEAKING;
	return 0;
}

int ComputerControl::script_setMovingWalking( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to move walking, expects no parameters", funcName) );

	m_moveMode = MOVE_WALKING;
	return 0;
}

int ComputerControl::script_lookToAngle( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} tells character to look to an angle, expects X Angle, Y Angle and Degrees/Sec parameters, returns true if character is already looking at this angle", funcName) );
	
	if ( vm->toNumber(3) <= 0.f )
		throw ScriptException( Format("{0} does not accept angle speed less or equal to 0", funcName ) );

	m_lookTo.xAngle	= Math::toRadians( vm->toNumber(1) );
	m_lookTo.yAngle	= Math::toRadians( vm->toNumber(2) );
	m_lookTo.angleSpeed	= Math::toRadians( vm->toNumber(3) );	

	vm->pushBoolean( isLookingAt( m_lookTo.xAngle, m_lookTo.yAngle, m_lookTo.angleSpeed ) );
	return 1;
}

int ComputerControl::script_physicalAttackStrike( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to attack physically", funcName) );

	this->physicalAttackStrike( true );
	return 0;
}

int ComputerControl::script_physicalAttackKick( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to attack physically", funcName) );

	this->physicalAttackKick( true );
	return 0;
}

int ComputerControl::script_turnToMovement( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to face the direction it is moving to", funcName) );

	m_turnTo = m_goTo;
	return 0;
}

int ComputerControl::script_getFireState( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} true if player is firing", funcName) );

	vm->pushBoolean( m_owner->secondaryState() == GameCharacter::SECONDARY_STATE_ATTACKING );
	return 1;
}

int ComputerControl::script_fireOnce( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} tells character to fire once", funcName) );

	m_fireOnce = true;
	return 0;
}

int ComputerControl::script_setShootDelay( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects Aim delay time (time between shoots) in seconds", funcName) );

	m_aimingDelay = vm->toNumber(1);
	return 0;
}

