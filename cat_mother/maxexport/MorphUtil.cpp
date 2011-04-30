#include "StdAfx.h"
#include "MorphUtil.h"
#include "KeyFrame.h"
#include "KeyFrameContainer.h"
#include "resampling.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/String.h>
#include <wm3.h>

//-----------------------------------------------------------------------------

MorphR3* MorphUtil::getMorpherModifier( INode* node )
{
	// Get object from node. Abort if no object.
	::Object* obj = node->GetObjectRef();
	if ( !obj ) 
		return 0;

	// Is derived object ?
	while ( obj->SuperClassID() == GEN_DERIVOB_CLASS_ID && obj )
	{
		// Yes -> Cast.
		IDerivedObject* derivedObj = static_cast<IDerivedObject*>( obj );
						
		// Iterate over all entries of the modifier stack.
		for ( int modStackIndex = 0 ; modStackIndex < derivedObj->NumModifiers() ; ++modStackIndex )
		{
			// Get current modifier.
			Modifier* mod = derivedObj->GetModifier( modStackIndex );

			// Is this morpher modifier ?
			if ( mod->ClassID() == MR3_CLASS_ID )
			{
				// Yes -> Exit.
				return static_cast<MorphR3*>(mod);
			}
		}
		obj = derivedObj->GetObjRef();
	}

	// Not found.
	return 0;
}

int MorphUtil::sampleRate()
{
	return SGEXPORT_SAMPLE_RATE;
}

void MorphUtil::getWeightAnimation( const morphChannel& chn, Interval animRange, util::Vector<float>* anim )
{
	require( animRange.Start() <= animRange.End() );

	anim->clear();
	TimeValue dt = SGEXPORT_TICKS_PER_SAMPLE;
	// always resample whole animation ignoring current start frame
	for ( TimeValue t = 0/*animRange.Start()*/ ; t <= animRange.End() ; t += dt )
	{
		float v = 0.f;
		Interval valid = FOREVER;
		chn.cblock->GetValue( 0, t, v, valid );
		//bool keyframe = (0 != chn.cblock->KeyFrameAtTime( 0, t ));
		anim->add( v/100.f );
	}

	require( anim->size() > 0 );
}
