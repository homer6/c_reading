#include <tester/Test.h>
#include <math/Matrix3x3.h>
#include <math/Matrix4x4.h>
#include <math.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

static int test()
{
	// point-plane projection test
	{
		Vector3 pp( 0,0,0 );
		Vector3 pn( 0,1,0 );
		Vector3 p = Vector3(3,7,4);
		Vector3 l = p + Vector3(0,100,0);
		Matrix4x4 proj;
		proj.setPointPlaneProjection( l, pn, pp );
		
		Vector4 v = Vector4(p.x,p.y,p.z,1.f);
		Vector4 p1 = proj * v;
		p1 *= 1.f / p1.w;
		assert( (Vector3(p1.x,p1.y,p1.z)-Vector3(p.x,0,p.z)).length() < 1e-3f );
	}

	// direct-plane projection test
	{
		Vector3 v( 3,7,4 );
		Vector3 l = Vector3( -1,-1,0 ).normalize();
		Vector3 p( 0,0,0 );
		Vector3 n( 0,1,0 );

		float k = -l.dot(n);
		Vector3 v1 = l * ( (v-p).dot(n)/k ) + v;

		Matrix4x4 m;
		m.setDirectPlaneProjection( l, n, p );

		Vector4 vp = m * Vector4(v.x,v.y,v.z,1);
		vp *= 1.f / vp.w;
		assert( (Vector3(vp.x,vp.y,vp.z)-v1).length() < 1e-3f );
	}

	// orthonormal basis
	{
		Vector3 axis	= Vector3(0.214791f, 0.973763f, -0.0751746f).normalize();
		Matrix3x3 rot;
		rot.generateOrthonormalBasis( axis );
		assert( fabsf(rot.getColumn(0).dot(rot.getColumn(1))) < 1e-4f );
		assert( fabsf(rot.getColumn(2).dot(rot.getColumn(1))) < 1e-4f );
		assert( fabsf(rot.getColumn(0).dot(rot.getColumn(2))) < 1e-4f );
		rot = rot;
	}
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
