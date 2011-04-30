#ifndef _MATH_LERP_H
#define _MATH_LERP_H


namespace math
{


/** 
 * Linear interpolation of clamped range [a,b]. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> T lerp( const T& a, const T& b, float s )
{
	if ( s < 0.f )
		s = 0.f;
	else if ( s > 1.f )
		s = 1.f;
	return (b-a)*s + a;
}


} // math


#endif // _MATH_LERP_H
