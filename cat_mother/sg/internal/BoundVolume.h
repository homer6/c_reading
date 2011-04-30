#ifndef _SG_BOUNDVOLUME_H
#define _SG_BOUNDVOLUME_H


namespace math {
	class Vector3;
	class Vector4;
	class Matrix4x4;}


namespace sg
{


/** 
 * Bounding volume computation utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BoundVolume
{
public:
	/**
	 * Checks if sphere intersects or is inside volume.
	 * Plane normals point away from the volume.
	 * @param pos Sphere center position.
	 * @param r Sphere radius.
	 * @param planes Volume planes in the same space as the sphere.
	 * @param planeCount Number of planes defining the volume.
	 * @return true if the sphere is (partially) inside the volume.
	 */
	static bool		testSphereVolume( const math::Vector3& pos, float r,
						const math::Vector4* planes, int planeCount );

	/**
	 * Checks if cloud of points intersects or is inside volume.
	 * Plane normals point away from the volume.
	 * @param points First item of the point set.
	 * @param pointCount Number of items in the point set.
	 * @param planes Volume planes in the same space as the points.
	 * @param planeCount Number of planes defining the volume.
	 * @return true if any point is inside the volume.
	 */
	static bool		testPointsVolume( const math::Vector3* points, int pointCount,
						const math::Vector4* planes, int planeCount );
};


} // sg


#endif // _SG_BOUNDVOLUME_H
