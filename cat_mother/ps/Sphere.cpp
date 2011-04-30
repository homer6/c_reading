#include "Sphere.h"
#include <lang/Math.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace ps
{


Sphere::Sphere( const Vector3& pos, float r )
{
	m_pos = pos;
	m_r = r;
}

void Sphere::getRandomPoint( Vector3* point )
{
	// axis by axis
	float r = m_r;
	float x = Math::random()*2.f - 1.f;
	float d2 = 1.f - x*x;
	if ( d2 < 0.f )
		d2 = 0.f;
	float d = Math::sqrt( d2 );
	float y = (Math::random()*2.f - 1.f) * d;
	d2 = 1.f - x*x - y*y;
	if ( d2 < 0.f )
		d2 = 0.f;
	d = Math::sqrt( d2 );
	float z = (Math::random()*2.f - 1.f) * d;
	*point = m_pos;
	*point += Vector3( x*r, y*r, z*r );

/*
	// rejection method
	float r = m_r;
	Vector3 p(0,0,0);
	int n = 6;			// avg number of tries is 1.9 so this should be ok
	float r2 = r*r;
	float twor = r*2.f;
	float sqr;
	do
	{
		sqr = 0.f;
		for ( int i = 0 ; i < 3 ; ++i )
		{
			float v = (Math::random()-0.5f) * twor;
			sqr += v*v;
			p[i] = v;
		}
	} while ( sqr > r2 && --n > 0 );

	*point = m_pos;
	if ( sqr < r2 )
		*point += p;
*/
}


} // ps
