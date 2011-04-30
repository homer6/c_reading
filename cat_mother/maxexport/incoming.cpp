#include "StdAfx.h"
#include "incoming.h"

//-----------------------------------------------------------------------------

void incoming( const KeyFrame* key0prev, const KeyFrame* key0, const KeyFrame* key1, const KeyFrame* key1next, float* in, int channels )
{
	float t;
	int i;

	switch ( key1->interpolation )
	{
	case KeyFrame::INTERPOLATE_CATMULLROM:
		if ( key1next ) 
		{
			t = ( key1->time - key0->time ) / ( key1next->time - key0->time );
			for ( i = 0 ; i < channels ; ++i )
				in[i] = t * ( key1next->getChannel(i) - key0->getChannel(i) );
		}
		else
		{
			for ( i = 0 ; i < channels ; ++i )
				in[i] = (key1->getChannel(i) - key0->getChannel(i));
		}
		break;

	case KeyFrame::INTERPOLATE_LINEAR:
		if ( key1next ) 
		{
			t = ( key1->time - key0->time ) / ( key1next->time - key0->time );
			for ( i = 0 ; i < channels ; ++i )
				in[i] = t * ( key1next->getChannel(i) - key1->getChannel(i) + (key1->getChannel(i) - key0->getChannel(i)) );
		}
		else
		{
			for ( i = 0 ; i < channels ; ++i )
				in[i] = key1->getChannel(i) - key0->getChannel(i);
		}
		break;

	case KeyFrame::INTERPOLATE_STEPPED:
	default:
		for ( i = 0 ; i < channels ; ++i )
			in[i] = 0.f;
		break;
	}
}
