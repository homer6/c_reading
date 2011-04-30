#ifndef _MESHBUILDER_MESHBUILDER_H
#define _MESHBUILDER_MESHBUILDER_H


#include <lang/Object.h>


namespace lang {
	class String;}


namespace mb
{


class VertexReference;
class PolygonReference;
class Polygon;
class Vertex;
class VertexMap;
class DiscontinuousVertexMap;
class VertexMapFormat;


/** 
 * Flexible polygon mesh. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MeshBuilder :
	public lang::Object
{
public:
	///
	MeshBuilder();

	///
	virtual ~MeshBuilder();

	/** 
	 * Splits vertices are used by many polygons and which have 
	 * different vertex map values assigned for each polygon.
	 */
	void				splitVertexDiscontinuities( DiscontinuousVertexMap* vmad );

	/** 
	 * Removes vertices which are not used by any polygon. 
	 * Note that this method causes vertex re-indexing.
	 */
	void				removeUnusedVertices();

	/** Adds a new polygon to the mesh. */
	Polygon*			addPolygon();

	/** Adds a new vertex to the mesh. */
	Vertex*				addVertex();

	/** Adds an n-dimensional vertex map to the mesh. */
	VertexMap*			addVertexMap( int dimensions, const lang::String& name, 
							const VertexMapFormat& format );

	/** Adds an n-dimensional discontinuous vertex map to the mesh. */
	DiscontinuousVertexMap*	addDiscontinuousVertexMap( int dimensions, const lang::String& name, 
								const VertexMapFormat& format );

	/** Removes a polygon from the mesh. */
	void				removePolygon( int index );

	/** 
	 * Removes a vertex from the mesh. 
	 * There must be no polygons using the vertex.
	 */
	void				removeVertex( int index );

	/** Removes a vertex map from the mesh. */
	void				removeVertexMap( int index );

	/** Removes a discontinuous vertex map from the mesh. */
	void				removeDiscontinuousVertexMap( int index );

	/** Removes everything (polygons, vertices, vertex maps, etc.) from the mesh. */
	virtual void		clear();

	/** Removes all polygons from the mesh. */
	void				removePolygons();

	/** 
	 * Removes all vertices from the mesh. 
	 * There must be no polygons using the vertices.
	 */
	void				removeVertices();

	/** Removes all vertex maps from the mesh. */
	void				removeVertexMaps();

	/** Removes all discontinuous vertex maps from the mesh. */
	void				removeDiscontinuousVertexMaps();

	/** Returns specified polygon from the mesh. */
	Polygon*			getPolygon( int index );

	/** Returns specified vertex from the mesh. */
	Vertex*				getVertex( int index );

	/** Returns specified vertex map from the mesh. */
	VertexMap*			getVertexMap( int index );

	/** Returns specified discontinuous vertex map from the mesh. */
	DiscontinuousVertexMap*	getDiscontinuousVertexMap( int index );

	/** 
	 * Returns specified vertex map from the mesh by name or 0 if the map was not found. 
	 */
	VertexMap*			getVertexMapByName( const lang::String& name );

	/** 
	 * Returns specified discontinuous vertex map from the mesh by name or 0 if the map was not found.
	 */
	DiscontinuousVertexMap*	getDiscontinuousVertexMapByName( const lang::String& name );

	/** Returns number of polygons. */
	int					polygons() const;

	/** Returns number of vertices. */
	int					vertices() const;

	/** Returns number of vertex maps. */
	int					vertexMaps() const;

	/** Returns number of discontinuous vertex maps. */
	int					discontinuousVertexMaps() const;

	/** Returns true if the geometry is equal. */
	bool				operator==( const MeshBuilder& other ) const;

	/** Returns true if the geometry is not equal. */
	bool				operator!=( const MeshBuilder& other ) const;

protected:
	/** Allocates a vertex. */
	virtual Vertex*					allocateVertex();

	/** Allocates a polygon. */
	virtual Polygon*				allocatePolygon();

	/** Allocates a vertex reference. */
	virtual VertexReference*		allocateVertexReference();

	/** Allocates a polygon reference. */
	virtual PolygonReference*		allocatePolygonReference();

	/** Allocates a vertex map. */
	virtual VertexMap*				allocateVertexMap();

	/** Allocates a discontinuous vertex map. */
	virtual DiscontinuousVertexMap*	allocateDiscontinuousVertexMap();

	/** Frees a vertex. */
	virtual void	freeVertex( Vertex* item );

	/** Frees a polygon. */
	virtual void	freePolygon( Polygon* item );

	/** Frees a vertex reference. */
	virtual void	freeVertexReference( VertexReference* item );

	/** Frees a polygon reference. */
	virtual void	freePolygonReference( PolygonReference* item );

	/** Frees a vertex map. */
	virtual void	freeVertexMap( VertexMap* item );

	/** Frees a discontinuous vertex map. */
	virtual void	freeDiscontinuousVertexMap( DiscontinuousVertexMap* item );

private:
	friend class Polygon;
	friend class Vertex;

	class MeshBuilderImpl;
	P(MeshBuilderImpl)	m_this;

	MeshBuilder( const MeshBuilder& );
	MeshBuilder& operator=( const MeshBuilder& );
};


} // mb


#endif // _MESHBUILDER_MESHBUILDER_H
