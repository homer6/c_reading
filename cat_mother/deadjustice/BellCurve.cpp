#include "BellCurve.h"
#include <lang/Math.h>
#include "config.h"


using namespace lang;


// -4 and 4 are approximated as normal distribution (bell()) zero points although the curve does continue to infinity

float BellCurve::evaluateFull( float minlimit, float maxlimit, float pos ) 
{
	float range = maxlimit - minlimit;
	float midpoint = ( maxlimit + minlimit ) / 2.f;

	return bell( ( 8.f / range ) * ( pos - midpoint ) ) / 0.4f;
}

float BellCurve::evaluatePosHalf( float minlimit, float maxlimit, float pos ) 
{
	float range = ( maxlimit - minlimit ) * 2.f;
	float midpoint = minlimit;

	return bell( ( 8.f / range ) * ( pos - midpoint ) ) / 0.4f;
}

float BellCurve::evaluateNegHalf( float minlimit, float maxlimit, float pos ) 
{
	float range = ( maxlimit - minlimit ) * 2.f;
	float midpoint = maxlimit;

	return bell( ( 8.f / range ) * ( pos - midpoint ) ) / 0.4f;
}

float BellCurve::bell( float pos ) 
{
	//  e^-(x^2/2) / sqrt( 2*pi )

	return Math::exp( -Math::pow( pos, 2.f )/ 2.f ) / Math::sqrt ( 2.f * Math::PI );
}

