#ifndef _BSP_BSPCOLLISIONUTIL_H
#define _BSP_BSPCOLLISIONUTIL_H


namespace math {
	class Vector3;}


namespace bsp
{


class BSPNode;
class BSPPolygon;


/** 
 * BSP collision detection utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPCollisionUtil
{
public:
	/** Collision checking statistics. */
	class Statistics
	{
	public:
		/** Number of line-polygon intersection tests. */
		int		linePolygonTests;
		/** Number of moving sphere-polygon intersection tests. */
		int		movingSpherePolygonTests;

		/** Resets statistics. */
		Statistics();

		/** Resets statistics. */
		void	clear();
	};

	/** 
	 * Finds the first line segment intersection against BSP tree.
	 * @param root The root node of the BSP tree.
	 * @param start Line segment start position.
	 * @param delta Distance vector to the end position.
	 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
	 * @param t [out] Receives relative length [0,1] to intersection or 1 if none.
	 * @param cpoly [out] Receives pointer to the collision polygon.
	 * @return true if line segments intersects some BSP tree polygon.
	 */
	static bool	findLineIntersection( BSPNode* root, const math::Vector3& start, 
					const math::Vector3& delta, int collisionMask,
					float* t, const BSPPolygon** cpoly=0 );

	/** 
	 * Finds the last line segment intersection against BSP tree.
	 * @param root The root node of the BSP tree.
	 * @param start Line segment start position.
	 * @param delta Distance vector to the end position.
	 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
	 * @param t [out] Receives relative length [0,1] to intersection or 0 if none.
	 * @param cpoly [out] Receives pointer to the collision polygon.
	 * @return true if line segments intersects some BSP tree polygon.
	 */
	static bool	findLastLineIntersection( BSPNode* root, const math::Vector3& start, 
					const math::Vector3& delta, int collisionMask,
					float* t, const BSPPolygon** cpoly=0 );

	/**
	 * Finds the first moving sphere intersection against BSP tree.
	 * @param root The root node of the BSP tree.
	 * @param start Sphere start position.
	 * @param delta Distance vector to the end position.
	 * @param r Radius of the sphere.
	 * @param collisionMask Mask for which collision polygons to take into account. Pass -1 for all collision polygons.
	 * @param t [out] Receives relative length [0,1] to intersection or 1 if none.
	 * @param cpoly [out] Receives pointer to the collision polygon.
	 * @param cnormal [out] Receives collision plane normal if any.
	 * @param cpoint [out] Receives collision plane point if any.
	 * @return true if moving sphere intersects some BSP tree polygon.
	 */
	static bool	findMovingSphereIntersection( BSPNode* root, const math::Vector3& start, 
					const math::Vector3& delta, float r, int collisionMask,
					float* t, const BSPPolygon** cpoly=0, math::Vector3* cnormal=0, math::Vector3* cpoint=0 );

	/** Returns collision checking statistics. */
	static Statistics&	statistics();
};


} // bsp


#endif // _BSP_BSPCOLLISIONUTIL_H
