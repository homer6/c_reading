#ifndef _BSP_BSPPOLYGONSTORAGE_H
#define _BSP_BSPPOLYGONSTORAGE_H


#include <util/Vector.h>
#include <math/Vector3.h>
#include <math/Vector4.h>


namespace bsp
{


class BSPNode;
class BSPPolygon;


/** 
 * Shared storage for per polygon data. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPStorage :
	public lang::Object
{
public:
	/** Vertex indices of all polygons in the BSP-tree. */
	util::Vector<int>				indexData;
	/** Vertices of all polygons in the BSP-tree. */
	util::Vector<math::Vector3>		vertexData;
	/** Edge planes of all polygons in the BSP-tree. */
	util::Vector<math::Vector4>		edgePlaneData;
	/** Polygons stored in nodes. */
	util::Vector<BSPPolygon*>		nodePolygonData;

	BSPStorage();

	/** Sets number of BSP polygons to allocate at once. */
	void			setPolygonAllocationUnit( int size );

	/** Sets number of BSP nodes to allocate at once. */
	void			setNodeAllocationUnit( int size );

	/** Allocates a BSP polygon. */
	BSPPolygon*		createPolygon();

	/** 
	 * Allocates a BSP node. 
	 * @param plane Split plane of the node.
	 * @param polys Polygons on the node split plane.
	 * @param pos Positive half space.
	 * @param neg Negative half space.
	 */
	BSPNode*		createNode( const math::Vector4& plane, 
						const util::Vector<BSPPolygon*>& polys,
						BSPNode* pos, BSPNode* neg );

	/** Returns ith node in storage. */
	BSPNode*		getNode( int i ) const;

	/** Returns ith polygon in storage. */
	BSPPolygon*		getPolygon( int i ) const;

	/** Returns index of specified polygon. */
	int				getPolygonIndex( const BSPPolygon* poly ) const;

	/** Returns number of polygons in storage. */
	int				polygons() const;

	/** Returns number of polygons in storage. */
	int				nodes() const;

private:
	class BSPPolygonArray :
		public lang::Object
	{
	public:
		bsp::BSPPolygon*	polys;
		int					used;
		int					capacity;

		explicit BSPPolygonArray( int newCapacity );
		~BSPPolygonArray();

	private:
		BSPPolygonArray( const BSPPolygonArray& );
		BSPPolygonArray& operator=( const BSPPolygonArray& );
	};

	class BSPNodeArray :
		public lang::Object
	{
	public:
		bsp::BSPNode*		nodes;
		int					used;
		int					capacity;

		explicit BSPNodeArray( int newCapacity );
		~BSPNodeArray();

	private:
		BSPNodeArray( const BSPNodeArray& );
		BSPNodeArray& operator=( const BSPNodeArray& );
	};

	util::Vector<P(BSPPolygonArray)>	m_polyArrays;
	int									m_polyCount;
	int									m_polyAllocUnit;
	util::Vector<P(BSPNodeArray)>		m_nodeArrays;
	int									m_nodeCount;
	int									m_nodeAllocUnit;

	BSPStorage( const BSPStorage& );
	BSPStorage& operator=( const BSPStorage& );
};


} // bsp


#endif // _BSP_BSPPOLYGONSTORAGE_H
