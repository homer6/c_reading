#include "VectorInterpolator.h"
#include <lang/Float.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace anim
{


VectorInterpolator::VectorInterpolator( int channels ) :
	Interpolator(channels), 
	m_interp(INTERPOLATE_LINEAR)
{
}

VectorInterpolator::VectorInterpolator( const VectorInterpolator& other ) :
	Interpolator( other ),
	m_interp( other.m_interp )
{
}

void VectorInterpolator::setInterpolation( InterpolationType interp )
{
	m_interp = interp;
}

int	VectorInterpolator::getValue( float time, float* value, int size, int hint ) const
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
		const float* v1 = getKeyValue(key);
		const float* v2 = getKeyValue(key+1);
		float keyInterval = getKeyTime(key+1) - getKeyTime(key);
		assert( keyInterval > Float::MIN_VALUE );
		float u = (time - getKeyTime(key)) / keyInterval;
		for ( int i = 0 ; i < size ; ++i )
			value[i] = v1[i] + (v2[i]-v1[i])*u;
		break;}

	case INTERPOLATE_CATMULLROM:{
		// outgoing/incoming tangents
		int channels = this->channels();
		float* buf = getTempBuffer(channels*2);
		float* out = buf;
		float* in = buf+channels;

		// outgoing tangent
		if ( key > 0 )
		{
			const float* v0 = getKeyValue(key-1);
			const float* v2 = getKeyValue(key+1);
			float n01 = getKeyTime(key) - getKeyTime(key-1);
			float n02 = getKeyTime(key+1) - getKeyTime(key-1);
			float t = n01 / n02;
			for ( int i = 0 ; i < channels ; ++i )
				out[i] = t * (v2[i] - v0[i]);
		}
		else
		{
			const float* v1 = getKeyValue(key);
			const float* v2 = getKeyValue(key+1);
			for ( int i = 0 ; i < channels ; ++i )
				out[i] = v2[i] - v1[i];
		}

		// incoming tangent
		if ( key+2 <= last )
		{
			const float* v1 = getKeyValue(key);
			const float* v3 = getKeyValue(key+2);
			float n12 = getKeyTime(key+1) - getKeyTime(key);
			float n13 = getKeyTime(key+2) - getKeyTime(key);
			float t = n12 / n13;
			for ( int i = 0 ; i < channels ; ++i )
				in[i] = t * (v3[i] - v1[i]);
		}
		else
		{
			const float* v1 = getKeyValue(key);
			const float* v2 = getKeyValue(key+1);
			for ( int i = 0 ; i < channels ; ++i )
				in[i] = v2[i] - v1[i];
		}

		// Hermite interpolation
		float keyInterval = getKeyTime(key+1) - getKeyTime(key);
		assert( keyInterval > Float::MIN_VALUE );
		float u = (time - getKeyTime(key)) / keyInterval;
		float u2 = u * u;
		float u3 = u * u2;
		float utmp1 = 2.f * u3 - 3.f * u2;
		float h1 = utmp1 + 1.f;
		float h2 = -utmp1;
		float h3 = u3 - 2.f * u2 + u;
		float h4 = u3 - u2;
		const float* v1 = getKeyValue(key);
		const float* v2 = getKeyValue(key+1);
		for ( int i = 0 ; i < size ; ++i )
			value[i] = h1 * v1[i] + h2 * v2[i] + h3 * out[i] + h4 * in[i];
		break;}
	}

	return key;
}

VectorInterpolator::InterpolationType VectorInterpolator::interpolation() const
{
	return m_interp;
}


} // anim
