#include "Double.h"
#include "NumberParse.h"
#include <limits>
#include "config.h"

//-----------------------------------------------------------------------------

#undef min
#undef max

//-----------------------------------------------------------------------------

namespace lang
{


double	Double::MAX_VALUE			= std::numeric_limits<double>::max();
double	Double::MIN_VALUE			= std::numeric_limits<double>::min();
double	Double::POSITIVE_INFINITY	= std::numeric_limits<double>::infinity();
double	Double::NEGATIVE_INFINITY	= -std::numeric_limits<double>::infinity();
double	Double::NaN					= std::numeric_limits<double>::quiet_NaN();

//-----------------------------------------------------------------------------

double Double::parseDouble( const lang::String& str )
{
	return NumberParse<double>::parse( str );
}


} // lang
