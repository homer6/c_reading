#include "Animatable.h"
#include "config.h"

//-----------------------------------------------------------------------------

namespace anim
{


void Animatable::setState( float time )
{
	Animatable* anims[] = {this};
	float times[] = {time};
	float weights[] = {1.f};
	blendState( anims, times, weights, 1 );
}


} // anim
