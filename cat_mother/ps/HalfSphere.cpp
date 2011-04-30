#include "HalfSphere.h"
#include <lang/Math.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace ps
{


HalfSphere::HalfSphere( const Vector3& pos, float r )
{
	m_pos = pos;
	m_r = r;
}

void HalfSphere::getRandomPoint( Vector3* point )
{
	float r = m_r;
	Vector3 p(0,0,0);
	int n = 10;			// avg number of tries is 1.8 so this should be ok
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

	if ( p[1] < 0.f )
		p[1] = -p[1];

	*point = m_pos;
	if ( sqr < r2 )
		*point += p;
}


} // ps
