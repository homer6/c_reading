#include <tester/Test.h>
#include <math/Vector3.h>
#include <math/BezierUtil.h>
#include <math.h>
#include <assert.h>
#include <math/internal/config.h>

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

static int test()
{
	/*
	  (rules based on integration of Lagrange interpolating polynomials)

	  xi = x0 + i*h
	  f(xi) == fi

	  Trapezoidal rule:
	  I_x1_x2 f(x)dx = h/2 * [f1 + f2]

	  Simpson's rule:
	  I_x1_x3 f(x)dx = h/3 * [f1 + 4*f2 + f3]

	  Simpson's 3/8 rule:
	  I_x1_x4 f(x)dx = h/8 * [3*f1 + 9*f2 + 9*f3 + 3*f4]

	  Bode's rule:
	  I_x1_x5 f(x)dx = h/45 * [14*f1 + 64*f2 + 24*f3 + 64*f4 + 14*f5]

	  6-point rule:
	  I_x1_x6 f(x)dx = h * (5/288) * [19*f1 + 75*f2 + 50*f3 + 50*f4 + 75*f5 + 19*f6]
	*/
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
