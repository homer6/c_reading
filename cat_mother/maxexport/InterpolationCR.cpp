#include "StdAfx.h"
#include "InterpolationCR.h"
#include "KeyFrame.h"
#include "hermite.h"
#include "incoming.h"
#include "outgoing.h"

//-----------------------------------------------------------------------------

void InterpolationCR::interpolate( 
	const KeyFrame* key0prev, const KeyFrame* key0,
	const KeyFrame* key1, const KeyFrame* key1next,
	float t, float tlength, int channels, float* values ) const
{
	float out[KeyFrame::MAX_CHANNELS];
	float in[KeyFrame::MAX_CHANNELS];

	outgoing( key0prev, key0, key1, key1next, out, channels );
	incoming( key0prev, key0, key1, key1next, in, channels );
	float h1, h2, h3, h4;
	hermite( t, &h1, &h2, &h3, &h4 );
	for ( int i = 0 ; i < channels ; ++i )
		values[i] = h1 * key0->getChannel(i) + h2 * key1->getChannel(i) + h3 * out[i] + h4 * in[i];
}
