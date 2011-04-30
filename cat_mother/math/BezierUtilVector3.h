#ifndef _MATH_BEZIERUTILVECTOR3_H
#define _MATH_BEZIERUTILVECTOR3_H


#include <math/Vector3.h>
#include <math/BezierUtil.h>


namespace math
{


/** 
 * Bezier curve utilities specific for Vector3. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BezierUtilVector3 :
	public BezierUtil<Vector3>
{
public:
	/** 
	 * Returns length of a cubic Bezier curve segment [a,b]. 
	 * Uses constant-time Bode's integration rule (5-point version
	 * of Simpson's rule).
	 */
	static float	getCubicBezierCurveLength( const Vector3 x[4],
						float a, float b );

	/**
	 * Returns length of a cubic Bezier curve segment [a,b]. 
	 * Uses Gravesen's formula. More accurate than getCubicBezierLength
	 * but also about 10-15 times slower, depending on desired accuracy.
	 * Maximum error estimation can be taken for example from Bezier 
	 * polyline length, e.g. (|x0x1|+|x1x2|+|x2x3|)/1000.
	 */
	static float	getCubicBezierCurveLength2( const Vector3 x[4],
						float a, float b, float err, int recursionLimit=10 );

	/**
	 * Returns Bezier patch flatness. Flatness is maximum
	 * distance to the plane generated from the patch vertices (0,0),
	 * (0,3) and (3,0).
	 * @return Bezier flatness or -1f if couldn't be computed.
	 */
	static float	getCubicBezierPatchFlatness( const Vector3 patch[4][4] );
};


} // math


#endif // _MATH_BEZIERUTILVECTOR3_H
