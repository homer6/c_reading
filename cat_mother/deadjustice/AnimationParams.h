#ifndef _ANIMATIONPARAMS_H
#define _ANIMATIONPARAMS_H


#include <lang/String.h>


/**
 * @author Toni Aittoniemi
 */
class AnimationParams 
{
public:
	AnimationParams() :
		blendTime( 0.f ),
		blendDelay( 0.f ),
		time( 0.f ),
		name( "" ),
		weight( 0.f ),
		speed( 1.f )
	{
	}

	/** Curent phase of blend in seconds. */
	float			blendTime;
	/** Amount of time required to blend this animation in or out. */
	float			blendDelay;
	/** Current time of animation in seconds. */
	float			time;
	/** Animation name, a valid nodeGroupSet. */
	lang::String	name;
	/** Blend weight */
	float			weight;
	/** Multiplier to delta time */
	float			speed;
};


#endif // _ANIMATIONPARAMS_H
