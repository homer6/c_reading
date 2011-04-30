#include "ControlSector.h"
#include <lang/Math.h>
#include <lang/Debug.h>
#include <math/Vector3.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

ControlSector::ControlSector() :
	m_minAngle(0.f), 
	m_maxAngle(0.f), 
	m_controlLimit(0.f)
{
}

ControlSector::ControlSector( float minAngle, float maxAngle, float controlLimit ) :
	m_minAngle( minAngle ), 
	m_maxAngle( maxAngle ), 
	m_controlLimit( controlLimit )
{
}

bool ControlSector::isInSector( const Vector3& movementControlVector ) const
{
	float angle = Math::atan2( movementControlVector.z, movementControlVector.x );
	if ( angle < 0.f )
		angle += Math::PI*2.f;
	
	return angle >= m_minAngle && angle < m_maxAngle;
}

float ControlSector::controlLimit() const
{
	return m_controlLimit;
}

float ControlSector::remapAngle( float angle )
{
	const float HALF_PI = Math::PI*.5f;

	float oldangle = angle;

	if ( angle >= 0.f && angle <= HALF_PI )		// Q1
		angle = HALF_PI - angle;
	else if ( angle <= 0.f && angle >= -HALF_PI )		// Q2
		angle = HALF_PI - angle;
	else if ( angle <= -HALF_PI && angle >= -Math::PI )		// Q3
		angle = -Math::PI - HALF_PI - angle;
	else if ( angle >= HALF_PI && angle <= Math::PI )		// Q3
		angle = HALF_PI - angle;
	if ( angle < 0.f )
		angle += Math::PI;

	Debug::println( "ControlSector mapped angle {0} to {1} degrees", Math::toDegrees(oldangle), Math::toDegrees(angle) );
	return angle;
}
