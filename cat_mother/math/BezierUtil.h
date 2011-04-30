#ifndef _MATH_BEZIERUTIL_H
#define _MATH_BEZIERUTIL_H


#ifndef _MATH_CONFIG_INL_H
#include <math/internal/config_inl.h>
#endif


namespace math
{


/** 
 * Bezier curve utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class BezierUtil
{
public:
	/** Computes cubic Bezier curve. */
	static T	getCubicBezierCurve( const T x[4], float u );

	/** Computes total differential of cubic Bezier curve. */
	static T	getCubicBezierCurveDt( const T x[4], float u );

	/** Splits cubic Bezier curve to two curves. */
	static void splitCubicBezierCurve( const T x[4], T a[4], T b[4] );

	/** Splits cubic Bezier curve to two curves. */
	static void splitCubicBezierCurve( const T& x0, const T& x1, const T& x2, const T& x3, T* a0, T* a1, T* a2, T* a3, T* b0, T* b1, T* b2, T* b3 );

	/** Splits cubic Bezier curve segment [a,b] to separate curve. */
	static void splitCubicBezierCurve( const T x[4], float a, float b, T y[4] );

	/** 
	 * Computes cubic Bezier surface. 
	 * @param u Position [0,1] of the horizontal curve (x?0,x?1,x?2,x?3).
	 * @param v Position [0,1] of the vertical curve (x0?,x1?,x2?,x3?).
	 */
	static T	getCubicBezierPatch( const T x[4][4], float u, float v );
};


#include "BezierUtil.inl"


} // math


#endif // _MATH_BEZIERUTIL_H
