#include "StdAfx.h"
#include "outgoing.h"

//-----------------------------------------------------------------------------

void outgoing( const KeyFrame* key0prev, const KeyFrame* key0, const KeyFrame* key1, const KeyFrame* key1next, float* out, int channels )
{
	float t;
	int i;

	switch ( key0->interpolation )
	{
	case KeyFrame::INTERPOLATE_CATMULLROM:
		if ( key0prev ) 
		{
			t = ( key1->time - key0->time ) / ( key1->time - key0prev->time );
			for ( i = 0 ; i < channels ; ++i )
				out[i] = t * ( key0prev->getChannel(i) + key1->getChannel(i) );
		}
		else
		{
			for ( i = 0 ; i < channels ; ++i )
				out[i] = (key1->getChannel(i) - key0->getChannel(i));
		}
		break;

	case KeyFrame::INTERPOLATE_LINEAR:
		if ( key0prev ) 
		{
			t = ( key1->time - key0->time ) / ( key1->time - key0prev->time );
			for ( i = 0 ; i < channels ; ++i )
				out[i] = t * ( key0->getChannel(i) - key0prev->getChannel(i) + (key1->getChannel(i) - key0->getChannel(i)) );
		}
		else
		{
			for ( i = 0 ; i < channels ; ++i )
				out[i] = key1->getChannel(i) - key0->getChannel(i);
		}
		break;

	case KeyFrame::INTERPOLATE_STEPPED:
	default:
		for ( i = 0 ; i < channels ; ++i )
			out[i] = 0.f;
		break;
	}
}
