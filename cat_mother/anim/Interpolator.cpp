#include "Interpolator.h"
#include "PtrLess.h"
#include <lang/String.h>
#include <lang/Exception.h>
#include <math.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace anim
{


Interpolator::Interpolator( int channels ) :
	m_times( Allocator<float>(__FILE__,__LINE__) ),
	m_values( Allocator<float>(__FILE__,__LINE__) ),
	m_temp( Allocator<float>(__FILE__,__LINE__) ),
	m_channels( channels ), 
	m_endBehaviour(BEHAVIOUR_REPEAT),
	m_preBehaviour(BEHAVIOUR_CONSTANT)
{
}

Interpolator::Interpolator( const Interpolator& other ) :
	m_times( other.m_times ),
	m_values( other.m_values ),
	m_temp( other.m_temp ),
	m_channels( other.m_channels ),
	m_endBehaviour( other.m_endBehaviour ),
	m_preBehaviour( other.m_preBehaviour )
{
}

void Interpolator::setKeys( int count )
{
	m_times.setSize( count );
	m_values.setSize( count*m_channels );
}

void Interpolator::setKeyValue( int i, const float* value, int size )
{
	assert( i >= 0 && i < keys() );
	assert( size == m_channels );

	float* v = m_values.begin() + i*m_channels;
	for ( int i = 0 ; i < size ; ++i )
		v[i] = value[i];
}

void Interpolator::setKeyTime( int i, float time )
{
	assert( i >= 0 && i < keys() );
	
	m_times[i] = time;
}

void Interpolator::setEndBehaviour( Interpolator::BehaviourType behaviour )
{
	m_endBehaviour = behaviour;
}

void Interpolator::setPreBehaviour( Interpolator::BehaviourType behaviour )
{
	m_preBehaviour = behaviour;
}

void Interpolator::getKeyValue( int i, float* value, int size ) const
{
	assert( i >= 0 && i < keys() );
	assert( size == m_channels );

	const float* v = m_values.begin() + i*m_channels;
	for ( int i = 0 ; i < size ; ++i )
		value[i] = v[i];
}

const float* Interpolator::getKeyValue( int i ) const
{
	assert( i >= 0 && i < keys() );
	return m_values.begin() + i*m_channels;
}

float Interpolator::getKeyTime( int i ) const
{
	assert( i >= 0 && i < keys() );
	return m_times[i];
}

Interpolator::BehaviourType Interpolator::endBehaviour() const
{
	return m_endBehaviour;
}

Interpolator::BehaviourType Interpolator::preBehaviour() const
{
	return m_preBehaviour;
}

float* Interpolator::getTempBuffer( int size ) const
{
	assert( size > 0 );
	
	m_temp.setSize( size );
	return m_temp.begin();
}

int	Interpolator::findKey( float time, int hint ) const
{
	assert( keys() > 0 );

	// validate hint
	int last = m_times.size() - 1;
	if ( hint < 0 || hint > last )
		hint = 0;

	// check start/end boundaries
	if ( time <= m_times[0] )
		return 0;
	else if ( time >= m_times.lastElement() )
		return last;
	assert( time > m_times[0] && time < m_times.lastElement() );

	// find correct interval, starting from hint
	for ( int i = hint ; i < last ; ++i )
	{
		assert( i == last || m_times[i] < m_times[i+1] );	// ensure order
		if ( time >= m_times[i] && time < m_times[i+1] )
			return i;
	}
	for ( int i = 0 ; i < hint ; ++i )
	{
		assert( i == last || m_times[i] < m_times[i+1] );	// ensure order
		if ( time >= m_times[i] && time < m_times[i+1] )
			return i;
	}
	return 0;
}

float Interpolator::getTimeDirection( float time ) const
{
	if ( m_endBehaviour == BEHAVIOUR_OSCILLATE && keys() > 2 )
	{
		float startTime = m_times[0];
		float endTime = m_times.lastElement();
		float length = endTime - startTime;
		assert( length > 1e-9f );

		// start behaviour
		if ( time < startTime )
			time = startTime;

		time = fmodf( time-startTime, 2.f*length );
		if ( time >= length )
			return -1.f;
	}
	return 1.f;
}

float Interpolator::getNormalizedTime( float time ) const
{
	assert( keys() > 0 );

	if ( keys() < 2 )
		return m_times[0];

	float startTime = m_times[0];
	float endTime = m_times.lastElement();
	float length = endTime - startTime;
	assert( length > 1e-9f );

	// start behaviour
	if ( time < startTime )
	{
		switch ( m_preBehaviour )
		{
		case BEHAVIOUR_RESET:
			time = endTime;
			break;
		case BEHAVIOUR_CONSTANT:
			time = startTime;
			break;
		case BEHAVIOUR_REPEAT:
			time = endTime - fmodf( startTime-time, length );
			break;
		case BEHAVIOUR_OSCILLATE:
			time = -fmodf( startTime-time, 2.f*length );
			if ( time >= length )
				time = 2.f*length - time;
			time += startTime;
			break;
		}
	}
	// end behaviour
	switch ( m_endBehaviour )
	{
	case BEHAVIOUR_RESET:
		if ( time >= endTime )
			time = startTime;
		break;

	case BEHAVIOUR_CONSTANT:
		if ( time > endTime )
			time = endTime;
		break;

	case BEHAVIOUR_REPEAT:
		time = startTime + fmodf( time-startTime, length );
		break;

	case BEHAVIOUR_OSCILLATE:
		time = fmodf( time-startTime, 2.f*length );
		if ( time >= length )
			time = 2.f*length - time;
		time += startTime;
		break;
	}

	// ensure limits
	if ( time < startTime )
		time = startTime;
	else if ( time > endTime )
		time = endTime;

	assert( time >= startTime );
	assert( time <= endTime );
	return time;
}

void Interpolator::sortKeys()
{
	Vector<int> order( Allocator<int>(__FILE__,__LINE__) );
	order.setSize( m_times.size() );
	for ( int i = 0 ; i < m_times.size() ; ++i )
		order[i] = i;
	std::sort( order.begin(), order.end(), PtrLess<float>(m_times.begin()) );

	reorderKeys( order.begin() );
}

void Interpolator::reorderKeys( const int* order )
{
	int keys = this->keys();

	// key times
	Vector<float> times( Allocator<float>(__FILE__,__LINE__) );
	times.setSize( m_times.size() );
	for ( int i = 0 ; i < keys ; ++i )
		times[i] = m_times[ order[i] ];

	// key values
	Vector<float> values( Allocator<float>(__FILE__,__LINE__) );
	values.setSize( m_values.size() );
	for ( int i = 0 ; i < keys ; ++i )
	{
		const float* src = &m_values[ order[i] * m_channels ];
		float* dst = &values[ i * m_channels ];
		for ( int k = 0 ; k < m_channels ; ++k )
			dst[k] = src[k];
	}

	m_times = times;
	m_values = values;
}

Interpolator::BehaviourType Interpolator::toBehaviour( const String& str )
{
	if ( str == "RESET" )
		return Interpolator::BEHAVIOUR_RESET;
	else if ( str == "CONSTANT" )
		return Interpolator::BEHAVIOUR_CONSTANT;
	else if ( str == "REPEAT" )
		return Interpolator::BEHAVIOUR_REPEAT;
	else if ( str == "OSCILLATE" )
		return Interpolator::BEHAVIOUR_OSCILLATE;
	else
		throw Exception( Format("String {0} is not valid animation end behaviour", str) );
}

float Interpolator::endTime() const
{
	float t = 0.f;
	if ( m_times.size() > 0 )
		t = m_times.lastElement();
	return t;
}


} // anim
