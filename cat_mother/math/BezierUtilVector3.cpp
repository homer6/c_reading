#include "BezierUtilVector3.h"
#include <math.h>
#include <float.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


static float getCubicBezierCurveLengthRecurse2( const Vector3 c[4],
	float err, int recursionLimit )
{
	float poly = (c[1]-c[0]).length() + (c[2]-c[1]).length() + (c[3]-c[2]).length();
	float cord = (c[3]-c[0]).length();
	float approx = cord*.5f + poly*.5f;
	if ( poly-cord > err && recursionLimit > 0 )
	{
		Vector3 a[4], b[4];
		BezierUtil<Vector3>::splitCubicBezierCurve( c, a, b );
		return 
			getCubicBezierCurveLengthRecurse2(a,err,recursionLimit-1) + 
			getCubicBezierCurveLengthRecurse2(b,err,recursionLimit-1);
	}
	return approx;
}

//-----------------------------------------------------------------------------

float BezierUtilVector3::getCubicBezierCurveLength( const Vector3 x[4], float a, float b )
{
	// Bode's rule:
	// I_x1_x5 f(x)dx = h/45 * [14*f1 + 64*f2 + 24*f3 + 64*f4 + 14*f5]
	float h = (b-a)*(1.f/4.f);
	return h * (
		(14.f/45.f) * BezierUtil<Vector3>::getCubicBezierCurveDt(x,a).length() + 
		(64.f/45.f) * BezierUtil<Vector3>::getCubicBezierCurveDt(x,a+h).length() + 
		(24.f/45.f) * BezierUtil<Vector3>::getCubicBezierCurveDt(x,a+h*2.f).length() + 
		(64.f/45.f) * BezierUtil<Vector3>::getCubicBezierCurveDt(x,a+h*3.f).length() + 
		(14.f/45.f) * BezierUtil<Vector3>::getCubicBezierCurveDt(x,b).length() );
}

float BezierUtilVector3::getCubicBezierCurveLength2( const Vector3 x[4], 
	float a, float b, float err, int recursionLimit )
{
	bool sign = false;
	if ( a > b )
	{
		float tmp = a;
		a = b;
		b = tmp;
		sign = true;
	}

	Vector3 y[4];
	splitCubicBezierCurve( x, a, b, y );
	float len = getCubicBezierCurveLengthRecurse2( y, err, recursionLimit );

	if ( sign )
		len = -len;
	return len;
}

float BezierUtilVector3::getCubicBezierPatchFlatness( const Vector3 patch[4][4] )
{
	// get patch plane equation
	Vector3 v1 = patch[0][3]-patch[0][0];
	Vector3 v2 = patch[3][0]-patch[0][0];
	Vector3 n = v1.cross(v2);
	float nlen = n.length();
	if ( fabsf(nlen) < FLT_MIN )
		return -1.f;
	n *= 1.f / nlen;
	float plane[4] = { n.x, n.y, n.z, -patch[0][0].dot(n) };
	
	// find maximum distance to the plane
	float maxDist = 0.f;
	for ( int i = 0 ; i < 4 ; ++i )
	{
		for ( int j = 0 ; j < 4 ; ++j )
		{
			const Vector3& p = patch[i][j];
			float dist = fabsf( p.x*plane[0] + p.y*plane[1] + p.z*plane[2] + plane[3] );
			if ( dist > maxDist )
				maxDist = dist;
		}
	}

	return maxDist;
}


} // math
