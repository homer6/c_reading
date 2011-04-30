#include "StdAfx.h"
#include "InterpolationLinear.h"
#include "KeyFrame.h"

//-----------------------------------------------------------------------------

void InterpolationLinear::interpolate( 
	const KeyFrame* /*key0prev*/, const KeyFrame* key0,
	const KeyFrame* key1, const KeyFrame* /*key1next*/,
	float t, float /*tlength*/, int channels, float* value ) const
{
	for ( int i = 0 ; i < channels ; ++i )
	{
		float d10 = key1->getChannel(i) - key0->getChannel(i);
		float res = key0->getChannel(i) + t * d10;
		value[i] = res;
	}
}
