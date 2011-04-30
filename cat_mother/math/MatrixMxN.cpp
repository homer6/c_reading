#include "MatrixMxN.h"
#include <math.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//----------------------------------------------------------------------------

namespace math
{


float MatrixMxN::valueMax()														{return FLT_MAX;}
float MatrixMxN::valueMin()														{return FLT_MIN;}
float MatrixMxN::valueAbs( float value )										{return ::fabsf(value);}
float MatrixMxN::valueSqrt( float value )										{return ::sqrtf(value);}
float MatrixMxN::valueRSqrt( float value )										{assert(value>FLT_MIN||value<-FLT_MIN); return 1.f / ::sqrtf(value);}
float MatrixMxN::valueSin( float value )										{return ::sinf(value);}
float MatrixMxN::valueASin( float value )										{return ::asinf(value);}
float MatrixMxN::valueCos( float value )										{return ::cosf(value);}
float MatrixMxN::valueACos( float value )										{return ::acosf(value);}
float MatrixMxN::valueTan( float value )										{return ::tanf(value);}
float MatrixMxN::valueATan( float value )										{return ::atanf(value);}
bool MatrixMxN::valueFinite( float value )										{return ::fabsf(value) <= FLT_MAX;}
bool MatrixMxN::valueIsSigned()													{return true;}
float MatrixMxN::valueQuietNan()												{return ::sqrtf(-1.f);}

void MatrixMxN::set( float* begin, float* end, float value )
{
	do 
	{
		*begin = value;
	} while ( ++begin < end );
}

bool MatrixMxN::inFiniteRange( const float* begin, const float* end )
{
	float max = valueMax();
	float min = ( valueIsSigned() ? -max : float(0) );
	return inClosedRange(begin,end,min,max);
}

bool MatrixMxN::inClosedRange( const float* begin, const float* end, 
	float min, float max )
{
	do 
	{
		if ( *begin < min || *begin > max )
			return false;
	} while ( ++begin < end );
	return true;
}

float MatrixMxN::det3( const float* m1, const float* m2, const float* m3, int i, int j, int k )
{
	return 
		m1[i] * m2[j] * m3[k] + 
		m1[j] * m2[k] * m3[i] + 
		m1[k] * m2[i] * m3[j] - 
		m1[k] * m2[j] * m3[i] - 
		m1[j] * m2[i] * m3[k] - 
		m1[i] * m2[k] * m3[j];
}

float MatrixMxN::minElement( const float* begin, const float* end )
{
	assert( end-begin > 0 );
	float v = *begin;
	for ( ; begin != end ; ++begin )
		if ( *begin < v )
			v = *begin;
	return v;
}

float MatrixMxN::maxElement( const float* begin, const float* end )
{
	assert( end-begin > 0 );
	float v = *begin;
	for ( ; begin != end ; ++begin )
		if ( *begin > v )
			v = *begin;
	return v;
}

void MatrixMxN::minElements( float* begin, float* end, const float* a, const float* b )
{
	int c = end - begin;
	for ( int i = 0 ; i < c ; ++i )
		begin[i] = ( a[i] < b[i] ? a[i] : b[i] );
}

void MatrixMxN::maxElements( float* begin, float* end, const float* a, const float* b )
{
	int c = end - begin;
	for ( int i = 0 ; i < c ; ++i )
		begin[i] = ( a[i] > b[i] ? a[i] : b[i] );
}


} // math
