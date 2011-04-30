#include "BoundVolume.h"
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


bool BoundVolume::testSphereVolume( const Vector3& pos, float r,
	const Vector4* planes, int planeCount )
{
	assert( planeCount > 0 );

	for ( int i = 0 ; i < planeCount ; ++i )
	{
		Vector4 p( pos.x - planes[i].x * r,
			pos.y - planes[i].y * r,
			pos.z - planes[i].z * r, 1.f );

		if ( p.dot(planes[i]) > 0.f )
			return false;
	}
	return true;
}

bool BoundVolume::testPointsVolume( 
	const Vector3* points, int pointCount,
	const Vector4* planes, int planeCount )
{
	assert( planeCount > 0 );

	for ( int i = 0 ; i < planeCount ; ++i )
	{
		const Vector4& plane = planes[i];
		int out = 0;

		const Vector3* end = points + pointCount;
		for ( const Vector3* point = points ; point != end ; ++point )
		{
			Vector4 p( point->x, point->y, point->z, 1.f );
			if ( p.dot(plane) > 0.f )
				++out;
			else
				break;
		}

		if ( out == pointCount )
			return false;
	}
	return true;
}


} // sg
