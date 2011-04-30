#include "Float.h"
#include "NumberParse.h"
#include <limits>
#include "config.h"

//-----------------------------------------------------------------------------

#undef min
#undef max

//-----------------------------------------------------------------------------

namespace lang
{


float	Float::MAX_VALUE			= std::numeric_limits<float>::max();
float	Float::MIN_VALUE			= std::numeric_limits<float>::min();
float	Float::POSITIVE_INFINITY	= std::numeric_limits<float>::infinity();
float	Float::NEGATIVE_INFINITY	= -std::numeric_limits<float>::infinity();
float	Float::NaN					= std::numeric_limits<float>::quiet_NaN();

//-----------------------------------------------------------------------------

float Float::parseFloat( const lang::String& str )
{
	return NumberParse<float>::parse( str );
}


} // lang
