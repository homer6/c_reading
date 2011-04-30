#include "Gravity.h"
#include <lang/Float.h>
#include <lang/Math.h>
#include <math/Vector3.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace ps
{


const float Gravity::EARTH_RADIUS	= 6.37e6f;
const float Gravity::EARTH_MASS		= 5.98e24f;
const float Gravity::MOON_RADIUS	= 1.74e6f;
const float Gravity::MOON_MASS		= 7.36e22f;

//-----------------------------------------------------------------------------

Gravity::Gravity( float value )
{
	m_std	= true;
	m_g		= value;
}

Gravity::Gravity( const math::Vector3& pos, float mass )
{
	m_std	= false;
	m_pos	= pos;
	m_mass	= mass;
}

void Gravity::apply( float /*time*/, float /*dt*/,
	Vector3* positions, 
	Vector3* /*velocities*/,
	Vector3* forces,
	int count )
{
	if ( m_std )
	{
		for ( int i = 0 ; i < count ; ++i )
			forces[i].y -= m_g;
	}
	else
	{
		const float G				= 6.672e-11f;	// gravitational constant
		const float particleMass	= 1.f;

		for ( int i = 0 ; i < count ; ++i )
		{
			Vector3 d = m_pos - positions[i];
			float r2 = d.lengthSquared();
			if ( r2 > Float::MIN_VALUE )
			{
				float invr2 = 1.f / r2;
				float f = G * m_mass * particleMass * invr2;
				d *= Math::sqrt(invr2) * f;
				forces[i] += d;
			}
		}
	}
}


} // ps
