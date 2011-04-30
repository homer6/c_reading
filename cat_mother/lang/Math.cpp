#include "Math.h"
#include "Mutex.h"
#include <math.h>
#include <stdlib.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


/** Spin lock to guard global random(). */
static long			s_spin			= 0;
static const float	s_randScale		= float(1) / ( float(RAND_MAX) + float(1) );

//-----------------------------------------------------------------------------

const float		Math::E			= 2.718281828459045f;
const float		Math::PI		= 3.141592653589793f;

//-----------------------------------------------------------------------------

float Math::random()
{
	while ( Mutex::testAndSet(&s_spin) );
	float v = rand() * s_randScale;
	Mutex::testAndSet(&s_spin,0);
	return v;
}


} // lang
