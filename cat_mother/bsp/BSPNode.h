#ifndef _BSP_BSPNODE_H
#define _BSP_BSPNODE_H


#include <bsp/BSPPolygon.h>
#include <util/Vector.h>
#include <math/Vector4.h>


namespace bsp
{


class BSPStorage;


/** 
 * BSP tree node. Node divides space about plane to two halfspaces.
 * Each BSPNode is owned by BSPStorage.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPNode
{
public:
	/** Thickness (+-) of a node plane. */
	static const float	PLANE_THICKNESS;

	///
	BSPNode();
	
	///
	~BSPNode();

	/** 
	 * Creates a node from the split plane and the half spaces. 
	 * @param plane Split plane of the node.
	 * @param polys Polygons on the node split plane.
	 * @param pos Positive half space.
	 * @param neg Negative half space.
	 */
	void create( const math::Vector4& plane, 
		const util::Vector<BSPPolygon*>& polys,
		BSPNode* pos, BSPNode* neg, BSPStorage* storage );

	/** Returns separating plane of the node. */
	const math::Vector4&	plane() const											{return m_plane;}

	/** Returns true if this node is a leaf (no left or right children). */
	bool					leaf() const											{return !m_pos && !m_neg;}

	/** Returns the positive half space or 0 if this is a leaf. */
	BSPNode*				positive() const										{return m_pos;}

	/** Returns the negative half space or 0 if this is a leaf. */
	BSPNode*				negative() const										{return m_neg;}

	/** Returns number of polygons on the node plane. */
	int						polygons() const										{return m_polyCount;}

	/** 
	 * Returns ith polygon on the node plane. 
	 * Note that as the plane has thickness, the polygon
	 * normal might not be the same as the plane normal.
	 */
	const BSPPolygon&		getPolygon( int i ) const								{return *m_storage->nodePolygonData[m_firstPoly+i];}

	/** Returns depth of the deeper subtree, this node included. */
	int						depth() const;

	/** Returns number of nodes in the subtree, this node included. */
	int						nodes() const;

private:
	math::Vector4				m_plane;
	int							m_firstPoly;
	int							m_polyCount;
	BSPNode*					m_pos;
	BSPNode*					m_neg;
	BSPStorage*					m_storage;
};


} // bsp


#endif // _BSP_BSPNODE_H
