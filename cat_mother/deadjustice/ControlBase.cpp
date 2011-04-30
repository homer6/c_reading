#include "ControlBase.h"
#include "GameCharacter.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Exception.h>
#include <math/lerp.h>
#include <math/Vector3.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

const float ControlBase::ZERO_CONTROL_VALUE = 0.005f;

//-----------------------------------------------------------------------------

ControlBase::ControlBase( script::VM* vm, io::InputStreamArchive* arch, GameCharacter* owner ) :
	GameScriptable( vm, arch, 0, 0 ),
	m_owner( owner )
{
}

void ControlBase::getVelocityFromMovementControlVector( const Vector3& movementControl,
	Vector3* movement, Vector3* velocity, Vector3* adjustedMovementControl ) const
{
	*adjustedMovementControl = scaleMovementControlVector( movementControl );
	getVelocityFromScaledMovementControlVector( *adjustedMovementControl, movement, velocity );
}

void ControlBase::getVelocityFromScaledMovementControlVector( const Vector3& movementControlVector,
	Vector3* movement, Vector3* velocity ) const
{
	Vector3 movementVector(0,0,0);
	Vector3 vel(0,0,0);

	// check that movement control is continuous in range [0,1] 
	if ( m_owner->minSneakingControl() > 0.f )
		throw Exception( Format("minSneakingControl must be 0 for control range [0,1] to be continuous") );
	if ( m_owner->maxRunningControl() < 1.f )
		throw Exception( Format("maxRunningControl must be 1 for control range [0,1] to be continuous") );
	if ( m_owner->maxSneakingControl() != m_owner->minWalkingControl() )
		throw Exception( Format("maxSneakingControl must be minWalkingControl for control range [0,1] to be continuous") );
	if ( m_owner->maxWalkingControl() != m_owner->minRunningControl() )
		throw Exception( Format("maxWalkingControl must be minRunningControl for control range [0,1] to be continuous") );

	if ( GameCharacter::isWalking( m_owner->primaryState() ) || GameCharacter::isStrafing( m_owner->primaryState() ) )
	{
		// adjust velocity and movement vector length by scaled movement control vector length
		if ( movementControlVector.lengthSquared() > Float::MIN_VALUE )
		{
			m_owner->rotation().rotate(movementControlVector,&vel);
			vel = vel.normalize();
			movementVector = movementControlVector.normalize();

			float movementControlLen = movementControlVector.length();
			if ( movementControlLen >= m_owner->minSneakingControl() && movementControlLen < m_owner->maxSneakingControl() )
			{
				// player is sneaking
				float s = ( movementControlLen - m_owner->minSneakingControl() ) / ( m_owner->maxSneakingControl() - m_owner->minSneakingControl() );
				vel *= lerp( m_owner->minSneakingSpeed(), m_owner->maxSneakingSpeed(), s );
				movementVector *= lerp( m_owner->minSneakingSpeed(), m_owner->maxSneakingSpeed(), s );
			}
			else if ( movementControlLen >= m_owner->minWalkingControl() && movementControlLen < m_owner->maxWalkingControl() )
			{
				// player is walking
				float s = ( movementControlLen - m_owner->minWalkingControl() ) / ( m_owner->maxWalkingControl() - m_owner->minWalkingControl() );
				vel *= lerp( m_owner->minWalkingSpeed(), m_owner->maxWalkingSpeed(), s );
				movementVector *= lerp( m_owner->minWalkingSpeed(), m_owner->maxWalkingSpeed(), s );
			}
			else
			{
				// player is running
				float s = ( movementControlLen - m_owner->minRunningControl() ) / ( m_owner->maxRunningControl() - m_owner->minRunningControl() );
				vel *= lerp( m_owner->minRunningSpeed(), m_owner->maxRunningSpeed(), s );
				movementVector *= lerp( m_owner->minRunningSpeed(), m_owner->maxRunningSpeed(), s );
			}
		}
	}
	else if ( GameCharacter::isCrouched( m_owner->primaryState() ) )
	{
		// adjust velocity and movement vector length by scaled movement control vector length
		if ( movementControlVector.lengthSquared() > Float::MIN_VALUE )
		{
			m_owner->rotation().rotate(movementControlVector,&vel);
			movementVector = movementControlVector;

			float movementControlLen = movementControlVector.length();
			if ( movementControlLen > ZERO_CONTROL_VALUE )
			{
				vel *= m_owner->crouchWalkingSpeed();
				movementVector *= m_owner->crouchWalkingSpeed();
			}
		}
	}
	else if ( GameCharacter::isRolling( m_owner->primaryState() ) )
	{
		if ( m_owner->primaryState() == GameCharacter::PRIMARY_STATE_ROLLING_FORWARD )
		{
			movementVector += Vector3(0,0,1) * m_owner->rollingSpeedForward();
			vel += m_owner->forward() * m_owner->rollingSpeedForward();
		}	
		else if ( m_owner->primaryState() == GameCharacter::PRIMARY_STATE_ROLLING_BACKWARD )
		{
			movementVector += Vector3(0,0,-1) * m_owner->rollingSpeedBackward();
			vel -= m_owner->forward() * m_owner->rollingSpeedBackward();			
		}
		else if ( m_owner->primaryState() == GameCharacter::PRIMARY_STATE_ROLLING_LEFT )
		{
			movementVector += Vector3(-1,0,0) * m_owner->rollingSpeedSideways();
			vel -= m_owner->right() * m_owner->rollingSpeedSideways();
		}
		else if ( m_owner->primaryState() == GameCharacter::PRIMARY_STATE_ROLLING_RIGHT )
		{
			movementVector += Vector3(1,0,0) * m_owner->rollingSpeedSideways();
			vel += m_owner->right() * m_owner->rollingSpeedSideways();
		}
	}

	*movement = movementVector;
	*velocity = vel;
}

Vector3 ControlBase::scaleMovementControlVector( const Vector3& movementControlVector ) const
{
	if ( GameCharacter::isWalking( m_owner->primaryState() ) || GameCharacter::isStrafing( m_owner->primaryState() ) )
	{
		Vector3 vec = movementControlVector;
		
		for ( int i = 0 ; i < m_owner->movementControlSectors() ; ++i )
		{
			const ControlSector& sec = m_owner->getMovementControlSector(i);
			if ( sec.isInSector(vec) )
			{
				vec *= sec.controlLimit();
				break;
			}
		}

		return vec;
	}
	else if ( GameCharacter::isCrouched( m_owner->primaryState() ) )
	{
		Vector3 vec(0,0,0);
		if ( movementControlVector.length() > ZERO_CONTROL_VALUE )
		{
			// adjust by asymmetric scales
			vec = movementControlVector.normalize();
			float sx = m_owner->crouchStrafingSpeed() / m_owner->crouchWalkingSpeed();
			float szn = m_owner->crouchBackwardSpeed() / m_owner->crouchWalkingSpeed();
			vec.x *= sx;
			if ( vec.z < 0.f )
				vec.z *= szn;
		}
		return vec;
	}
	return movementControlVector;
}
