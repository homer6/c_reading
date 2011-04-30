#ifndef _MESHBUILDER_VERTEXMAPFORMAT_H
#define _MESHBUILDER_VERTEXMAPFORMAT_H


namespace mb
{


/** 
 * Type identifier for vertex map data. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class VertexMapFormat
{
public:
	/** Vertex map data types. */
	enum VertexMapFormatType
	{
		/** Vertex map contains unknown data. */
		VERTEXMAP_UNKNOWN,
		/** Vertex map contains vertex weights for skinning. */
		VERTEXMAP_WEIGHT,
		/** Vertex map contains texture coordinates. */
		VERTEXMAP_TEXCOORD,
		/** Vertex map contains RGB colors of nominal level range [0,1]. */
		VERTEXMAP_RGB,
		/** Vertex map contains RGBA colors of nominal level range [0,1]. */
		VERTEXMAP_RGBA,
		/** Vertex map contains 3D unit vectors. */
		VERTEXMAP_NORMALS
	};

	///
	VertexMapFormat()																: m_type(VERTEXMAP_UNKNOWN) {}

	///
	VertexMapFormat( VertexMapFormatType type )										: m_type(type) {}

	/** Returns data type of the vertex map. */
	VertexMapFormatType		type() const											{return m_type;}

	/** Returns true if the formats are same. */
	bool					operator==( const VertexMapFormat& other ) const		{return m_type==other.m_type;}

	/** Returns true if the formats are different. */
	bool					operator!=( const VertexMapFormat& other ) const		{return m_type==other.m_type;}

private:
	VertexMapFormatType		m_type;
};


} // mb


#endif // _MESHBUILDER_VERTEXMAPFORMAT_H
