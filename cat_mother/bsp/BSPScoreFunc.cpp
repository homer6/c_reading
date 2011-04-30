#include "BSPScoreFunc.h"
#include "BSPPolygon.h"
#include <util/Vector.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


int BSPScoreFunc::getPlaneScore( const Vector4& plane, 
	const BSPPolygon* begin, const BSPPolygon* end ) const
{
	const float planeThickness = 1e-2f;

	int pos = 0;
	int neg = 0;

	for ( const BSPPolygon* poly = begin ; poly != end ; ++poly )
	{
		int vpos = 0;
		int vneg = 0;

		const int verts = poly->vertices();
		for ( int k = 0 ; k < verts ; ++k )
		{
			const Vector3& v = poly->getVertex(k);
			float sdist = v.x*plane.x + v.y*plane.y + v.z*plane.z + plane.w;

			if ( sdist >= planeThickness )
				++vpos;
			else if ( sdist <= -planeThickness )
				++vneg;
		}

		if ( verts == vpos )
			++pos;
		else if ( verts == vneg )
			++neg;
	}

	if ( pos < neg )
		return pos;
	else
		return neg;
}


} // bsp
