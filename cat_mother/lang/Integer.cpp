#include "Integer.h"
#include "NumberParse.h"
#include <limits>
#include "config.h"

//-----------------------------------------------------------------------------

#undef max
#undef min

//-----------------------------------------------------------------------------

namespace lang
{


int		Integer::MAX_VALUE = std::numeric_limits<int>::max();
int		Integer::MIN_VALUE = std::numeric_limits<int>::min();

//-----------------------------------------------------------------------------

int Integer::parseInt( const lang::String& str )
{
	return NumberParse<int>::parse( str );
}


} // lang
