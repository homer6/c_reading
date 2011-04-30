#ifndef _LANG_MATH_H
#define _LANG_MATH_H


#include <math.h>

// Undefine the macros that some arrogant header files might define.
#undef min
#undef max
#undef abs
#undef PI
#undef E


namespace lang
{


/** 
 * Math contains methods for performing basic numeric operations.
 * Functionality includes for example elementary exponential, logarithm, 
 * square root, and trigonometric functions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Math
{
public:
	/** 
	 * A value as close as possible to <i>e</i>.
	 * (the base of the natural logarithms)
	 */
	static const float E;

	/** 
	 * A value as close as possible to <i>pi</i>.
	 * (the ratio of the circumference of a circle to its diameter)
	 */
	static const float PI;

	/** Returns the absolute value of a double. */
	static double	abs( double x )													{return ::fabs(x);}

	/** Returns the absolute value of a float. */
	static float	abs( float x )													{return ::fabsf(x);}

	/** Returns the absolute value of an int. */ 
	static int		abs( int x )													{return x < 0 ? -x : x;}

	/** Returns the absolute value of a long. */ 
	static long		abs( long x )													{return x < 0 ? -x : x;}

	/** Returns the greater of two doubles. */ 
	static double	max( double x, double y )										{return x > y ? x : y;}

	/** Returns the greater of two floats. */ 
	static float	max( float x, float y )											{return x > y ? x : y;}

	/** Returns the greater of two ints. */ 
	static int		max( int x, int y )												{return x > y ? x : y;}

	/** Returns the greater of two longs. */ 
	static long		max( long x, long y )											{return x > y ? x : y;}

	/** Returns the smaller of two doubles. */ 
	static double	min( double x, double y )										{return x < y ? x : y;}

	/** Returns the smaller of two floats. */ 
	static float	min( float x, float y )											{return x < y ? x : y;}

	/** Returns the smaller of two ints. */ 
	static int		min( int x, int y )												{return x < y ? x : y;}

	/** Returns the smaller of two longs. */ 
	static long		min( long x, long y )											{return x < y ? x : y;}

	/** Returns the closest long to the argument. */ 
	static long		round( double x )												{return (long)::floor(x+.5);}

	/** Returns the closest int to the argument. */ 
	static int		round( float x )												{return (int)::floorf(x+.5f);}

	/** Returns a value in range [0,1). */
	static float	random();

	/** Returns the arc cosine of an angle, in the range of 0.0 through pi. */ 
	static double	acos( double x )												{return ::acos(x);}

	/** Returns the arc sine of an angle, in the range of -pi/2 through pi/2. */ 
	static double	asin( double x )												{return ::asin(x);}

	/** Returns the arc tangent of an angle, in the range of -pi/2 through pi/2. */ 
	static double	atan( double x )												{return ::atan(x);}

	/** Converts rectangular coordinates (b, a) to polar (r, theta). */ 
	static double	atan2( double x, double y )										{return ::atan2(x,y);}

	/** Returns the smallest (closest to negative infinity) value that is not less than the argument and is equal to a mathematical integer. */ 
	static double	ceil( double x )												{return ::ceil(x);}

	/** Returns the trigonometric cosine of an angle. */ 
	static double	cos( double x )													{return ::cos(x);}

	/** Returns the exponential number e (i.e., 2.718...) raised to the power of a value. */ 
	static double	exp( double x )													{return ::exp(x);}

	/** Returns the largest (closest to positive infinity) value that is not greater than the argument and is equal to a mathematical integer. */ 
	static double	floor( double x )												{return ::floor(x);}

	/** Returns the natural logarithm (base e) of a value. */ 
	static double	log( double x )													{return ::log(x);}

	/** Returns of value of the first argument raised to the power of the second argument. */ 
	static double	pow( double x, double y )										{return ::pow(x,y);}

	/** Returns the value that is closest in value to a and is equal to a mathematical integer. */ 
	static double	rint( double x )												{return ::floor(x+.5);}

	/** Returns the trigonometric sine of an angle. */ 
	static double	sin( double x )													{return ::sin(x);}

	/** Returns the correctly rounded positive square root of a value. */ 
	static double	sqrt( double x )												{return ::sqrt(x);}

	/** Returns the trigonometric tangent of an angle. */ 
	static double	tan( double x )													{return ::tan(x);}

	/** Converts an angle measured in radians to the equivalent angle measured in degrees. */ 
	static double	toDegrees( double radians )										{return radians * 57.29577951308232;}

	/** Converts an angle measured in degrees to the equivalent angle measured in radians. */ 
	static double	toRadians( double degrees )										{return degrees * 0.017453292519943295;}

	/** Returns the arc cosine of an angle, in the range of 0.0 through pi. */ 
	static float	acos( float x )													{return ::acosf(x);}

	/** Returns the arc sine of an angle, in the range of -pi/2 through pi/2. */ 
	static float	asin( float x )													{return ::asinf(x);}

	/** Returns the arc tangent of an angle, in the range of -pi/2 through pi/2. */ 
	static float	atan( float x )													{return ::atanf(x);}

	/** Converts rectangular coordinates (b, a) to polar (r, theta). */ 
	static float	atan2( float x, float y )										{return ::atan2f(x,y);}

	/** Returns the smallest (closest to negative infinity) value that is not less than the argument and is equal to a mathematical integer. */ 
	static float	ceil( float x )													{return ::ceilf(x);}

	/** Returns the trigonometric cosine of an angle. */ 
	static float	cos( float x )													{return ::cosf(x);}

	/** Returns the exponential number e (i.e., 2.718...) raised to the power of a value. */ 
	static float	exp( float x )													{return ::expf(x);}

	/** Returns the largest (closest to positive infinity) value that is not greater than the argument and is equal to a mathematical integer. */ 
	static float	floor( float x )												{return ::floorf(x);}

	/** Returns the natural logarithm (base e) of a value. */ 
	static float	log( float x )													{return ::logf(x);}

	/** Returns of value of the first argument raised to the power of the second argument. */ 
	static float	pow( float x, float y )											{return ::powf(x,y);}

	/** Returns the value that is closest in value to a and is equal to a mathematical integer. */ 
	static float	rint( float x )													{return ::floorf(x+.5f);}

	/** Returns the trigonometric sine of an angle. */ 
	static float	sin( float x )													{return ::sinf(x);}

	/** Returns the correctly rounded positive square root of a value. */ 
	static float	sqrt( float x )													{return ::sqrtf(x);}

	/** Returns the trigonometric tangent of an angle. */ 
	static float	tan( float x )													{return ::tanf(x);}

	/** Converts an angle measured in radians to the equivalent angle measured in degrees. */ 
	static float	toDegrees( float radians )										{return radians * 57.29577951308232f;}

	/** Converts an angle measured in degrees to the equivalent angle measured in radians. */ 
	static float	toRadians( float degrees )										{return degrees * 0.017453292519943295f;}

private:
	Math();
	Math( const Math& );;
	Math& operator=( const Math& );;
};


} // lang


#endif // _LANG_MATH_H
