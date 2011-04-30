#include "StdAfx.h"
#include "GmMorphChannel.h"

//-----------------------------------------------------------------------------

GmMorphChannel::GmMorphChannel() :
	weightAnim( 1, KeyFrame::DATA_SCALAR ),
	target(0)
{
}

GmMorphChannel::GmMorphChannel( const lang::String& chnName, 
	const KeyFrameContainer& chnWeightAnim, GmModel* chnTarget ) :
	name(chnName), 
	weightAnim(chnWeightAnim),
	target(chnTarget)
{
}

GmMorphChannel::~GmMorphChannel()
{
}
