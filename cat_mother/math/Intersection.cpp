#include "Intersection.h"
#include <dev/Profile.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Matrix4x4.h>
#include <math/BezierUtil.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


bool Intersection::findRayPlaneIntersection(
	const Vector3& orig, const Vector3& dir,
	const Vector4& plane, float* t )
{
	// orig is on the negative side of the plane?
	float sdist = orig.x*plane.x + orig.y*plane.y + orig.z*plane.z + plane.w;
	if ( sdist <= 0.f )
		return false;

	// ray pointing away from or along the plane?
	float d = dir.x * plane.x + dir.y * plane.y + dir.z * plane.z;
	if ( d > -FLT_MIN )
		return false;

	assert( fabsf(d) >= FLT_MIN );
	*t = -sdist / d;
	return true;
}

bool Intersection::findRaySphereIntersection(
	const Vector3& orig, const Vector3& dir,
	const Vector3& center, float radius,
	float* t )
{
	Vector3 origc = orig - center;
	float origDistSqr = origc.dot( origc );
	float radiusSqr = radius*radius;
	if ( origDistSqr <= radiusSqr )
	{
		*t = 0.f;
		return true;
	}

	float a = dir.dot(dir);
	if ( a >= FLT_MIN )
	{
		float b = 2.f * origc.dot(dir);
		float c = origDistSqr - radiusSqr;
		float d = b*b - 4.f*a*c;
		if ( d >= 0.f )
		{
			float dsqrt = sqrtf(d);
			float div = .5f / a;
			float t0 = ( -b - dsqrt )*div;
			//float t1 = ( -b + dsqrt )*div;
			if ( t0 >= 0.f )
			{
				if ( t )
					*t = t0;
				return true;
			}
		}
	}
	return false;
}

bool Intersection::findRayTriangleIntersection( 
	const Vector3& orig, const Vector3& dir,
	const Vector3& vert0, const Vector3& vert1, const Vector3& vert2,
	float* t )
{
	// find vectors for two edges sharing vert0
	Vector3 edge1 = vert1 - vert0;
	Vector3 edge2 = vert2 - vert0;

	// begin calculating determinant - also used to calculate U parameter
	Vector3 pvec = dir.cross( edge2 );

	// if determinant is near zero, ray lies in plane of triangle
	float det = edge1.dot( pvec );

	// the culling branch
	if ( fabsf(det) <= FLT_MIN )
		return false;
	float invdet = 1.f / det;

	// calculate distance from vert0 to ray orig
	Vector3 tvec = orig - vert0;

	// calculate U parameter and test bounds
	float u = tvec.dot(pvec) * invdet;
	if ( u < 0.f || u > 1.f )
		return false;

	// prepare to test V parameter
	Vector3 qvec = tvec.cross( edge1 );

	// calculate V parameter and test bounds
	float v = dir.dot(qvec) * invdet;
	if ( v < 0.f || u + v > 1.f )
		return false;

	// calculate s, ray intersects triangle
	float s = edge2.dot(qvec) * invdet;
	if ( s < 0.f )
		return false;

	*t = s;
	return true;
}

bool Intersection::findRayCubicBezierPatchIntersection( const Vector3& orig, 
	const Vector3& dir, const Vector3 patch[4][4], int subdiv,
	float* t, float* u, float* v )
{
	assert( subdiv > 0 );

	float t0 = FLT_MAX;
	float s = 0.f;
	float ds = 1.f / subdiv;
	for ( int i = 0 ; i < subdiv ; ++i )
	{
		float q = 0.f;
		float dq = 1.f / subdiv;
		for ( int j = 0 ; j < subdiv ; ++j )
		{
			Vector3 p0 = BezierUtil<Vector3>::getCubicBezierPatch( patch, q, s );
			Vector3 p1 = BezierUtil<Vector3>::getCubicBezierPatch( patch, q+dq, s );
			Vector3 p2 = BezierUtil<Vector3>::getCubicBezierPatch( patch, q+dq, s+ds );
			Vector3 p3 = BezierUtil<Vector3>::getCubicBezierPatch( patch, q, s+ds );

			float t1;
			if ( findRayTriangleIntersection(orig,dir,p0,p1,p2,&t1) )
			{
				if ( t1 < t0 )
				{
					t0 = t1;
					Vector3 ipnt = orig + dir*t1;
					*u = (ipnt-p0).dot(p1-p0) / (p1-p0).lengthSquared() * dq + q;
					*v = (ipnt-p0).dot(p2-p1) / (p2-p1).lengthSquared() * ds + s;
				}
			}
			else if ( findRayTriangleIntersection(orig,dir,p0,p2,p3,&t1) )
			{
				if ( t1 < t0 )
				{
					t0 = t1;
					Vector3 ipnt = orig + dir*t1;
					*u = (ipnt-p0).dot(p2-p3) / (p2-p3).lengthSquared() * dq + q;
					*v = (ipnt-p0).dot(p3-p0) / (p3-p0).lengthSquared() * ds + s;
				}
			}

			q += dq;
		}

		s += ds;
	}

	if ( t )
		*t = t0;

	return t0 < FLT_MAX;
}

bool Intersection::testPointBox( const Vector3& point, const Matrix4x4& box, const Vector3& dim )
{
	Vector3 d = point - box.translation();
	for ( int i = 0 ; i < 3 ; ++i )
	{
		float ad = d.dot( Vector3(box(0,i),box(1,i),box(2,i)) );
		if ( fabsf(ad) > dim[i] )
			return false;
	}
	return true;
}

bool Intersection::testPointBox( const Vector3& point, const Matrix4x4& box, const Vector3& dimMin, const Vector3& dimMax )
{
	Vector3 d = point - box.translation();
	for ( int i = 0 ; i < 3 ; ++i )
	{
		float ad = d.dot( Vector3(box(0,i),box(1,i),box(2,i)) );
		if ( ad < dimMin[i] || ad > dimMax[i] )
			return false;
	}
	return true;
}

bool Intersection::testLineBox( const Vector3& orig, const Vector3& delta, 
	const Matrix4x4& box, const Vector3& dim )
{
	if ( testPointBox(orig,box,dim) )
		return true;

	Vector3 segdir = delta*.5f;
	Vector3 segmid = orig + segdir;
	Vector3 diff = segmid - box.translation();

	float w[3], d[3];
	for ( int i = 0 ; i < 3 ; ++i )
	{
		Vector3 axis( box(0,i), box(1,i), box(2,i) );
		w[i] = fabsf( segdir.dot(axis) );
		d[i] = fabsf( diff.dot(axis) );
		float r = dim[i] + w[i];
		if ( d[i] > r )
			return false;
	}

	float wb[3] = {w[2], w[2], w[2]};
	float wc[3] = {w[1], w[0], w[0]};
	Vector3 wxd = segdir.cross( diff );
	for ( int i = 0 ; i < 3 ; ++i )
	{
		Vector3 axis( box(0,i), box(1,i), box(2,i) );
		float d = fabsf( wxd.dot(axis) );
		float r = dim[1]*wb[2] + dim[2]*wc[1];
		if ( d > r )
			return false;
	}

	return true;
}

bool Intersection::testBoxBox( const Vector3& a,
	const Matrix4x4& B, const Vector3& b )
{
	// Based on Dave Eberly's source code, www.magic-software.com

	// 15 separate axis tests:
	// 3 tests for each (unique) face direction of box A
	// 3 tests for each (unique) face direction of box B
	// 9 for pairwise combinations of edges

	const Vector3 T( B(0,3), B(1,3), B(2,3) );
	Matrix3x3 Bf;
	for ( int i = 0 ; i < 3 ; ++i )
		for ( int j = 0 ; j < 3 ; ++j )
			Bf(i,j) = fabsf( B(i,j) );

	// A1 x A2 = A0
	float t = fabsf( T[0] );
	if ( t > a[0] + b[0] * Bf(0,0) + b[1] * Bf(0,1) + b[2] * Bf(0,2) )
		return false;

	// B1 x B2 = B0
	float s = T[0]*B(0,0) + T[1]*B(1,0) + T[2]*B(2,0);
	t = fabsf(s);
	if ( t > b[0] + a[0] * Bf(0,0) + a[1] * Bf(1,0) + a[2] * Bf(2,0) )
		return false;
  
	// A2 x A0 = A1
	t = fabsf(T[1]);
	if ( t > a[1] + b[0] * Bf(1,0) + b[1] * Bf(1,1) + b[2] * Bf(1,2) )
		return false;

	// A0 x A1 = A2
	t = fabsf(T[2]);
	if ( t > a[2] + b[0] * Bf(2,0) + b[1] * Bf(2,1) + b[2] * Bf(2,2) )
		return false;

	// B2 x B0 = B1
	s = T[0]*B(0,1) + T[1]*B(1,1) + T[2]*B(2,1);
	t = fabsf(s);
	if ( t > b[1] + a[0] * Bf(0,1) + a[1] * Bf(1,1) + a[2] * Bf(2,1) )
		return false;

	// B0 x B1 = B2
	s = T[0]*B(0,2) + T[1]*B(1,2) + T[2]*B(2,2);
	t = fabsf(s);
	if ( t > b[2] + a[0] * Bf(0,2) + a[1] * Bf(1,2) + a[2] * Bf(2,2) )
		return false;

	// A0 x B0
	s = T[2] * B(1,0) - T[1] * B(2,0);
	t = fabsf(s);
	if ( t > a[1] * Bf(2,0) + a[2] * Bf(1,0) + b[1] * Bf(0,2) + b[2] * Bf(0,1) )
		return false;

	// A0 x B1
	s = T[2] * B(1,1) - T[1] * B(2,1);
	t = fabsf(s);
	if ( t > a[1] * Bf(2,1) + a[2] * Bf(1,1) + b[0] * Bf(0,2) + b[2] * Bf(0,0) )
		return false;

	// A0 x B2
	s = T[2] * B(1,2) - T[1] * B(2,2);
	t = fabsf(s);
	if ( t > a[1] * Bf(2,2) + a[2] * Bf(1,2) + b[0] * Bf(0,1) + b[1] * Bf(0,0) )
		return false;

	// A1 x B0
	s = T[0] * B(2,0) - T[2] * B(0,0);
	t = fabsf(s);
	if ( t > a[0] * Bf(2,0) + a[2] * Bf(0,0) + b[1] * Bf(1,2) + b[2] * Bf(1,1) )
		return false;

	// A1 x B1
	s = T[0] * B(2,1) - T[2] * B(0,1);
	t = fabsf(s);
	if ( t > a[0] * Bf(2,1) + a[2] * Bf(0,1) + b[0] * Bf(1,2) + b[2] * Bf(1,0) )
		return false;

	// A1 x B2
	s = T[0] * B(2,2) - T[2] * B(0,2);
	t = fabsf(s);
	if ( t > a[0] * Bf(2,2) + a[2] * Bf(0,2) + b[0] * Bf(1,1) + b[1] * Bf(1,0) )
		return false;

	// A2 x B0
	s = T[1] * B(0,0) - T[0] * B(1,0);
	t = fabsf(s);
	if ( t > a[0] * Bf(1,0) + a[1] * Bf(0,0) + b[1] * Bf(2,2) + b[2] * Bf(2,1) )
		return false;

	// A2 x B1
	s = T[1] * B(0,1) - T[0] * B(1,1);
	t = fabsf(s);
	if ( t > a[0] * Bf(1,1) + a[1] * Bf(0,1) + b[0] * Bf(2,2) + b[2] * Bf(2,0) )
		return false;

	// A2 x B2
	s = T[1] * B(0,2) - T[0] * B(1,2);
	t = fabsf(s);
	if ( t > a[0] * Bf(1,2) + a[1] * Bf(0,2) + b[0] * Bf(2,1) + b[1] * Bf(2,0) )
		return false;

	return true;
}

bool Intersection::findLineBoxIntersection( const Vector3& orig, const Vector3& delta,
	const Matrix4x4& box, const Vector3& dim, float* t, Vector3* normal )
{
	if ( testPointBox(orig,box,dim) )
	{
		if ( normal )
			*normal = Vector3(0,0,0);

		if ( t )
			*t = 0.f;

		return true;
	}

	Vector3 nx = Vector3(box(0,0),box(1,0),box(2,0));
	Vector3 ny = Vector3(box(0,1),box(1,1),box(2,1));
	Vector3 nz = Vector3(box(0,2),box(1,2),box(2,2));
	Vector3 dx = nx * dim[0];
	Vector3 dy = ny * dim[1];
	Vector3 dz = nz * dim[2];
	Vector3 c = box.translation();

	Vector3 v[8] =
	{
		c-dx+dy-dz,
		c+dx+dy-dz,
		c+dx-dy-dz,
		c-dx-dy-dz,
		c-dx+dy+dz,
		c+dx+dy+dz,
		c+dx-dy+dz,
		c-dx-dy+dz,
	};

	Vector3 faces[24] =
	{
		// front
		v[0], v[1], v[2], v[3],
		// back
		v[5], v[4], v[7], v[6], 
		// right
		v[1], v[5], v[6], v[2],
		// left
		v[4], v[0], v[3], v[7],
		// top
		v[4], v[5], v[1], v[0],
		// bottom
		v[3], v[2], v[6], v[7]
	};

	Vector3 normals[6] =
	{
		-nz,
		nz,
		nx,
		-nx,
		ny,
		-ny,
	};

	float umin = 1.f;
	for ( int i = 0 ; i < 24 ; i += 4 )
	{
		float u = 0.f;
		if ( ( findRayTriangleIntersection( orig, delta, faces[i], faces[i+1], faces[i+2], &u ) && u < umin ) ||
			( findRayTriangleIntersection( orig, delta, faces[i], faces[i+2], faces[i+3], &u ) && u < umin ) )
		{
			umin = u;
			if ( normal )
				*normal = normals[i/4];
		}
	}

	if ( t )
		*t = umin;
	return umin < 1.f;
}

bool Intersection::findLineBoxIntersection( const Vector3& orig, const Vector3& delta,
	const Matrix4x4& box, const Vector3& dimMin, const Vector3& dimMax, float* t, Vector3* normal )
{
	if ( testPointBox(orig,box,dimMin,dimMax) )
	{
		if ( normal )
			*normal = Vector3(0,0,0);

		if ( t )
			*t = 0.f;

		return true;
	}

	Vector3 nx = Vector3(box(0,0),box(1,0),box(2,0));
	Vector3 ny = Vector3(box(0,1),box(1,1),box(2,1));
	Vector3 nz = Vector3(box(0,2),box(1,2),box(2,2));
	Vector3 dxn = nx * dimMin[0];
	Vector3 dyn = ny * dimMin[1];
	Vector3 dzn = nz * dimMin[2];
	Vector3 dxp = nx * dimMax[0];
	Vector3 dyp = ny * dimMax[1];
	Vector3 dzp = nz * dimMax[2];
	Vector3 c = box.translation();

	Vector3 v[8] =
	{
		c+dxn+dyp+dzn,
		c+dxp+dyp+dzn,
		c+dxp+dyn+dzn,
		c+dxn+dyn+dzn,
		c+dxn+dyp+dzp,
		c+dxp+dyp+dzp,
		c+dxp+dyn+dzp,
		c+dxn+dyn+dzp,
	};

	Vector3 faces[24] =
	{
		// front
		v[0], v[1], v[2], v[3],
		// back
		v[5], v[4], v[7], v[6], 
		// right
		v[1], v[5], v[6], v[2],
		// left
		v[4], v[0], v[3], v[7],
		// top
		v[4], v[5], v[1], v[0],
		// bottom
		v[3], v[2], v[6], v[7]
	};

	Vector3 normals[6] =
	{
		-nz,
		nz,
		nx,
		-nx,
		ny,
		-ny,
	};

	float umin = 1.f;
	for ( int i = 0 ; i < 24 ; i += 4 )
	{
		float u = 0.f;
		if ( ( findRayTriangleIntersection( orig, delta, faces[i], faces[i+1], faces[i+2], &u ) && u < umin ) ||
			( findRayTriangleIntersection( orig, delta, faces[i], faces[i+2], faces[i+3], &u ) && u < umin ) )
		{
			umin = u;
			if ( normal )
				*normal = normals[i/4];
		}
	}

	if ( t )
		*t = umin;
	return umin < 1.f;
}

bool Intersection::findRayCylinderIntersection(
	const Vector3& orig, const Vector3& dir,
	const Vector3& center, const Vector3& axis, float radius, float halfHeight,
	float* t, Vector3* normal )
{
	assert( orig.finite() );
	assert( dir.finite() );
	assert( axis.finite() && fabs(axis.length()-1.f) < 1e-3f );

	// solution type
	enum SolType
	{
		SOL_NONE,
		SOL_INSIDE,
		SOL_CAP,
		SOL_WALL,
	};

	// setup
	const float eps = 1e-10f;
	float radiusSqr = radius * radius;
	float sol[2] = {-1.f,-1.f}; // solutions relative to world space dir length
	SolType solType[2] = {SOL_NONE,SOL_NONE};
	int solCount = 0;

	// generate cylinder coordinate space
	Matrix3x3 cylRot;
	cylRot.generateOrthonormalBasis( axis );
	//Matrix4x4 cylTmInv; cylTmInv.setInverseOrthonormalTransform( cylRot, center );
	Matrix4x4 cylTm( 1.f ); cylTm.setRotation( cylRot ); cylTm.setTranslation( center ); Matrix4x4 cylTmInv = cylTm.inverse();

	// transform ray to cylinder space
	Vector3 origC = cylTmInv.transform( orig );
	Vector3 dirC = cylTmInv.rotate( dir );

	// start point inside cylinder?
	bool origInInfCyl = origC.x*origC.x + origC.y*origC.y <= radiusSqr;
	bool origBetweenCaps = fabsf(origC.z) <= halfHeight;
	if ( origBetweenCaps && origInInfCyl )
	{
		// case 1: startpoint inside
		solType[solCount] = SOL_INSIDE;
		sol[solCount++] = 0.f;
	}
	else
	{
		// valid direction?
		float dirLen = dirC.length();
		if ( dirLen >= eps )
		{
			float dirLenInv = 1.f / dirLen;
			dirC *= dirLenInv;

			// direction parallel to cylinder?
			if ( fabsf(dirC.z) >= 1.f-eps )
			{
				if ( origInInfCyl )
				{
					// case 2: intersects cap(s), direction parallel
					float tmp = dirLenInv / dirC.z;
					solType[solCount] = SOL_CAP;
					sol[solCount++] = (halfHeight - origC.z) * tmp;
					solType[solCount] = SOL_CAP;
					sol[solCount++] = (-halfHeight - origC.z) * tmp;
				}
			}
			else
			{
				// direction perpendicular to cylinder?
				if ( fabsf(dirC.z) <= eps )
				{
					// inside caps?
					if ( origBetweenCaps )
					{
						// intersects origin centered circle?
						float a = dirC.x*dirC.x + dirC.y*dirC.y;
						float b = 2.f * (origC.x*dirC.x + origC.y*dirC.y);
						float c = origC.x*origC.x + origC.y*origC.y - radiusSqr;
						float discr = b*b - 4.f*a*c;
						if ( discr > FLT_MIN )
						{
							// case 3: intersects wall(s), direction perpendicular
							float root = sqrtf( discr );
							float tmp = dirLenInv / (2.f*a);
							solType[solCount] = SOL_WALL;
							sol[solCount++] = (-b + root) * tmp;
							solType[solCount] = SOL_WALL;
							sol[solCount++] = (-b - root) * tmp;
						}
					}
				}
				else
				{
					// test cap plane intersections
					assert( 0 == solCount );
					float inv = 1.f / dirC.z;
					float u0 = (+halfHeight - origC.z)*inv;
					float tmp0 = origC.x + u0*dirC.x;
					float tmp1 = origC.y + u0*dirC.y;
					if ( tmp0*tmp0 + tmp1*tmp1 <= radiusSqr )
					{
						// case 4: intersects cap(s)
						solType[solCount] = SOL_CAP;
						sol[solCount++] = u0*dirLenInv;
					}

					float u1 = (-halfHeight - origC.z)*inv;
					tmp0 = origC.x + u1*dirC.x;
					tmp1 = origC.y + u1*dirC.y;
					if ( tmp0*tmp0 + tmp1*tmp1 <= radiusSqr )
					{
						// case 4: intersects cap(s)
						solType[solCount] = SOL_CAP;
						sol[solCount++] = u1*dirLenInv;
					}

					// ray does not intersects both end caps?
					if ( solCount < 2 )
					{
						// intersects origin centered circle?
						float a = dirC.x*dirC.x + dirC.y*dirC.y;
						float b = origC.x*dirC.x + origC.y*dirC.y;
						float c = origC.x*origC.x + origC.y*origC.y - radiusSqr;
						float discr = b*b - a*c;
						if ( discr > FLT_MIN )
						{
							// case 5: intersects wall(s)
							float root = sqrtf( discr );
							float tmp = 1.f / a;
							float u = (-b - root) * tmp;
							if ( u0 <= u1 )
							{
								if ( u0 <= u && u <= u1 )
								{
									solType[solCount] = SOL_WALL;
									sol[solCount++] = u * dirLenInv;
								}
							}
							else
							{
								if ( u1 <= u && u <= u0 )
								{
									solType[solCount] = SOL_WALL;
									sol[solCount++] = u * dirLenInv;
								}
							}

							// max 2 intersections, solve second only if it is possible
							if ( solCount < 2 )
							{
								float u = (-b + root) * tmp;
								if ( u0 <= u1 )
								{
									if ( u0 <= u && u <= u1 )
									{
										solType[solCount] = SOL_WALL;
										sol[solCount++] = u * dirLenInv;
									}
								}
								else
								{
									if ( u1 <= u && u <= u0 )
									{
										solType[solCount] = SOL_WALL;
										sol[solCount++] = u * dirLenInv;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// store closest intersection
	float u = FLT_MAX; // relative to world space dir
	SolType intersectionType = SOL_NONE;
	for ( int i = 0 ; i < solCount ; ++i )
	{
		if ( sol[i] >= 0.f )
		{
			if ( sol[i] < u )
			{
				u = sol[i];
				intersectionType = solType[i];
			}
		}
	}

	// compute normal at intersection point
	if ( normal && intersectionType != SOL_NONE )
	{
		Vector3 ipointC = origC + cylTmInv.rotate( dir*u );
		SolType itype = intersectionType;

		// inside -> wall or cap normal, which one is closer
		if ( itype == SOL_INSIDE )
		{
			float capDist = fabsf( fabsf(ipointC.z) - halfHeight );
			float cylDist = fabsf( sqrtf(ipointC.x*ipointC.x + ipointC.y*ipointC.y) - radius );
			if ( capDist < cylDist )
				itype = SOL_CAP;
			else
				itype = SOL_WALL;
		}
		
		if ( itype == SOL_CAP )
		{
			if ( ipointC.z < 0.f )
				*normal = -axis;
			else
				*normal = axis;
		}
		else if ( itype == SOL_WALL )
		{
			Vector3 inormal = Vector3(ipointC.x,ipointC.y,0);
			float inormalLen = inormal.length();
			if ( inormalLen < FLT_MIN )
				*normal = axis;
			else
				*normal = ( cylRot * inormal ) * (1.f/inormalLen);
		}
	}

	// store ray length to closest intersection
	if ( t )
		*t = u;
	return intersectionType != SOL_NONE;
}

bool Intersection::findLineCylinderIntersection(
	const Vector3& orig, const Vector3& delta,
	const Vector3& center, const Vector3& axis, float radius, float halfHeight,
	float* t, Vector3* normal )
{
	float u = 1.f;
	bool intersects = findRayCylinderIntersection( orig, delta, center, axis, radius, halfHeight, &u, normal );
	intersects = ( intersects && u < 1.f );
	if ( intersects && t )
		*t = u;
	return intersects;
}



bool Intersection::findLinePlaneIntersection(
	const Vector3& orig, const Vector3& delta,
	const Vector4& plane, float* t )
{
	float u = 1.f;
	bool intersects = findRayPlaneIntersection( orig, delta, plane, &u );
	intersects = ( intersects && u < 1.f );
	if ( intersects && t )
		*t = u;
	return intersects;
}

bool Intersection::findLineTriangleIntersection(
	const Vector3& orig, const Vector3& delta,
	const Vector3& vert0, const Vector3& vert1, const Vector3& vert2,
	float* t )
{
	float u = 1.f;
	bool intersects = findRayTriangleIntersection( orig, delta, vert0, vert1, vert2, &u );
	intersects = ( intersects && u < 1.f );
	if ( intersects && t )
		*t = u;
	return intersects;
}

bool Intersection::findLineSphereIntersection(
	const Vector3& orig, const Vector3& delta,
	const Vector3& center, float radius,
	float* t )
{
	float u = 1.f;
	bool intersects = findRaySphereIntersection( orig, delta, center, radius, &u );
	intersects = ( intersects && u < 1.f );
	if ( intersects && t )
		*t = u;
	return intersects;
}

bool Intersection::testVerticalCylinderCylinder( 
		const Vector3& bot1, float height1, float radius1, 
		const Vector3& bot2, float height2, float radius2 )
{
	Vector3 bottom1 = bot1;
	Vector3 bottom2 = bot2;

	// test if heights overlap
	if ( ( bottom1.y > (bottom2.y + height2) ) || ( (bottom1.y + height1) < bottom2.y ) )
		return false;

	// remove Y axis and test if limiting circles overlap
	bottom1.y = 0;
	bottom2.y = 0;
	if ( (bottom1 - bottom2).length() > (radius1 + radius2) )
		return false;

	return true;
}

} // math
