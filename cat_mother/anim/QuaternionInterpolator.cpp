#include "QuaternionInterpolator.h"
#include "Quat.h"
#include <lang/Float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace anim
{


QuaternionInterpolator::QuaternionInterpolator() :
	Interpolator(4), 
	m_interp(INTERPOLATE_LINEAR)
{
}

QuaternionInterpolator::QuaternionInterpolator( const QuaternionInterpolator& other ) :
	Interpolator( other ),
	m_interp( other.m_interp )
{
}

void QuaternionInterpolator::setInterpolation( InterpolationType interp )
{
	m_interp = interp;
}

int	QuaternionInterpolator::getValue( float time, float* value, int size, int hint ) const
{
	assert( keys() > 0 );
	assert( size == channels() );

	// single key?
	int last = keys() - 1;
	if ( 0 == last )
	{
		getKeyValue( last, value, size );
		return last;
	}

	// find key frame and normalize time
	time = getNormalizedTime( time );
	int key = findKey( time, hint );

	// end of animation?
	if ( time >= getKeyTime(last) )
	{
		getKeyValue( last, value, size );
		return last;
	}
	assert( time < getKeyTime(last) );
	assert( key < last );

	// interpolate
	switch ( m_interp )
	{
	case INTERPOLATE_STEPPED:{
		getKeyValue( key, value, size );
		break;}

	case INTERPOLATE_LINEAR:{
		// get keys, ensure acute angle
		const float* p = getKeyValue(key);
		float q[4];
		getKeyValue( key+1, q, 4 );
		if ( Quat::dot(p,q) < 0.f )
			Quat::negate( q, q );

		// interpolate
		float keyInterval = getKeyTime(key+1) - getKeyTime(key);
		assert( keyInterval > Float::MIN_VALUE );
		float u = (time - getKeyTime(key)) / keyInterval;
		assert( u >= 0.f && u <= 1.f );
		Quat::slerp( value, u, p, q );
		Quat::normalize( value, value );
		break;}
	}

	return key;
}

QuaternionInterpolator::InterpolationType QuaternionInterpolator::interpolation() const
{
	return m_interp;
}


} // anim
