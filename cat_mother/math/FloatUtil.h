#ifndef _MATH_FLOATUTIL_H
#define _MATH_FLOATUTIL_H


namespace math
{


/** 
 * Floating point support routines. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class FloatUtil
{
public:
	/** 
	 * Returns integer part of specified floating point value.
	 * Note that rounding type is dependent on FPU state
	 * so this routine should not be used when absolute accuracy is required.
	 */
	static int		floatToInt( float x );
};


#include "FloatUtil.inl"


} // math


#endif // _MATH_FLOATUTIL_H
