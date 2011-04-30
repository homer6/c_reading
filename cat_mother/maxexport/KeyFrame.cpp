#include "StdAfx.h"
#include "KeyFrame.h"
#include <assert.h>

//-----------------------------------------------------------------------------

KeyFrame::KeyFrame()
{
	time			= 0.f;
	interpolation	= KeyFrame::INTERPOLATE_LINEAR;
	tension			= 0.f;
	continuity		= 0.f;
	bias			= 0.f;

	for ( int i = 0 ; i < MAX_CHANNELS ; ++i )
		m_channels[i] = 0.f;
}

KeyFrame::KeyFrame( float time, KeyFrame::InterpolationType interpolation, const float* values, int channels )
{
	this->time			= time;
	this->interpolation	= interpolation;
	tension				= 0.f;
	continuity			= 0.f;
	bias				= 0.f;

	setChannels( values, channels );
}

void KeyFrame::setChannel( int i, float value )
{
	assert( i >= 0 && i < MAX_CHANNELS );
	m_channels[i] = value;
}

void KeyFrame::setChannels( const float* values, int count )
{
	assert( count <= MAX_CHANNELS );
	for ( int i = 0 ; i < count ; ++i )
		m_channels[i] = values[i];
}
