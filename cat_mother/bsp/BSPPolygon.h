#ifndef _BSP_BSPPOLYGON_H
#define _BSP_BSPPOLYGON_H


#include "BSPStorage.h"


namespace bsp
{


class BSPStorage;


/** 
 * BSP tree polygon. Polygon vertices are defined in clockwise order.
 * Each BSPPolygon is owned by BSPStorage.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPPolygon
{
public:
	/** Creates undefined polygon. */
	BSPPolygon()																	: m_id(-1), m_firstIndex(0), m_firstEdgePlane(0), m_vertices(0) {}

	/** Creates n-vertex polygon. */
	BSPPolygon( const math::Vector3* v, int n, int id,
		BSPStorage* storage, int collisionMask );

	/** Creates n-vertex polygon with vertices already in storage. */
	BSPPolygon( const int* indices, int n, int id,
		BSPStorage* storage, int collisionMask );

	/** Creates n-vertex polygon with vertices already in storage. */
	void create( const int* indices, int n, int id,
		BSPStorage* storage, int collisionMask );

	/** Returns number of vertices in the polygon. */
	int		vertices() const														{return m_vertices;}

	/** Returns user defined id of the polygon. */
	int		id() const																{return m_id;}

	/** Returns ith vertex of the polygon. */
	const math::Vector3&	getVertex( int i ) const								{return m_storage->vertexData.get( m_storage->indexData.get(m_firstIndex+i) );}

	/** Returns ith vertex index of the polygon. */
	int						getVertexIndex( int i ) const							{return m_storage->indexData.get(m_firstIndex+i);}

	/** Returns plane of the polygon. */
	const math::Vector4&	plane() const											{return m_plane;}

	/** 
	 * Returns true if specified point is inside the polygon (inclusive). 
	 * Assumes that the point is on the polygon plane.
	 */
	bool					isPointInPolygon( const math::Vector3& point ) const;

	/** Returns distance to polygon bounding sphere squared. */
	float					getDistanceSquared( const math::Vector3& point ) const	{float dx=m_boundSphereCenter[0]-point.x; float dy=m_boundSphereCenter[1]-point.y; float dz=m_boundSphereCenter[2]-point.z; return dx*dx+dy*dy+dz*dz-m_boundSphereRadiusSqr;}

	/** Returns bounding sphere radius squared. */
	float					boundSphereRadiusSquared() const						{return m_boundSphereRadiusSqr;}

	/** Returns shared polygon data storage. */
	BSPStorage*				storage() const											{return m_storage;}

	/** Returns true if polygon vertices are identical. */
	bool					operator==( const BSPPolygon& other ) const;

	/** 
	 * Returns collision mask. 
	 * Collision flags are used to filter out collision checks.
	 */
	int						collisionMask() const									{return m_collisionMask;}

private:
	int					m_id;
	int					m_firstIndex;
	int					m_firstEdgePlane;
	int					m_vertices;
	BSPStorage*			m_storage;
	math::Vector4		m_plane;
	float				m_boundSphereCenter[3];
	float				m_boundSphereRadiusSqr;
	int					m_collisionMask;

	void	computePlanes();
};


} // bsp


#endif // _BSP_BSPPOLYGON_H
