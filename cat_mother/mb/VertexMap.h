#ifndef _MESHBUILDER_VERTEXMAP_H
#define _MESHBUILDER_VERTEXMAP_H


#include <mb/MeshBuilderItem.h>


namespace lang {
	class String;}


namespace mb
{


class VMapImpl;
class MeshBuilder;
class VertexMapFormat;


/**
 * N dimensional vertex map.
 *
 * Each n-dimensional value in the vertex map is associated with specific vertex index.
 * Values for every vertex in the mesh don't need to be in the map.
 * Vertex map is flexible structure for storing vertex specific data.
 *
 * @see DiscontinuousVertexMap
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class VertexMap :
	public MeshBuilderItem
{
public:
	///
	VertexMap();

	///
	~VertexMap();

	/** 
	 * Sets number of dimensions in the map. 
	 * Must be set before any value is added.
	 */
	void					setDimensions( int dim );

	/** Sets name of the map. */
	void					setName( const lang::String& name );

	/** Sets vertex map data type. */
	void					setFormat( const VertexMapFormat& format );

	/** Adds an item to vertex map. */
	void					addValue( int vertexIndex, const float* data, int size );

	/** Removes an item from the vertex map. */
	void					removeValue( int vertexIndex );

	/** Called if a vertex is removed from the mesh. */
	void					vertexRemoved( int index );

	/** 
	 * Gets an item from the vertex map. 
	 *
	 * @return false if the item was not found from the map.
	 */
	bool					getValue( int vertexIndex, float* data, int size ) const;

	/** Returns number of dimensions in the vertex map. */
	int						dimensions() const;

	/** Returns name of the vertex map. */
	const lang::String&		name() const;

	/** Returns type of the vertex map data. */
	const VertexMapFormat&	format() const;

	/** Returns number of entries in the vertex map. */
	int						size() const;

	/** Compares vertex map entries. */
	bool					operator==( const VertexMap& other ) const;

	/** Compares vertex map entries. */
	bool					operator!=( const VertexMap& other ) const;

private:
	P(VMapImpl) m_this;

	VertexMap( const VertexMap& );
	VertexMap& operator=( const VertexMap& );
};


} // mb


#endif // _MESHBUILDER_VERTEXMAP_H
