#include "BlendData.h"

BlendData::BlendData() :
	id( -1 ),
	anim( AnimationParams() ),
	state( BLEND_INACTIVE ),
	targetWeight( 0 ),
	weightDeltaInSecond( 0 )
{
}


BlendData::BlendData( int _id, const AnimationParams& animparams, BlendState blendstate, float weight ) :
	id( _id ),	
	anim( animparams ),
	state( blendstate ),
	targetWeight( weight ),
	weightDeltaInSecond( (1.f / ( animparams.blendDelay > 0 ? animparams.blendDelay : 0.000001f ) ) * ( targetWeight - animparams.weight )  )
{
}

void BlendData::init( int _id, const AnimationParams& animparams, BlendState blendstate, float weight )
{
	id					= _id;
	anim				= animparams;
	state				= blendstate;
	targetWeight		= weight;
	weightDeltaInSecond = (1.f / ( animparams.blendDelay > 0 ? animparams.blendDelay : 0.000001f ) ) * ( targetWeight - animparams.weight );		
}
