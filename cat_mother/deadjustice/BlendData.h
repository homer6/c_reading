#ifndef _BLENDDATA_H
#define _BLENDDATA_H

#include "AnimationParams.h"
#include <lang/Object.h>


/** 
 * The internal state of one animation in the Blender. 
 * @author Toni Aittoniemi
 */
class BlendData :
	public lang::Object
{
public:
	/** Blend state. */
	enum BlendState
	{
		/** Blend is fading in. */
		BLEND_FADEIN,
		/** Blend has been faded in and is static. */
		BLEND_STATIC,
		/** Blend is fadeout. */
		BLEND_FADEOUT,
		/** Blend is inactive. */
		BLEND_INACTIVE
	};
	int					id;
	AnimationParams 	anim;
	BlendState			state;
	float				targetWeight;
	float				weightDeltaInSecond;

	BlendData();
	BlendData( int id, const AnimationParams& animparams, BlendState blendstate, float weight );

	void init( int id, const AnimationParams& animparams, BlendState blendstate, float weight );
};

#endif