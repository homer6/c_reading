#ifndef _MESHBUILDER_VERTEX_H
#define _MESHBUILDER_VERTEX_H


#include <mb/MeshBuilderItem.h>


namespace mb
{


class MeshBuilder;
class PolygonReference;
class Polygon;


/** 
 * Vertex of flexible mesh. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Vertex : 
	public MeshBuilderItem
{
public:
	///
	Vertex();

	///
	~Vertex();

	/** 
	 * Creates a new copy from this vertex. 
	 *
	 * Both this and the new vertex will have identical information except
	 * that the new vertex will be at the end of the mesh vertex list
	 * (so the index of the new vertex will be different)
	 * and the new vertex will not be used by any polygon.
	 * Otherwise the new vertex behaves as any vertex added to the mesh
	 * with MeshBuilder::addVertex() function.
	 *
	 * @return The pointer to the new vertex.
	 */
	virtual Vertex*			clone() const;

	/** Sets position of the vertex. */
	void					setPosition( float x, float y, float z );

	/** Returns ith polygon using the vertex. */
	Polygon*				getPolygon( int index );

	/** 
	 * Sets the vertex index. 
	 * Use with caution as setting index can invalidate vertex map searches
	 * if you have stored values to vertex maps with the old index.
	 */
	//void					setIndex( int i );

	/** Returns position of the vertex. */
	void					getPosition( float* x, float* y, float* z ) const;

	/** 
	 * Returns vertex normal. Vertex normal is computed from polygon information. 
	 * If the vertex isn't connected to any polygons then the normal is (0,0,0).
	 */
	void					getNormal( float* x, float* y, float* z ) const;

	/** Returns ith polygon using the vertex. */
	const Polygon*			getPolygon( int index ) const;

	/** Returns number of polygons using the vertex. */
	int						polygons() const;

	/** Returns index of the polygon in mesh. */
	int						index() const;

	/** Compares vertex positions. */
	bool					operator==( const Vertex& other ) const;

	/** Compares vertex positions. */
	bool					operator!=( const Vertex& other ) const;

private:
	friend class MeshBuilder;
	friend class Polygon;

	int					m_index;
	MeshBuilder*		m_mesh;
	PolygonReference*	m_lastPolygon;
	int					m_polygons;
	float				m_position[3];

	void				create( MeshBuilder* mesh, int index );
	void				destroy();
	void				addPolygon( Polygon* polygon );
	void				removePolygon( Polygon* polygon );
	void				removePolygons();

	PolygonReference*	findRef( Polygon* polygon );

	Vertex( const Vertex& );
	Vertex& operator=( const Vertex& );
};


} // mb


#endif // _MESHBUILDER_VERTEX_H
