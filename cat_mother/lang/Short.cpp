#include "Short.h"
#include "NumberParse.h"
#include <limits>
#include "config.h"

//-----------------------------------------------------------------------------

#undef max
#undef min

//-----------------------------------------------------------------------------

namespace lang
{


short	Short::MAX_VALUE = std::numeric_limits<short>::max();
short	Short::MIN_VALUE = std::numeric_limits<short>::min();

//-----------------------------------------------------------------------------

short Short::parseShort( const lang::String& str )
{
	return NumberParse<short>::parse( str );
}


} // lang
