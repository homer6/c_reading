#include <tester/Test.h>
#include <math/Vector2.h>
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

static bool isNear( Vector2 a, Vector2 b )
{
	return (a-b).length() < 1e-4f;
}

static int test()
{
	assert( isNear( Vector2(2,3), Vector2(0,1)+Vector2(2,2) ) );
	assert( isNear( Vector2(0,1), Vector2(2,3)-Vector2(2,2) ) );
	assert( Vector2(2,3)[1] == 3 );
	assert( isNear( Vector2(2,3).dot(Vector2(2,2)), 4+6 ) );
	assert( isNear( Vector2(1,2)*2, Vector2(2,4) ) );
	assert( isNear( Vector2(4,8).normalize(), Vector2(1,2).normalize() ) );
	assert( isNear( Vector2(4,0).normalize(), Vector2(1,0) ) );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
