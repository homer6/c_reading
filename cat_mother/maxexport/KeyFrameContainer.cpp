#include "StdAfx.h"
#include "KeyFrameContainer.h"
#include "KeyFrame.h"
#include "InterpolationCR.h"
#include "InterpolationLinear.h"
#include "InterpolationStepped.h"
#include "InterpolationQuaternionSlerp.h"
#include <util/Vector.h>
#include <assert.h>
#include <math.h>
#include <algorithm>

//-----------------------------------------------------------------------------

using namespace util;

//-----------------------------------------------------------------------------

class KeyFrameContainer::KeyFrameContainerImpl :
	public lang::Object
{
public:
	Vector<KeyFrame>			keys;
	bool						sorted;
	KeyFrame::BehaviourType		postBehaviour;
	int							channels;
	KeyFrame::DataType			data;

	explicit KeyFrameContainerImpl( int channels, KeyFrame::DataType data ) :
		keys( Allocator<KeyFrame>(__FILE__,__LINE__) )
	{
		assert( channels > 0 && channels <= KeyFrame::MAX_CHANNELS );
		sorted = true;
		postBehaviour = KeyFrame::BEHAVIOUR_REPEAT;
		this->channels = channels;
		this->data = data;
	}

	/** Finds key from specified range. */
	bool findKey( float time, int begin, int end, int* key ) const
	{
		assert( begin >= 0 && (begin < (int)keys.size() || begin == end) );
		assert( end >= 0 && end <= (int)keys.size() );
		assert( begin <= end );
		assert( key );

		for ( int i = begin ; i < end ; ++i )
		{
			if ( keys[i].time <= time && time < keys[i+1].time )
			{
				*key = i;
				return true;
			}
		}
		return false;
	}

	/** Sorts keys if needed. */
	void validate()
	{
		if ( !sorted )
		{
			std::sort( keys.begin(), keys.end() );
			keys.setSize( std::unique( keys.begin(), keys.end() ) - keys.begin() );
			sorted = true;
		}
	}
};

//-----------------------------------------------------------------------------

KeyFrameContainer::KeyFrameContainer( int channels, KeyFrame::DataType data )
{
	m_this = new KeyFrameContainerImpl( channels, data );
}

KeyFrameContainer::~KeyFrameContainer()
{
}

void KeyFrameContainer::insertKey( const KeyFrame& key )
{
	assert( m_this );
	m_this->keys.add( key );
	m_this->sorted = false;
}

void KeyFrameContainer::removeKey( int index )
{
	assert( m_this );
	assert( index >= 0 && index < keys() );
	m_this->keys.remove( index );
}

int	KeyFrameContainer::channels() const
{
	if ( m_this )
		return m_this->channels;
	else
		return 0;
}

float KeyFrameContainer::length() const
{
	if ( m_this )
	{
		m_this->validate();
		int keys = m_this->keys.size();
		if ( keys > 0 )
			return m_this->keys[keys-1].time;
	}
	return 0.f;
}

int KeyFrameContainer::keys() const
{
	if ( m_this )
	{
		m_this->validate();
		return m_this->keys.size();
	}
	else
	{
		return 0;
	}
}

KeyFrame& KeyFrameContainer::getKey( int index )
{
	assert( m_this );
	assert( index >= 0 && index < keys() );
	m_this->validate();
	return m_this->keys[index];
}

const KeyFrame& KeyFrameContainer::getKey( int index ) const
{
	assert( m_this );
	assert( index >= 0 && index < keys() );
	m_this->validate();
	return m_this->keys[index];
}

void KeyFrameContainer::setEndBehaviour( KeyFrame::BehaviourType type )
{
	assert( m_this );
	m_this->postBehaviour = type;
}

KeyFrame::BehaviourType KeyFrameContainer::endBehaviour() const
{
	assert( m_this );
	return m_this->postBehaviour;
}

int	KeyFrameContainer::getValue( float time, float* values, int channels, int hintKey ) const
{
	assert( m_this );
	assert( keys() > 0 );
	assert( channels == this->channels() );

	m_this->validate();

	const int	keys		= m_this->keys.size();
	const float	length		= (float)m_this->keys[keys-1].time;

	// handle 1 key motion as special case
	if ( 1 == keys )
	{
		m_this->keys[0].getChannels( values, channels );
		return 0;
	}

	// validate hint key
	int newHintKey = hintKey;
	if ( newHintKey < 0 )
		newHintKey = 0;
	else if ( newHintKey >= keys )
		newHintKey = keys-1;

	// pre behaviour (constant)
	if ( time <= m_this->keys[0].time )
	{
		m_this->keys[0].getChannels( values, channels );
		return 0;
	}

	// post behaviour
	if ( time >= m_this->keys[keys-1].time )
	{
		switch ( m_this->postBehaviour )
		{
		case KeyFrame::BEHAVIOUR_RESET:
			time = 0.f;
			break;

		case KeyFrame::BEHAVIOUR_REPEAT:
			time = fmodf( time, length );
			break;

		case KeyFrame::BEHAVIOUR_CONSTANT:
			m_this->keys[keys-1].getChannels( values, channels );
			return keys-1;

		case KeyFrame::BEHAVIOUR_OSCILLATE:
			time = fmodf( time, 2.f*length );
			if ( time >= length )
				time = 2.f*length - time;
			if ( time >= m_this->keys[keys-1].time )
			{
				m_this->keys[keys-1].getChannels( values, channels );
				return keys-1;
			}
			break;

		default:
			m_this->keys[keys-1].getChannels( values, channels );
			return keys-1;
		}
	}

	// after pre/post behaviour we should have time in valid range
	assert( time >= m_this->keys[0].time );
	assert( time < m_this->keys[keys-1].time );

	// find key pair
	bool keyfound = m_this->findKey( time, newHintKey, keys, &newHintKey );
	if ( !keyfound )
		keyfound = m_this->findKey( time, 0, newHintKey, &newHintKey );
	assert( keyfound );

	// get surrounding keys
	int nextKeyIndex = newHintKey+1;
	assert( nextKeyIndex >= 0 && nextKeyIndex < keys );
	const KeyFrame* key0			= &m_this->keys[ newHintKey ];
	const KeyFrame* key1			= &m_this->keys[ nextKeyIndex ];
	const KeyFrame* key0prev		= 0;
	const KeyFrame* key1next		= 0;

	if ( newHintKey > 0 )
		key0prev = &m_this->keys[newHintKey-1];
	if ( nextKeyIndex+1 < keys )
		key1next = &m_this->keys[nextKeyIndex+1];

    // get tween length and fractional tween position
    float tlength = key1->time - key0->time;
	assert( tlength > 1e-6f );
	float t = (time - key0->time) / (float)tlength;
	assert( t >= 0.f && t < 1.f );

	// compute the channel values
	if ( KeyFrame::DATA_SCALAR == m_this->data )
	{
		switch ( key1->interpolation )
		{
		case KeyFrame::INTERPOLATE_CATMULLROM:
			InterpolationCR().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;

		case KeyFrame::INTERPOLATE_LINEAR:
			InterpolationLinear().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;

		case KeyFrame::INTERPOLATE_STEPPED:
			InterpolationStepped().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;

		default:
			InterpolationLinear().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;
		}
	}
	else if ( KeyFrame::DATA_QUATERNION == m_this->data )
	{
		switch ( key1->interpolation )
		{
		case KeyFrame::INTERPOLATE_STEPPED:
			InterpolationStepped().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;

		case KeyFrame::INTERPOLATE_LINEAR:
			InterpolationQuaternionSlerp().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;

		case KeyFrame::INTERPOLATE_CATMULLROM:
			InterpolationQuaternionSlerp().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;

		default:
			InterpolationQuaternionSlerp().interpolate( key0prev, key0, key1, key1next, t, tlength, channels, values );
			break;
		}
	}
	else
	{
		for ( int i = 0 ; i < channels ; ++i )
			values[i] = 0.f;
	}

	return newHintKey;
}

KeyFrameContainer::KeyFrameContainer( const KeyFrameContainer& other )
{
	m_this = new KeyFrameContainerImpl( *other.m_this );
}

KeyFrameContainer& KeyFrameContainer::operator=( const KeyFrameContainer& other )
{
	*m_this = *other.m_this;
	return *this;
}

KeyFrame::DataType KeyFrameContainer::data() const
{
	return m_this->data;
}

void KeyFrameContainer::clear()
{
	m_this->keys.clear();
}

KeyFrame::InterpolationType KeyFrameContainer::interpolation() const
{
	if ( keys() > 1 )
		return m_this->keys[0].interpolation;
	else
		return KeyFrame::INTERPOLATE_LINEAR;
}
