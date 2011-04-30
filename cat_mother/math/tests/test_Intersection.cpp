#include <tester/Test.h>
#include <dev/TimeStamp.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Matrix4x4.h>
#include <math/Intersection.h>
#include <math/BezierUtilVector3.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <stdio.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace dev;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

static Vector4 makePlane( const Vector3& p0, const Vector3& p1, const Vector3& p2 )
{
	Vector3 n = (p1-p0).cross( p2-p0 );
	n = n.normalize();
	Vector4 plane;
	plane.x = n.x;
	plane.y = n.y;
	plane.z = n.z;
	plane.w = -p0.dot(n);
	return plane;
}

// Compare Intersection::findRayCylinderIntersection results against expected results.
static void testRayCyl( const Vector3& orig, const Vector3& dir, 
	bool expectedResult, float expectedT, const Vector3 expectedNormal,
	const Vector3& cylCenter, const Vector3& cylAxis, float cylRadius, float cylHalfHeight )
{
	float t = -1.f;
	Vector3 normal(0,0,0);
	bool intersects = Intersection::findRayCylinderIntersection( orig, dir, cylCenter, cylAxis, cylRadius, cylHalfHeight, &t, &normal );
	assert( intersects == expectedResult );
	assert( !intersects || Math::abs(expectedT-t) < 1.f && (normal-expectedNormal).length() < 1e-3f );
}

static int test()
{
	// ray-cylinder
	{
		// cylinder
		Vector3 cylTopLeft( 66,61,0 );
		Vector3 cylTopRight( 152,61,0 );
		Vector3 cylBottomRight( 152,99,0 );
		Vector3 cylBottomLeft( 66,99,0 );
		Vector3 cylCenter = (cylTopLeft+cylBottomRight)*.5f;
		Vector3 cylAxis = (cylTopRight - cylTopLeft).normalize();
		float cylHalfHeight = (cylTopRight - cylTopLeft).length() * .5f;
		float cylRadius = (cylBottomLeft - cylTopLeft).length() * .5f;

		// test case 1: startpoint inside
		testRayCyl( Vector3(76,64,0), Vector3(1,1,0), true, 0.f, Vector3(0,-1,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(69,89,0), Vector3(-1,1,0), true, 0.f, Vector3(-1,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(98,57,0), Vector3(1,-1,1), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );

		// test case 2: direction parallel
		testRayCyl( Vector3(21,67,0), Vector3(1,0,0), true, 45.f, Vector3(-1,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(158,77,0), Vector3(1,0,0), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(158,77,0), Vector3(-1,0,0), true, 6.f, Vector3(1,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(21,67,cylRadius-1.f), Vector3(1,0,0), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( cylCenter-cylAxis*100.f+Vector3(0,cylRadius-Float::MIN_VALUE,0), cylAxis, true, 57.f, Vector3(-1,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );

		// test case 3: direction perpendicular
		testRayCyl( Vector3(79,38,0), Vector3(0,1,0), true, 23.f, Vector3(0,-1,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(136,138,0), Vector3(0,-1,0), true, 39.f, Vector3(0,1,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(136,88,cylRadius), Vector3(0,-1,0), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(157,107,0), Vector3(0,-1,0), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		
		// test case 4: cap intersections
		testRayCyl( Vector3(55,67,0), Vector3(432,8,0).normalize(), true, 11.f, Vector3(-1,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(55,67,0), Vector3(179,308,0).normalize(), true, 21.8f, Vector3(-1,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(156,95,0), Vector3(-130,-13,0).normalize(), true, 4.f, Vector3(1,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(156,95,0), Vector3(-53,119,0).normalize(), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );

		// test case 5: wall intersections
		testRayCyl( Vector3(68,39,0), Vector3(23,23,0).normalize(), true, 31.f, Vector3(0,-1,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(161,140,0), Vector3(-33,-111,0).normalize(), true, 43.f, Vector3(0,1,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(45,166,0), Vector3(155,-94,0).normalize(), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );
		testRayCyl( Vector3(71,115,0), Vector3(127,-22,0).normalize(), false, 0.f, Vector3(0,0,0), cylCenter, cylAxis, cylRadius, cylHalfHeight );

		// bug case 1: wrong i-normal
		{
			Vector3 start = Vector3(3.98909f, 0.619994f, 2.28692f);
			Vector3 delta = Vector3(0, -0.03f, 0);
			Vector3 cylCenter = Vector3(4.05182f, 4.88758e-006f, 2.89225f);
			Vector3 cylAxis = Vector3(0,0,1);
			float r = 0.6f;
			float cylLen = 3.26180f;
			float t;
			Vector3 inormal;
			bool intersects = Intersection::findLineCylinderIntersection( start, delta, cylCenter, cylAxis, r, cylLen*.5f, &t, &inormal );
			Vector3 ipoint = start + delta*t;
			assert( inormal.y > 0.f );
		}

		// bug case 2: wrong i-normal
		{
			Vector3 start = Vector3(3.4318f, 0.609998f, 2.09573f);
			Vector3 delta = Vector3(0.0718586f, -3.81469e-008f, 0.0351617f);
			Vector3 cylCenter = Vector3(4.05182f, 1.90463f, 0.187856f);
			Vector3 cylAxis = Vector3(0, 0.402225f, -0.915541f);
			float r = 0.6f;
			float cylLen = 2.34505f;
			float t;
			Vector3 inormal;
			float dist1 = (start-(cylCenter+cylAxis*cylLen*0.5f)).length();
			float dist2 = (start-(cylCenter-cylAxis*cylLen*0.5f)).length();
			bool intersects = Intersection::findLineCylinderIntersection( start, delta, cylCenter, cylAxis, r, cylLen*.5f, &t, &inormal );
			assert( !intersects );
		}
	}

	// ray-plane
	{
		Vector4 plane = makePlane( Vector3(1,2.5f,0.f), Vector3(3,5,0), Vector3(1,2.5f,1.f) );
		Vector3 start = Vector3(2.5f,0,0);
		Vector3 end = Vector3(2,2,0);
		Vector3 dir = (end-start).normalize();
		float t = 0.f;
		bool ok = Intersection::findRayPlaneIntersection( start, dir, plane, &t );
		Vector3 ip = start + dir * t;
		float err = (ip-Vector3(1.666f,3.3333f,0)).length();
		assert( err < 0.1f );
		assert( ok );
		end = start - Vector3(0,1,0);
		dir = (end-start).normalize();
		t = 0.f;
		ok = !Intersection::findRayPlaneIntersection( start, dir, plane, &t );
		assert( ok );
	}
	
	// ray-sphere
	{
		Vector3 center( 3,2,0 );
		float r = 2.f;
		Vector3 orig( 7,-1,0 );
		Vector3 end( 3,5,0 );
		Vector3 dir = end-orig;
		float t;
		bool inters = Intersection::findRaySphereIntersection( orig, dir, center, r, &t );
		assert( inters );
		Vector3 intersp = orig + dir*t;
		assert( (intersp-Vector3(5,2,0)).length() < 1e-3f );
		end += Vector3(0,2,0);
		dir = end-orig;
		inters = Intersection::findRaySphereIntersection( orig, dir, center, r, &t );
		assert( !inters );
	}

	// line-box
	{
		Matrix4x4 box(1);
		Vector3 dim( Math::sqrt(2.f)/2.f, Math::sqrt(2.f), 1.f );
		box.setTranslation( Vector3(3.5f,4.5f,0) );
		box.setRotation( Matrix3x3(Vector3(0,0,1),-Math::toRadians(45.f)) );
		Vector3 origs[] = {Vector3(4,2,0),Vector3(4,2,0),Vector3(1,2,0),Vector3(4.5f,6,0)};
		Vector3 ends[] = {Vector3(6,6,0),Vector3(2,6,0),Vector3(2,3,0),Vector3(4.5f,7,0)};
		bool res[] = {false,true,false,false};
		const int count = sizeof(origs)/sizeof(origs[0]);
		Vector3 deltas[count];
		for ( int i = 0 ; i < count ; ++i )
		{
			Vector3 d = deltas[i] = ends[i] - origs[i];
			Vector3 o = origs[i];
			bool inters = Intersection::testLineBox(o,d,box,dim);
			assert( inters == res[i] );

			float t;
			Vector3 n;
			bool inters2 = Intersection::findLineBoxIntersection(o,d,box,dim,&t,&n);
			assert( inters2 == inters );
		}
	}

	// ray-patch
	{
		Vector3 patch[4][4] =
		{
			{Vector3(50,50,0), Vector3(150,50,0), Vector3(250,50,0), Vector3(350,50,0)},
			{Vector3(50,150,0), Vector3(150,150,100), Vector3(250,150,255), Vector3(350,150,0)},
			{Vector3(50,250,0), Vector3(150,250,50), Vector3(250,250,200), Vector3(350,250,0)},
			{Vector3(50,350,0), Vector3(150,350,0), Vector3(250,350,0), Vector3(350,350,0)},
		};

		float t,u,v;
		bool inters = false;
		Vector3 intersp, patchp;
		int subdiv = 10;
		float maxdif = 0.f;
		int intersc = 0;

		for ( int i = 0 ; i < 100 ; ++i )
		{
			Vector3 orig( 350+(Math::random()-.5f)*500, 50+(Math::random()-.5f)*500, 256+(Math::random()-.5f)*500 );
			Vector3 end( 50+(Math::random()-.5f)*500, 350+(Math::random()-.5f)*500, -256+(Math::random()-.5f)*500 );
			Vector3 dir = (end - orig).normalize();
			inters = Intersection::findRayCubicBezierPatchIntersection( orig, dir, patch, subdiv, &t, &u, &v );
			if ( inters )
			{
				++intersc;
				intersp = orig + dir*t;
				patchp = BezierUtilVector3::getCubicBezierPatch( patch, u, v );
				float dif = (intersp-patchp).length();
				if ( dif > maxdif )
					maxdif = dif;
				
				Intersection::findRayCubicBezierPatchIntersection( orig, dir, patch, subdiv, &t, &u, &v );
			}
		}

		printf( "intersc=%i, maxdif=%g\n", intersc, maxdif );
	}
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
