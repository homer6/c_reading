#include "Vector3.h"
#include <assert.h>
#include "config.h"

//----------------------------------------------------------------------------

namespace math
{


Vector3 Vector3::rotate( const Vector3& axis, float angle ) const
{
	assert( finite() );
	assert( axis.length() > Vector3::valueMin() ); // ensure that Axis isn't null vector

	const Vector3			vector		= *this;
	Vector3					unitAxis	= axis.normalize();
	const float				halfAngle	= angle/float(2);
	const float				s			= VectorN::valueSin(halfAngle);
	const float				c			= VectorN::valueCos(halfAngle);
	const float				x			= unitAxis.x * s;
	const float				y			= unitAxis.y * s;
	const float				z			= unitAxis.z * s;
	const float				w			= c;
	const float				xx			= x*x;
	const float				xy 			= y*x;
	const float				xz			= z*x;
	const float				yy			= y*y;
	const float				yz			= z*y;
	const float				zz			= z*z;
	const float				wx			= w*x;
	const float				wy			= w*y;
	const float				wz			= w*z;
						
	const float				M[3][3]		=
	{
		{float(1)-float(2)*(yy+zz),		float(2)*(xy-wz),				float(2)*(xz+wy)},
		{float(2)*(xy+wz),				float(1)-float(2)*(xx+zz),      float(2)*(yz-wx)},
		{float(2)*(xz-wy),				float(2)*(yz+wx),				float(1)-float(2)*(xx+yy)},
	};

	return Vector3( 
		vector.x*M[0][0] + vector.y*M[0][1] + vector.z*M[0][2],
		vector.x*M[1][0] + vector.y*M[1][1] + vector.z*M[1][2],
		vector.x*M[2][0] + vector.y*M[2][1] + vector.z*M[2][2] );
}


} // math
