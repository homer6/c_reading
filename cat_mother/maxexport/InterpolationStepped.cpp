#include "StdAfx.h"
#include "InterpolationStepped.h"
#include "KeyFrame.h"

//-----------------------------------------------------------------------------

void InterpolationStepped::interpolate( 
	const KeyFrame* /*key0prev*/, const KeyFrame* key0,
	const KeyFrame* /*key1*/, const KeyFrame* /*key1next*/,
	float t, float /*tlength*/, int channels, float* values ) const
{
	for ( int i = 0 ; i < channels ; ++i )
	{
		float res = key0->getChannel(i);
		values[i] = res;
	}
}
