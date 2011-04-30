#include "Color.h"
#include "Colorf.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace pix
{


inline static uint8_t convertColorFloatToByte( float src )
{
	float f = src;
	if ( f < 0.f )
		f = 0.f;
	else if ( f > 1.f )
		f = 1.f;
	int i = (int)( f*255.f + .5f );
	if ( i < 0 )
		i = 0;
	else if ( i > 255 )
		i = 255;

	assert( i >= 0 && i < 256 );
	return (uint8_t)i;
}

//-----------------------------------------------------------------------------

Color::Color( const Colorf& source )
{
	setRed( convertColorFloatToByte(source.red()) );
	setGreen( convertColorFloatToByte(source.green()) );
	setBlue( convertColorFloatToByte(source.blue()) );
	setAlpha( convertColorFloatToByte(source.alpha()) );
}



} // pix
