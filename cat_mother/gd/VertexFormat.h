#ifndef _GD_VERTEXFORMAT_H
#define _GD_VERTEXFORMAT_H


#include <assert.h>


namespace gd
{


/**
 * Vertex format description.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class VertexFormat
{
public:
	/** Constructs a vertex format with position information only. */
	VertexFormat();

	/** Returns true if formats are equal. */
	bool			operator==( const VertexFormat& other ) const;

	/** Returns true if formats are inequal. */
	bool			operator!=( const VertexFormat& other ) const;

	/** Adds reciprocal homogenous W information to the vertex format. */
	VertexFormat&	addRHW();

	/** Adds vertex normal information to the vertex format. */
	VertexFormat&	addNormal();
	
	/** Adds diffuse color information to the vertex format. */
	VertexFormat&	addDiffuse();
	
	/** Adds specular color information to the vertex format. */
	VertexFormat&	addSpecular();

	/** Removes reciprocal homogenous W information from the vertex format. */
	VertexFormat&	removeRHW();

	/** Removes vertex normal information from the vertex format. */
	VertexFormat&	removeNormal();
	
	/** Removes diffuse color information from the vertex format. */
	VertexFormat&	removeDiffuse();
	
	/** Removes specular color information from the vertex format. */
	VertexFormat&	removeSpecular();

	/** 
	 * Sets vertex weight information to the model vertex format.
	 * Number of vertex weights is one less than the number of 
	 * affecting bones per vertex. (the last vertex weights 
	 * is implicitly defined by the other weights)
	 */
	VertexFormat&	setWeights( int weights );

	/** Adds a texture coordinate layer. */
	VertexFormat&	addTextureCoordinate( int dim );

	/** Sets a texture coordinate layer size in the vertex format. */
	void			setTextureCoordinateSize( int layer, int dim );

	/** Sets number of texture coordinate layers. */
	void			setTextureCoordinates( int layers );

	/** Returns true if the vertex format has reciprocal homogenous W information. */
	bool			hasRHW() const;

	/** Returns true if the vertex format has vertex normal information. */
	bool			hasNormal() const;
	
	/** Returns true if the vertex format has diffuse color information. */
	bool			hasDiffuse() const;
	
	/** Returns true if the vertex format has specular color information. */
	bool			hasSpecular() const;

	/** Returns number of texture coordinates in the vertex format information. */
	int				textureCoordinates() const;

	/** Returns number of dimensions in the texture coordinate layer. */
	int				getTextureCoordinateSize( int layerIndex ) const;

	/** Returns total number of dimensions in all texture coordinate layers. */
	int				totalTextureCoordinateSize() const;

	/** Returns number of vertex weights in the vertex format information. */
	int				weights() const;

private:
	enum Constants
	{ 
		MAX_LAYERS = 4 
	};

	enum Flags
	{
		VF_XYZ              = 1,
		VF_RHW				= 2,
		VF_NORMAL           = 4,
		VF_DIFFUSE          = 8,
		VF_SPECULAR         = 16,
		VF_SHADER			= (1<<14),
	};

	unsigned short	m_vf;
	unsigned char	m_texcoords;
	unsigned char	m_weights;
	unsigned char	m_texcoordSizes[MAX_LAYERS];
};


#include "VertexFormat.inl"


} // gd


#endif // _GD_VERTEXFORMAT_H
