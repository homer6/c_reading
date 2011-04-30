#include <tester/Test.h>
#include <math/Quaternion.h>
#include <math/Matrix3x3.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

static bool isNear( float a, float b )
{
	float d = a - b;
	if ( d < 0.f )
		d = -d;
	return d < 1e-4f;
}

static bool isNear( const Matrix3x3& a, const Matrix3x3& b )
{
	for ( int j = 0 ; j < 3 ; ++j )
		for ( int i = 0 ; i < 3 ; ++i )
			if ( !isNear( a(j,i), b(j,i) ) )
				return false;
	return true;
}

static int test()
{
	Matrix3x3 mx( Vector3(1,0,0), 0.23f );
	Matrix3x3 my( Vector3(0,1,0), 1.23f );
	Matrix3x3 mz( Vector3(0,0,1), 2.13f );
	Matrix3x3 m = mx * my * mz;
	Quaternion q( m );
	Matrix3x3 m1( q );
	assert( isNear(m1,m) );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
