#include "TextureAnimation.h"
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace util;

//-----------------------------------------------------------------------------

TextureAnimation::TextureAnimation() :
	materials( Allocator<P(Material)>(__FILE__) ),
	time(-1.f),
	lastFrame( 0 )
{
}

TextureAnimation::TextureAnimation( const TextureAnimation& other ) :
	name( other.name ),
	frames( other.frames ),
	frameCtrl( other.frameCtrl ),
	materials( other.materials ),
	time( other.time ),
	hint( other.hint ),
	lastFrame( other.lastFrame )
{
}

TextureAnimation::~TextureAnimation()
{
}
