#include "Long.h"
#include "NumberParse.h"
#include <limits>
#include "config.h"

//-----------------------------------------------------------------------------

#undef max
#undef min

//-----------------------------------------------------------------------------

namespace lang
{


long	Long::MAX_VALUE = std::numeric_limits<long>::max();
long	Long::MIN_VALUE = std::numeric_limits<long>::min();

//-----------------------------------------------------------------------------

long Long::parseLong( const lang::String& str )
{
	return NumberParse<long>::parse( str );
}


} // lang
