#include "Colorf.h"
#include "Color.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace pix
{


inline static float convertColorByteToFloat( uint8_t src )
{
	float f = float(src)*(1.f/255.f);
	if ( f < 0.f )
		f = 0.f;
	else if ( f > 1.f )
		f = 1.f;
	
	assert( f >= 0.f && f <= 1.f );
	return f;
}

//-----------------------------------------------------------------------------

Colorf::Colorf( const Color& source )
{
	setRed( convertColorByteToFloat(source.red()) );
	setGreen( convertColorByteToFloat(source.green()) );
	setBlue( convertColorByteToFloat(source.blue()) );
	setAlpha( convertColorByteToFloat(source.alpha()) );
}

Colorf Colorf::saturate() const
{
	Colorf c;
	for ( int i = 0 ; i < SIZE ; ++i )
	{
		if ( m_c[i] < 0.f )
			c.m_c[i] = 0.f;
		else if ( m_c[i] > 1.f )
			c.m_c[i] = 1.f;
		else
			c.m_c[i] = m_c[i];
	}
	return c;
}

bool Colorf::finite() const
{
	for ( int i = 0 ; i < SIZE ; ++i )
	{
		if ( !(m_c[i] > -1e3f && m_c[i] < 1e3f) )
			return false;
	}
	return true;
}

float Colorf::brightness() const
{
	return (m_c[RED_INDEX] + m_c[GREEN_INDEX] + m_c[BLUE_INDEX]) * (1.f/3.f);
}


} // pix
