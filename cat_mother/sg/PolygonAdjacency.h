#ifndef _SG_POLYGONADJACENCY_H
#define _SG_POLYGONADJACENCY_H


#include <util/Vector.h>
#include <assert.h>


namespace sg
{


class Model;


/** 
 * Polygon adjacency information. 
 * Adjacency information gives neighbours of each polygon in O(1).
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PolygonAdjacency
{
public:
	/** No adjancency information. */
	PolygonAdjacency();

	///
	~PolygonAdjacency();

	/**
	 * Sets number of polygons and number of edges per polygon.
	 * @param polys Number of polygons.
	 * @param edgesPerPolygon Number of edges per polygon.
	 */ 
	void	setPolygons( int polys, int edgesPerPolygon );

	/**
	 * Sets adjacency information for each edge of the polygon.
	 * Index of the adjacent polygon is set for each edge,
	 * or -1 if the edge has no adjacent polygon. 
	 * The first edge is (n-1,0), second (0,1) and the last (n-2,n-1).
	 * @param poly Index of the polygon to get adjacency information.
	 * @param adj [out] Receives adjacency for each edge.
	 * @param edges Number of edges to set.
	 */
	void	setAdjacent( int poly, const int* adj, int edges );

	/** 
	 * Clears polygon adjacency information.
	 */
	void	clear();

	/**
	 * Returns adjacency information for each edge of the polygon.
	 * Index of the adjacent polygon is returned for each edge,
	 * or -1 if the edge has no adjacent polygon. 
	 * The first edge is (n-1,0), second (0,1) and the last (n-2,n-1).
	 * @param poly Index of the polygon to get adjacency information.
	 * @param adj [out] Receives adjacency for each edge.
	 * @param edges Number of edges to get.
	 */
	void	getAdjacent( int poly, int* adj, int edges ) const;

	/** Returns number of polygons in adjacency map. */
	int		polygons() const;

private:
	util::Vector<int>	m_adj;
	int					m_edges;
};


#include "PolygonAdjacency.inl"


} // sg


#endif // _SG_POLYGONADJACENCY_H
