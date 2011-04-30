#ifndef _MESHBUILDER_DISCONTINUOUSVERTEXMAP_H
#define _MESHBUILDER_DISCONTINUOUSVERTEXMAP_H


#include <mb/MeshBuilderItem.h>


namespace lang {
	class String;}


namespace mb
{


class VMapImpl;
class MeshBuilder;
class VertexMapFormat;


/**
 * N dimensional discontinuous (polygon-based) vertex map.
 * Each n-dimensional value in the discontinuous vertex map is 
 * associated with specific vertex index and polygon index.
 * Values for every vertex or polygon in the mesh don't need to be in the map.
 * Discontinuous vertex map is flexible structure for storing 
 * polygon corner specific data.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DiscontinuousVertexMap :
	public MeshBuilderItem
{
public:
	///
	DiscontinuousVertexMap();

	///
	~DiscontinuousVertexMap();

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
	void					addValue( int vertexIndex, int polygonIndex, const float* data, int size );

	/** Removes an item from the vertex map. */
	void					removeValue( int vertexIndex, int polygonIndex );

	/** Called if a polygon is removed from the mesh. */
	void					polygonRemoved( int index );

	/** Called if a vertex is removed from the mesh. */
	void					vertexRemoved( int index );

	/** 
	 * Gets an item from the vertex map. 
	 *
	 * @return false if the item was not found from the map.
	 */
	bool					getValue( int vertexIndex, int polygonIndex, float* data, int size ) const;

	/** Returns number of dimensions in the vertex map. */
	int						dimensions() const;

	/** Returns number of entries (vertex-face pairs) in the discontinuous vertex map. */
	int						size() const;

	/** Returns name of the vertex map. */
	const lang::String&		name() const;

	/** Returns type of the vertex map data. */
	const VertexMapFormat&	format() const;

	/** Compares vertex map entries. */
	bool					operator==( const DiscontinuousVertexMap& other ) const;

	/** Compares vertex map entries. */
	bool					operator!=( const DiscontinuousVertexMap& other ) const;

private:
	P(VMapImpl) m_this;

	DiscontinuousVertexMap( const DiscontinuousVertexMap& );
	DiscontinuousVertexMap& operator=( const DiscontinuousVertexMap& );
};


} // mb


#endif // _MESHBUILDER_DISCONTINUOUSVERTEXMAP_H
