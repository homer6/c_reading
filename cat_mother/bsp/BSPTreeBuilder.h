#ifndef _BSP_BSPTREEBUILDER_H
#define _BSP_BSPTREEBUILDER_H


#include <bsp/BSPNode.h>
#include <lang/Object.h>


namespace math {
	class Vector3;}


namespace bsp
{


class BSPTree;
class BSPSplitSelector;


/** 
 * Helper class for building BSP trees.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPTreeBuilder :
	public lang::Object
{
public:
	/** Thickness of a node plane. */
	static const float	PLANE_THICKNESS;

	/** Creates empty BSP tree builder. */
	explicit BSPTreeBuilder();

	///
	~BSPTreeBuilder();

	/** 
	 * Inserts a source polygon to the BSP tree.
	 * Polygon vertices are defined in clockwise order.
	 * Ignores degenerate polygons.
	 * @param v Pointer to the first vertex of the polygon.
	 * @param n Number of vertices in the polygon.
	 * @param id User defined identifier of the polygon.
	 * @param collisionMask User defined mask for run-time collision checking filtering. Pass -1 to collide everything.
	 */
	void		addPolygon( const math::Vector3* v, int n, int id, int collisionMask );

	/** Removes all source polygons. */
	void		removePolygons();

	/** 
	 * Builds the BSP tree from added source polygons. 
	 * Uses specified split plane selector.
	 */
	BSPTree*	build( BSPSplitSelector* splitSel );

	/**
	 * Returns relative amount of build work done.
	 * This method is synchronized so that you can have another thread
	 * executing build() and another polling progress().
	 * @return Relative amount [0,1] of build work done.
	 */
	float		progress() const;

	/** Returns number of source polygons in the BSP tree. */
	int			polygons() const;

private:
	class BSPTreeBuilderImpl;
	P(BSPTreeBuilderImpl)	m_this;

	BSPTreeBuilder( const BSPTreeBuilder& );
	BSPTreeBuilder& operator=( const BSPTreeBuilder& );
};


} // bsp


#endif // _BSP_BSPTREEBUILDER_H
