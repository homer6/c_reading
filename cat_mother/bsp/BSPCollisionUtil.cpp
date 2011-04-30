#include "BSPCollisionUtil.h"
#include "BSPNode.h"
#include "BSPPolygon.h"
#include <dev/Profile.h>
#include <lang/Float.h>
#include <lang/Math.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Intersection.h>
#include <assert.h>
#include <functional>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


/** Internal collision info. */
class BSPCollisionInfo
{
public:
	const BSPPolygon*	cpoly;
	Vector3				cnormal;
	Vector3				cpoint;

	BSPCollisionInfo() :
		cpoly(0), cnormal(0,0,0), cpoint(0,0,0) 
	{
	}
};

//-----------------------------------------------------------------------------

static BSPCollisionUtil::Statistics s_statistics;

//-----------------------------------------------------------------------------

/** 
 * Finds the first intersection of a line segment against node polygons.
 * @param node Node of the BSP tree.
 * @param start Start of the line segment.
 * @param end End of the line segment.
 * @param delta Vector from start to end of the line segment.
 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
 * @param t [in/out] Receives relative length to intersection IF there is an intersection closer than initial value. (see cmp parameter)
 * @param cpoly [out] Receives pointer to the collision polygon.
 * @param cmp Binary function to compare if current intersection is closer than initial collision.
 */
template <class Cmp> static void findLineIntersectionPolygon( BSPNode* node, 
	const Vector3& start, const Vector3& end, 
	const Vector3& delta, int collisionMask,
	float* t, const BSPPolygon** cpoly, Cmp cmp )
{
	assert( t );
	assert( cpoly );

	if ( node )
	{
		for ( int i = 0 ; i < node->polygons() ; ++i )
		{
			const BSPPolygon* poly = &node->getPolygon(i);
			if ( poly->collisionMask() & collisionMask )
			{
				const Vector4&	plane		= poly->plane();
				float			startDist	= plane.x*start.x + plane.y*start.y + plane.z*start.z + plane.w;
				float			endDist		= plane.x*end.x + plane.y*end.y + plane.z*end.z + plane.w;

				if ( startDist > -BSPNode::PLANE_THICKNESS )
				{
					if ( endDist < BSPNode::PLANE_THICKNESS )
					{
						// +-
						float deltaDist = startDist - endDist;
						if ( deltaDist > Float::MIN_VALUE )
						{
							float u = startDist / deltaDist;
							if ( cmp(u,*t) )
							{
								Vector3 planePoint = start + delta * u;
								if ( poly->isPointInPolygon(planePoint) )
								{
									*t = u;
									*cpoly = poly;
								}
							}
						}
						else
						{
							// line goes along the plane
							// (ignore the case for now)
						}
					}
				}

				++s_statistics.linePolygonTests;
			}
		}
	}
}

/** 
 * Finds the first intersection of the line segment against BSP node tree. 
 * @param root Root node of the BSP tree.
 * @param start Start of the line segment.
 * @param end End of the line segment.
 * @param delta Vector from start to end of the line segment.
 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
 * @param t [in/out] Receives relative length to intersection IF there is an intersection closer than initial value.
 * @param cpoly [out] Receives pointer to the collision polygon.
 */
static void findLineIntersectionRecurse( BSPNode* root, 
	const Vector3& start, const Vector3& end, 
	const Vector3& delta, int collisionMask, float* t, const BSPPolygon** cpoly )
{
	assert( t );
	assert( cpoly );

	if ( !root )
		return;

	const Vector4&		plane		= root->plane();
	float				startDist	= plane.x*start.x + plane.y*start.y + plane.z*start.z + plane.w;
	float				endDist		= plane.x*end.x + plane.y*end.y + plane.z*end.z + plane.w;
	std::less<float>	cmp;

	if ( startDist > 0.f )
	{
		if ( endDist > 0.f )
		{
			// ++
			findLineIntersectionRecurse( root->positive(), start, end, delta, collisionMask, t, cpoly );
			if ( startDist <= BSPNode::PLANE_THICKNESS || endDist <= BSPNode::PLANE_THICKNESS ||
				root->leaf() )
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
		}
		else
		{
			// +-
			float t0 = *t;
			findLineIntersectionRecurse( root->positive(), start, end, delta, collisionMask, t, cpoly );
			if ( *t == t0 )
			{
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
				findLineIntersectionRecurse( root->negative(), start, end, delta, collisionMask, t, cpoly );
			}
		}
	}
	else
	{
		if ( endDist > 0.f )
		{
			// -+
			float t0 = *t;
			findLineIntersectionRecurse( root->negative(), start, end, delta, collisionMask, t, cpoly );
			if ( *t == t0 )
			{
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
				findLineIntersectionRecurse( root->positive(), start, end, delta, collisionMask, t, cpoly );
			}
		}
		else
		{
			// --
			findLineIntersectionRecurse( root->negative(), start, end, delta, collisionMask, t, cpoly );
			if ( -startDist <= BSPNode::PLANE_THICKNESS || -endDist <= BSPNode::PLANE_THICKNESS ||
				root->leaf() )
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
		}
	}
}

/** 
 * Finds the last intersection of the line segment against BSP node tree. 
 * @param root Root node of the BSP tree.
 * @param start Start of the line segment.
 * @param end End of the line segment.
 * @param delta Vector from start to end of the line segment.
 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
 * @param t [in/out] Receives relative length to intersection IF there is an intersection closer than initial value.
 * @param cpoly [out] Receives pointer to the collision polygon.
 */
static void findLastLineIntersectionRecurse( BSPNode* root, 
	const Vector3& start, const Vector3& end, 
	const Vector3& delta, int collisionMask, float* t, const BSPPolygon** cpoly )
{
	assert( t );
	assert( cpoly );

	if ( !root )
		return;

	const Vector4&		plane		= root->plane();
	float				startDist	= plane.x*start.x + plane.y*start.y + plane.z*start.z + plane.w;
	float				endDist		= plane.x*end.x + plane.y*end.y + plane.z*end.z + plane.w;
	std::greater<float>	cmp;

	if ( startDist > 0.f )
	{
		if ( endDist > 0.f )
		{
			// ++
			findLastLineIntersectionRecurse( root->positive(), start, end, delta, collisionMask, t, cpoly );
			if ( startDist <= BSPNode::PLANE_THICKNESS || endDist <= BSPNode::PLANE_THICKNESS ||
				root->leaf() )
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
		}
		else
		{
			// +-
			float t0 = *t;
			findLastLineIntersectionRecurse( root->negative(), start, end, delta, collisionMask, t, cpoly );
			if ( *t == t0 )
			{
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
				findLastLineIntersectionRecurse( root->positive(), start, end, delta, collisionMask, t, cpoly );
			}
		}
	}
	else
	{
		if ( endDist > 0.f )
		{
			// -+
			float t0 = *t;
			findLastLineIntersectionRecurse( root->positive(), start, end, delta, collisionMask, t, cpoly );
			if ( *t == t0 )
			{
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
				findLastLineIntersectionRecurse( root->negative(), start, end, delta, collisionMask, t, cpoly );
			}
		}
		else
		{
			// --
			findLastLineIntersectionRecurse( root->negative(), start, end, delta, collisionMask, t, cpoly );
			if ( -startDist <= BSPNode::PLANE_THICKNESS || -endDist <= BSPNode::PLANE_THICKNESS ||
				root->leaf() )
				findLineIntersectionPolygon( root, start, end, delta, collisionMask, t, cpoly, cmp );
		}
	}
}

/** Sets information about moving sphere vs. BSP collision. */
inline static void setMovingSphereCollisionInfo( const Vector3& start, const Vector3& /*delta*/,
	float u, const BSPPolygon* cpoly, 
	const Vector3& cnormal, const Vector3& cpoint, BSPCollisionInfo* cinfo, float* t )
{
	if ( u < *t )
	{
		*t = u;
		cinfo->cpoly = cpoly;
		cinfo->cnormal = cnormal;
		cinfo->cpoint = cpoint;
	}
	else if ( 0.f == u ) // both already inside...
	{
		assert( cinfo->cpoly );
		assert( *t == 0.f );

		float collisionDepthSqr = (cpoint - start).lengthSquared();
		float oldCollisionDepthSqr = (cinfo->cpoint - start).lengthSquared();
		if ( collisionDepthSqr < oldCollisionDepthSqr )
		{
			*t = u;
			cinfo->cpoly = cpoly;
			cinfo->cnormal = cnormal;
			cinfo->cpoint = cpoint;
		}
	}
}

/** 
 * Finds the first intersection of a moving sphere against node polygons.
 * @param node Node of the BSP tree.
 * @param start Start of the line segment.
 * @param end End of the line segment.
 * @param delta Vector from start to end of the line segment.
 * @param r Radius of the sphere.
 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
 * @param t [in/out] Receives relative length to intersection IF there is an intersection closer than initial value. (see cmp parameter)
 * @param cinfo [out] Collision check results.
 */
static void findMovingSphereIntersectionPolygon( BSPNode* node, const Vector3& start, 
	const Vector3& end, const Vector3& delta, float r, int collisionMask, float* t,
	BSPCollisionInfo* cinfo )
{
	assert( t );
	assert( cinfo );

	if ( node )
	{
		Vector3 moveCenter = start + delta * 0.5f;
		float moveRadius = (end-start).length() + r*2.f;
		float moveRadiusSqr = moveRadius * moveRadius;
		for ( int i = 0 ; i < node->polygons() ; ++i )
		{
			const BSPPolygon* poly = &node->getPolygon(i);
			if ( collisionMask & poly->collisionMask() )
			{
				if ( poly->getDistanceSquared(moveCenter) < moveRadiusSqr )
				{
					const Vector4&	plane		= poly->plane();
					float			startDist	= plane.x*start.x + plane.y*start.y + plane.z*start.z + plane.w;
					float			endDist		= plane.x*end.x + plane.y*end.y + plane.z*end.z + plane.w;

					// some part of sphere movement in node plane thickness range?
					if ( !( (startDist-r > BSPNode::PLANE_THICKNESS && endDist-r > BSPNode::PLANE_THICKNESS) ||
						(-startDist-r > BSPNode::PLANE_THICKNESS && -endDist-r > BSPNode::PLANE_THICKNESS) ) )
					{
						// test line segment against r-shifted polygon
						Vector3 planeNormal( plane.x, plane.y, plane.z );
						Vector3 shiftDelta = planeNormal * -r;
						Vector3 startShifted = start + shiftDelta;
						float u;
						if ( Intersection::findLinePlaneIntersection(start, shiftDelta, plane, &u) )
						{
							// already inside polygon
							Vector3 ipoint = start + shiftDelta*u;
							if ( poly->isPointInPolygon(ipoint) )
							{
								setMovingSphereCollisionInfo( start, delta, 0.f, poly, planeNormal, ipoint, cinfo, t );
								continue;
							}
						}
						else if ( Intersection::findLinePlaneIntersection(startShifted, delta, plane, &u) && u <= *t )
						{
							Vector3 ipoint = startShifted + delta*u;
							if ( poly->isPointInPolygon(ipoint) )
							{
								setMovingSphereCollisionInfo( start, delta, u, poly, planeNormal, ipoint, cinfo, t );
								continue;
							}
						}

						// test line segment against vertex r-spheres
						for ( int i = 0 ; i < poly->vertices() ; ++i )
						{
							const Vector3& vert = poly->getVertex(i);
							if ( Intersection::findLineSphereIntersection(start, delta, vert, r, &u) && u <= *t )
							{
								Vector3 iposition = start + delta*u;
								Vector3 inormal = (iposition - vert).normalize();
								Vector3 ipoint; 
								if ( u == 0.f )
									ipoint = vert;
								else
									ipoint = iposition - inormal*r;
								setMovingSphereCollisionInfo( start, delta, u, poly, inormal, ipoint, cinfo, t );
							}
						}

						// test line segment against edge (k,i) r-cylinders
						const float MIN_EDGE_LEN = 1e-12f;
						int k = poly->vertices() - 1;
						for ( int i = 0 ; i < poly->vertices() ; k = i++ )
						{
							Vector3 e0 = poly->getVertex(k);
							Vector3 e1 = poly->getVertex(i);
							Vector3 cylCenter = (e0+e1)*.5f;
							Vector3 cylAxis (e1-e0);
							float cylLen = cylAxis.length();
							if ( cylLen > MIN_EDGE_LEN )
							{
								Vector3 inormal(0,0,0);
								cylAxis *= 1.f / cylLen;
								if ( Intersection::findLineCylinderIntersection(start, delta, cylCenter, cylAxis, r, cylLen*.5f, &u, &inormal) && u <= *t )
								{ 
									Vector3 iposition = start + delta*u;
									Vector3 ipoint;
									float ndotp;
									if ( u == 0.f )
									{
										ndotp = inormal.dot(iposition-cylCenter);
										ipoint = iposition - inormal*ndotp;
									}
									else
									{
										ipoint = iposition - inormal*r;
									}
									setMovingSphereCollisionInfo( start, delta, u, poly, inormal, ipoint, cinfo, t );
								}
							}
						}
					}

					++s_statistics.movingSpherePolygonTests;
				}
			}
		}
	}
}

/**
 * Finds the first moving sphere intersection against BSP tree.
 * @param root The root node of the BSP tree.
 * @param start Sphere start position.
 * @param end Sphere end position.
 * @param delta Distance vector to the end position.
 * @param r Radius of the sphere.
 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
 * @param t [in/out] Receives relative length to intersection IF there is an intersection closer than initial value.
 * @param cinfo [out] Collision check results.
 */
static void findMovingSphereIntersectionRecurse( BSPNode* root, const Vector3& start, 
	const Vector3& end, const Vector3& delta, float r, int collisionMask, float* t,
	BSPCollisionInfo* cinfo )
{
	if ( !root )
		return;
	
	const Vector4&		plane		= root->plane();
	float				startDist	= plane.x*start.x + plane.y*start.y + plane.z*start.z + plane.w;
	float				endDist		= plane.x*end.x + plane.y*end.y + plane.z*end.z + plane.w;

	if ( startDist > 0.f )
	{
		if ( endDist > 0.f )
		{
			// ++
			findMovingSphereIntersectionRecurse( root->positive(), start, end, delta, r, collisionMask, t, cinfo );
			if ( startDist-r <= BSPNode::PLANE_THICKNESS || endDist-r <= BSPNode::PLANE_THICKNESS )
			{
				findMovingSphereIntersectionPolygon( root, start, end, delta, r, collisionMask, t, cinfo );
				findMovingSphereIntersectionRecurse( root->negative(), start, end, delta, r, collisionMask, t, cinfo );
			}
			else if ( root->leaf() )
			{
				findMovingSphereIntersectionPolygon( root, start, end, delta, r, collisionMask, t, cinfo );
			}
		}
		else
		{
			// +-
			findMovingSphereIntersectionRecurse( root->positive(), start, end, delta, r, collisionMask, t, cinfo );
			findMovingSphereIntersectionPolygon( root, start, end, delta, r, collisionMask, t, cinfo );
			findMovingSphereIntersectionRecurse( root->negative(), start, end, delta, r, collisionMask, t, cinfo );
		}
	}
	else
	{
		if ( endDist > 0.f )
		{
			// -+
			findMovingSphereIntersectionRecurse( root->negative(), start, end, delta, r, collisionMask, t, cinfo );
			findMovingSphereIntersectionPolygon( root, start, end, delta, r, collisionMask, t, cinfo );
			findMovingSphereIntersectionRecurse( root->positive(), start, end, delta, r, collisionMask, t, cinfo );
		}
		else
		{
			// --
			findMovingSphereIntersectionRecurse( root->negative(), start, end, delta, r, collisionMask, t, cinfo );
			if ( -startDist-r <= BSPNode::PLANE_THICKNESS || -endDist-r <= BSPNode::PLANE_THICKNESS )
			{
				findMovingSphereIntersectionPolygon( root, start, end, delta, r, collisionMask, t, cinfo );
				findMovingSphereIntersectionRecurse( root->positive(), start, end, delta, r, collisionMask, t, cinfo );
			}
			else if ( root->leaf() )
			{
				findMovingSphereIntersectionPolygon( root, start, end, delta, r, collisionMask, t, cinfo );
			}
		}
	}
}

//-----------------------------------------------------------------------------

bool BSPCollisionUtil::findLineIntersection( BSPNode* root, const Vector3& start, 
	const Vector3& delta, int collisionMask, float* t, const BSPPolygon** cpoly )
{
	dev::Profile pr( "BSP line checks" );

	const BSPPolygon* poly = 0;
	float u = 1.f;
	Vector3 end = start + delta;
	
	findLineIntersectionRecurse( root, start, end, delta, collisionMask, &u, &poly );

	if ( t ) 
		*t = u;
	if ( cpoly )
		*cpoly = poly;
	return u < 1.f;
}

bool BSPCollisionUtil::findLastLineIntersection( BSPNode* root, 
	const Vector3& start, const Vector3& delta, int collisionMask,
	float* t, const BSPPolygon** cpoly )
{
	dev::Profile pr( "BSP line checks" );

	const BSPPolygon* poly = 0;
	float u = 0.f;
	Vector3 end = start + delta;
	
	findLastLineIntersectionRecurse( root, start, end, delta, collisionMask, &u, &poly );

	if ( t ) 
		*t = u;
	if ( cpoly )
		*cpoly = poly;
	return u > 0.f;
}

bool BSPCollisionUtil::findMovingSphereIntersection( BSPNode* root, const Vector3& start, 
	const Vector3& delta, float r, int collisionMask,
	float* t, const BSPPolygon** cpoly, Vector3* cnormal, Vector3* cpoint )
{
	dev::Profile pr( "BSP sphere checks" );

	float u = 1.f;
	Vector3 end = start + delta;
	BSPCollisionInfo cinfo;
	
	findMovingSphereIntersectionRecurse( root, start, end, delta, r, collisionMask, &u, &cinfo );

	if ( t )
		*t = u;
	if ( cpoly )
		*cpoly = cinfo.cpoly;
	if ( cnormal )
		*cnormal = cinfo.cnormal;
	if ( cpoint )
		*cpoint = cinfo.cpoint;
	return u < 1.f;
}

BSPCollisionUtil::Statistics&	BSPCollisionUtil::statistics()
{
	return s_statistics;
}

//-----------------------------------------------------------------------------

BSPCollisionUtil::Statistics::Statistics()
{
	clear();
}

void BSPCollisionUtil::Statistics::clear()
{
	linePolygonTests			= 0;
	movingSpherePolygonTests	= 0;
}


} // bsp
