#ifndef _MESHBUILDER_POLYGON_H
#define _MESHBUILDER_POLYGON_H


#include <mb/MeshBuilderItem.h>


namespace mb
{


class MeshBuilder;
class VertexReference;
class Vertex;


/** Polygon of flexible mesh. */
class Polygon : 
	public MeshBuilderItem
{
public:
	///
	Polygon();

	///
	~Polygon();

	/** 
	 * Creates a new polygon from this polygon. 
	 *
	 * Both this and the new polygon will have identical information except
	 * that the new polygon will be at the end of the mesh polygon list
	 * (so the index of the new polygon will be different).
	 * Otherwise the new polygon behaves as any polygon added to the mesh
	 * with MeshBuilder::addPolygon() function.
	 *
	 * @return The pointer to the new polygon.
	 */
	virtual Polygon*	clone() const;

	/** 
	 * Splits polygon to two between two vertices. 
	 *
	 * @param v0 Index of the first vertex (between 0 and number of vertices in this polygon).
	 * @param v1 Index of the second vertex (between 0 and number of vertices in this polygon).
	 */
	void				split( int v0, int v1 );

	/** Adds a vertex to the polygon. */
	void				addVertex( Vertex* vertex );

	/** Sets ith vertex of the polygon. */
	void				setVertex( int index, Vertex* vertex );

	/** Returns ith vertex index of the polygon. */
	Vertex*				getVertex( int index );

	/** Removes specified vertex from the polygon. */
	void				removeVertex( Vertex* vertex );

	/** Removes all vertices from the polyogn. */
	void				removeVertices();

	/** 
	 * Removes a range of vertices from the polygon. 
	 *
	 * @param v0 Index of the first vertex to remove (between 0 and number of vertices in this polygon).
	 * @param v1 Index of the first remaining vertex (between 0 and number of vertices in this polygon).
	 */
	void				removeVertices( int v0, int v1 );

	/** Sets polygon material id. */
	void				setMaterial( int id );

	/** Returns ith vertex index of the polygon. */
	const Vertex*		getVertex( int index ) const;

	/** Returns index of specified vertex in the polygon. */
	int					getIndex( const Vertex* vertex ) const;

	/** Returns number of vertices in the polygon. */
	int					vertices() const;

	/** Returns index of the polygon in mesh. */
	int					index() const;

	/** Returns true if the polygon is using specified vertex. */
	bool				isUsingVertex( const Vertex* vertex ) const;

	/** Returns polygon material id. */
	int					material() const;

	/** 
	 * Returns polygon plane normal. 
	 * Takes cross product of last and first edge. 
	 * If there is less than 3 vertices then the normal is (0,0,0).
	 */
	void				getNormal( float* x, float* y, float* z ) const;

	/** Returns true if the vertex is used by the polygon. */
	bool				hasVertex( const Vertex* vertex ) const;

	/** Returns angle (radians) between the polygon face normals. */
	float				getAngle( const Polygon* poly ) const;

	/** Compares vertex indices. */
	bool				operator==( const Polygon& other ) const;

	/** Compares vertex indices. */
	bool				operator!=( const Polygon& other ) const;

private:
	friend class MeshBuilder;

	int						m_index;
	MeshBuilder*			m_mesh;
	VertexReference*		m_lastVertex;
	int						m_vertices;
	mutable float			m_normal[3];
	mutable bool			m_normalValid;
	int						m_matid;

	void					create( MeshBuilder* mesh, int index );
	void					destroy();

	VertexReference*		findRef( int index ) const;
	VertexReference*		findRef( const Vertex* vertex, int* index=0 ) const;

	Polygon( const Polygon& );
	Polygon& operator=( const Polygon& );
};


} // mb


#endif // _MESHBUILDER_POLYGON_H
