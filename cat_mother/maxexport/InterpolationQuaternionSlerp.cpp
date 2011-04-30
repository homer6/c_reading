#include "StdAfx.h"
#include "InterpolationQuaternionSlerp.h"
#include "QuaternionUtil.h"
#include "KeyFrame.h"
#include <assert.h>

//-----------------------------------------------------------------------------

void InterpolationQuaternionSlerp::interpolate( 
	const KeyFrame* /*key0prev*/, const KeyFrame* key0,
	const KeyFrame* key1, const KeyFrame* /*key1next*/,
	float t, float tlength, int channels, float* values ) const
{
	assert( 4 == channels );

	float p[4];
	key0->getChannels( p, 4 );
	float q[4];
	key1->getChannels( q, 4 );

	if ( QuaternionUtil::dot(p,q) < 0.f )
		QuaternionUtil::negate( q, q );

	QuaternionUtil::slerp( values, t, p, q );
	QuaternionUtil::normalize( values, values );
}
