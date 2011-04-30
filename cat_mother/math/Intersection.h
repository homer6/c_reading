#ifndef _MATH_INTERSECTION_H
#define _MATH_INTERSECTION_H


namespace math
{


class Vector3;
class Vector4;
class Matrix4x4;


/** 
 * Intersection testing utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Intersection
{
public:
	/** 
	 * Tests if point is inside oriented bounding box. 
	 * @param point Point position.
	 * @param box Box center transformation.
	 * @param dim Box axis lengths.
	 * @return true if point is inside.
	 */
	static bool testPointBox( const Vector3& point, 
		const Matrix4x4& box, const Vector3& dim );

	/** 
	 * Tests if point is inside oriented bounding box. 
	 * @param point Point position.
	 * @param box Box center transformation.
	 * @param dimMin Box axis lengths in negative directions.
	 * @param dimMax Box axis lengths in positive directions.
	 * @return true if point is inside.
	 */
	static bool testPointBox( const Vector3& point, 
		const Matrix4x4& box, const Vector3& dimMin, const Vector3& dimMax );

	/**
	 * Finds ray plane intersection if any.
	 * @param orig Ray origin.
	 * @param dir Ray direction.
	 * @param plane Plane equation. (n.x, n.y, n.z, -p0.dot(n))
	 * @param t [out] Receives length along ray to intersection if any.
	 * @return true if intersection, false otherwise.
	 */
	static bool findRayPlaneIntersection(
		const Vector3& orig, const Vector3& dir,
		const Vector4& plane, float* t );

	/**
	 * Finds ray sphere intersection if any.
	 * @param orig Ray origin.
	 * @param dir Ray direction.
	 * @param center Sphere center point.
	 * @param radius Sphere radius.
	 * @param t [out] Receives length along ray to the first intersection if any. Receives 0 if ray origin is inside sphere.
	 * @return true if intersection, false otherwise.
	 */
	static bool findRaySphereIntersection(
		const Vector3& orig, const Vector3& dir,
		const Vector3& center, float radius,
		float* t );

	/**
	 * Finds ray cylinder intersection if any.
	 * @param orig Ray origin.
	 * @param dir Ray direction.
	 * @param center Cylinder center point.
	 * @param axis Cylinder axis.
	 * @param radius Cylinder radius.
	 * @param halfHeight Half cylinder height (+-length from center point along axis).
	 * @param t [out] Receives length along ray to the first intersection if any. Receives 0 if ray origin is inside cylinder.
	 * @param normal [out] Receives intersection plane normal if any and if ptr not 0.
	 * @return true if intersection, false otherwise.
	 */
	static bool findRayCylinderIntersection(
		const Vector3& orig, const Vector3& dir,
		const Vector3& center, const Vector3& axis, float radius, float halfHeight,
		float* t, Vector3* normal );

	/** 
	 * Finds ray triangle intersection if any. 
	 * Assumes clockwise vertex order.
	 * Uses Tomas Moller's algorithm.
	 * @param orig Ray origin.
	 * @param dir Ray direction.
	 * @param vert0 The first vertex of the triangle.
	 * @param vert1 The second vertex of the triangle.
	 * @param vert2 The third vertex of the triangle.
	 * @param t [out] Receives length along ray to the intersection if any.
	 * @return true if intersection, false otherwise.
	 */
	static bool	findRayTriangleIntersection( 
		const Vector3& orig, const Vector3& dir,
		const Vector3& vert0, const Vector3& vert1, const Vector3& vert2,
		float* t );

	/**
	 * Finds ray Bezier patch intersection if any.
	 * Uses constant subdivision.
	 * @param orig Ray origin.
	 * @param dir Ray direction.
	 * @param patch Cubic Bezier patch control points.
	 * @param err Error limit. Maximum control polygon vertex distance from a plane.
	 * @param subdiv Subdivision level. For example subdiv=5 results in 5*5*2 tested triangles.
	 * @param t [out] Receives length along ray to the intersection if any.
	 * @param u [out] Receives patch u-coordinate of the intersection if any.
	 * @param v [out] Receives patch v-coordinate of the intersection if any.
	 */
	static bool	findRayCubicBezierPatchIntersection( const Vector3& orig, 
		const Vector3& dir, const Vector3 patch[4][4], int subdiv,
		float* t, float* u, float* v );

	/**
	 * Finds line plane intersection if any.
	 * @param orig Line start.
	 * @param delta Vector from line start to end.
	 * @param plane Plane equation. (n.x, n.y, n.z, -p0.dot(n))
	 * @param t [out] Receives length along line to intersection if any.
	 * @return true if intersection, false otherwise.
	 */
	static bool findLinePlaneIntersection(
		const Vector3& orig, const Vector3& delta,
		const Vector4& plane, float* t );

	/** 
	 * Finds line triangle intersection if any. 
	 * Assumes clockwise vertex order.
	 * Uses Tomas Moller's algorithm.
	 * @param orig Line origin.
	 * @param delta Line delta, vector from start to end.
	 * @param vert0 The first vertex of the triangle.
	 * @param vert1 The second vertex of the triangle.
	 * @param vert2 The third vertex of the triangle.
	 * @param t [out] Receives length along ray to the intersection if any.
	 * @return true if intersection, false otherwise.
	 */
	static bool	findLineTriangleIntersection( 
		const Vector3& orig, const Vector3& delta,
		const Vector3& vert0, const Vector3& vert1, const Vector3& vert2,
		float* t );

	/**
	 * Finds line sphere intersection if any.
	 * @param orig Line origin.
	 * @param delta Vector from line start to end.
	 * @param center Sphere center point.
	 * @param radius Sphere radius.
	 * @param t [out] Receives length along line to the first intersection if any. Receives 0 if line origin is inside sphere.
	 * @return true if intersection, false otherwise.
	 */
	static bool findLineSphereIntersection(
		const Vector3& orig, const Vector3& delta,
		const Vector3& center, float radius,
		float* t );

	/**
	 * Finds line cylinder intersection if any.
	 * @param orig Line start.
	 * @param delta Vector from line start to end.
	 * @param center Cylinder center point.
	 * @param axis Cylinder axis.
	 * @param radius Cylinder radius.
	 * @param halfHeight Half cylinder height (+-length from center point along axis).
	 * @param t [out] Receives length along line to the first intersection if any. Receives 0 if line origin is inside cylinder.
	 * @param normal [out] Receives intersection plane normal if any and if ptr not 0.
	 * @return true if intersection, false otherwise.
	 */
	static bool findLineCylinderIntersection(
		const Vector3& orig, const Vector3& delta,
		const Vector3& center, const Vector3& axis, float radius, float halfHeight,
		float* t, Vector3* normal );

	/**
	 * Computes line segment intersection against oriented bounding box if any.
	 * If line start point is inside then intersection is returned at t=0 and normal is set to zero.
	 * @param orig Line segment origin.
	 * @param delta Line segment direction and length.
	 * @param box Box center transformation.
	 * @param dim Box axis lengths.
	 * @param t [out] Receives relative length [0,1] of intersection along line segment.
	 * @param normal [out] Receives intersection point normal if intersection.
	 * @return true if intersection.
	 */
	static bool findLineBoxIntersection( const Vector3& orig, const Vector3& delta,
		const Matrix4x4& box, const Vector3& dim, float* t, Vector3* normal = 0 );

	/**
	 * Computes line segment intersection against oriented bounding box if any.
	 * If line start point is inside then intersection is returned at t=0 and normal is set to zero.
	 * @param orig Line segment origin.
	 * @param delta Line segment direction and length.
	 * @param box Box center transformation.
	 * @param dimMin Box axis lengths in negative directions.
	 * @param dimMax Box axis lengths in positive directions.
	 * @param t [out] Receives relative length [0,1] of intersection along line segment.
	 * @param normal [out] Receives intersection point normal if intersection.
	 * @return true if intersection.
	 */
	static bool findLineBoxIntersection( const Vector3& orig, const Vector3& delta,
		const Matrix4x4& box, const Vector3& dimMin, const Vector3& dimMax, float* t, Vector3* normal = 0 );

	/**
	 * Tests line segment against oriented bounding box.
	 * @param orig Line segment origin.
	 * @param delta Line segment direction and length.
	 * @param box Box center transformation.
	 * @param dim Box axis lengths.
	 * @return true if intersection.
	 */
	static bool	testLineBox( const Vector3& orig, const Vector3& delta, 
		const Matrix4x4& box, const Vector3& dim );

	/**
	 * Tests if two oriented boxes overlap.
	 * Uses Stefan Gottchalk's separating axis algorithm.
	 * @param dim Dimensions of the first OBB.
	 * @param otherTm Transformation of the second OBB in the first OBB space.
	 * @param otherDim Dimensions of the second OBB.
	 * @return true if the OBBs overlap, false otherwise.
	 */
	static bool testBoxBox( const Vector3& dim,
		const Matrix4x4& otherTm, const Vector3& otherDim );

	/**
	 * Tests if two vertical cylinders overlap.
	 * @param bot1 bottom vertex of first cylinder
	 * @param height1 height of first cylinder
	 * @param radius1 radius of first cylinder
	 * @param bot2 bottom vertex of second cylinder
	 * @param height2 height of second cylinder
	 * @param radius2 radius of second cylinder
	 */
	static bool testVerticalCylinderCylinder( const Vector3& bot1, float height1, float radius1, 
		const Vector3& bot2, float height2, float radius2 );
};


} // math


#endif // _MATH_INTERSECTION_H
