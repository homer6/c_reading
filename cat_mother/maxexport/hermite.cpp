#include "StdAfx.h"
#include "hermite.h"

//-----------------------------------------------------------------------------

void hermite( float t, 
	float* h1,
	float* h2,
	float* h3,
	float* h4 )
{
	float t2, t3, z;

	t2 = t * t;
	t3 = t * t2;
	z = 3.f * t2 - t3 - t3;

	*h1 = 1.f - z;
	*h2 = z;
	*h3 = t3 - t2 - t2 + t;
	*h4 = t3 - t2;
}
