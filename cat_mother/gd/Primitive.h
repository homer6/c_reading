#ifndef _GD_PRIMITIVE_H
#define _GD_PRIMITIVE_H


namespace pix {
	class Color;}

namespace math {
	class Vector4;
	class Vector3;
	class Matrix4x4;}


namespace gd
{


class LockMode;
class VertexFormat;
class GraphicsDevice;


/** 
 * Interface to visual primitive.
 * Primitive consists of vertices and optional indices,
 * which are interpreted by the primitive type.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Primitive
{
public:
	/** Type of visual primitive. */
	enum PrimitiveType
	{
		/** Primitive type has not been set. */
		PRIMITIVE_UNDEFINED,
		/** Primitive is vertex and index list. Index triplets define the triangles. */
		PRIMITIVE_INDEXEDTRIANGLELIST,
		/** Primitive is vertex list. Vertex triplets define the triangles. */
		PRIMITIVE_TRIANGLELIST,
		/** Primitive is vertex list. Vertex pairs define the lines. */
		PRIMITIVE_LINELIST
	};

	/** 
	 * Hints the driver how the primitive will be used. 
	 *
	 * Differences between static and dynamic primitives:
	 * <ul>
	 * <li>Dynamic primitive needs to be updated every time rendered.
	 * <li>Contents of dynamic primitive will be trashed when reallocate() is called.
	 * <li>create() and reallocate() need to be called with valid device as parameter.
	 * <li>Dynamic primitive contents is write-only.
	 * </ul>
	 */
	enum UsageType
	{
		/** 
		 * The primitive will be used for non-frequently changing content. 
		 */
		USAGE_STATIC,
		/** 
		 * The primitive will be used content changing in every rendering.
		 */
		USAGE_DYNAMIC
	};

	/** Increments reference count by one. */
	virtual void		addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void		release() = 0;

	/** 
	 * Allocates visual primitive.
	 * @param device Device is needed if primitive is dynamic.
	 * @param type Type of primitive to allocate.
	 * @param vertices Number of vertices to allocate.
	 * @param indices Number of indices to allocate. Set to 0 for non-indexed primitives.
	 * @param format Format of the vertex data.
	 * @param usage Hints the driver how primitive will be used.
	 * @return Error code or 0 if ok.
	 */
	virtual int			create( GraphicsDevice* device,
							PrimitiveType type, int vertices, int indices,
							const VertexFormat& format, UsageType usage ) = 0;

	/** Deinitializes the primitive explicitly. */
	virtual void		destroy() = 0;

	/** 
	 * Allocates more vertices and indices to the object.
	 * Keeps the content of the old data. 
	 * Invalidates device object and previously requested 
	 * locked data pointers.
	 * @param device Device is needed if primitive is dynamic.
	 * @param vertices Number of vertices to allocate.
	 * @param indices Number of indices to allocate.
	 */
	virtual void		reallocate( GraphicsDevice* device, int vertices, int indices ) = 0;

	/** Uploads object to the rendering device. */
	virtual void		load( GraphicsDevice* device ) = 0;

	/** Unloads object from the rendering device. */
	virtual void		unload() = 0;

	/** 
	 * Gets access to vertex data. 
	 * Call unlockVertices() to release the access.
	 * @param device Device is needed if primitive is dynamic.
	 * @return false if lock failed
	 */
	virtual bool		lockVertices( GraphicsDevice* device, const LockMode& mode ) = 0;
	
	/** 
	 * Release access to the vertex data. 
	 */
	virtual void		unlockVertices() = 0;

	/** 
	 * Gets access to index data. 
	 * Call unlockIndices() to release the access.
	 * @param device Device is needed if primitive is dynamic.
	 * @return false if lock failed
	 */
	virtual bool		lockIndices( GraphicsDevice* device, const LockMode& mode ) = 0;
	
	/** 
	 * Release access to the index data.
	 */
	virtual void		unlockIndices() = 0;

	/** 
	 * Draws the primitive to the rendering device. 
	 */
	virtual void		draw( GraphicsDevice* device ) = 0;

	/** 
	 * Draws part of the primitive to the rendering device. 
	 */
	virtual void		draw( GraphicsDevice* device, 
							int firstVertex, int vertices, 
							int firstIndex, int indices ) = 0;

	/** 
	 * Copies vertices from another primitive. 
	 * Requires that the primitives have identical vertex formats.
	 * Requires that the vertices of this object are locked for writing
	 * and vertices of the other object are locked for reading.
	 */
	virtual void		copyVertices( int firstVertex, Primitive* other, int otherFirstVertex, int count ) = 0;

	/** 
	 * Copies indices from another primitive. 
	 * Requires that the primitives have identical vertex formats.
	 * Requires that the indices of this object are locked for writing
	 * and indices of the other object are locked for reading.
	 */
	virtual void		copyIndices( int firstIndex, Primitive* other, int otherFirstIndex, int count ) = 0;

	/** 
	 * Returns pointer to vertex position data and offset to next vertex.
	 * Requires that the vertices are locked.
	 */
	virtual void		getVertexPositionData( float** data, int* pitch ) = 0;

	/** Sets vertex positions. Requires that the vertices are locked. */
	virtual void		setVertexPositions( int firstVertex, 
							const math::Vector3* positions, int count=1 ) = 0;

	/** 
	 * Sets vertex positions with reciprocal homogenous W. 
	 * Requires that the vertices are locked. 
	 */
	virtual void		setVertexPositionsRHW( int firstVertex, 
							const math::Vector4* positions, int count=1 ) = 0;

	/** Sets vertex normal. Requires that the vertices are locked. */
	virtual void		setVertexNormals( int firstVertex, 
							const math::Vector3* normals, int count=1 ) = 0;

	/** Sets vertex diffuse colors. Requires that the vertices are locked. */
	virtual void		setVertexDiffuseColors( int firstVertex, 
							const pix::Color* colors, int count=1 ) = 0;

	/** Sets vertex specular colors. Requires that the vertices are locked. */
	virtual void		setVertexSpecularColors( int firstVertex, 
							const pix::Color* colors, int count=1 ) = 0;

	/** Sets vertex texture coordinates. Requires that the vertices are locked. */
	virtual void		setVertexTextureCoordinates( int firstVertex, 
							int layer, int coordSize, const float* coord, int count=1 ) = 0;

	/** 
	 * Sets vertex blending weights. Requires that the vertices are locked. 
	 * @param vertexIndex Index of the vertex.
	 * @param boneIndices Array of bone transform indices.
	 * @param boneWeights Weights of the bone transforms.
	 * @param bones Number of bones influencing to the vertex.
	 */
	virtual void		setVertexWeights( int vertexIndex, 
							const int* boneIndices, const float* boneWeights,
							int bones ) = 0;

	/** Sets n index values. Requires that indices are locked. */
	virtual void		setIndices( int firstIndex, 
							const int* vertices, int count=1 ) = 0;

	/** 
	 * Returns pointer to index data and byte offset to next index.
	 * Requires that the indices are locked.
	 */
	virtual void		getIndexData( void** data, int* indexSize ) = 0;

	/** Returns vertex positions. Requires that the vertices are locked. */
	virtual void		getVertexPositions( int firstVertex, 
							math::Vector3* positions, int count=1 ) const = 0;

	/** 
	 * Returns vertex positions with reciprocal homogenous W. 
	 * Requires that the vertices are locked. 
	 */
	virtual void		getVertexPositionsRHW( int firstVertex, 
							math::Vector4* positions, int count=1 ) const = 0;

	/** 
	 * Returns transformed vertex positions when using specified transformation palette.
	 * Vertices are first transformed to world space and then specified post transformation is applied.
	 * Requires that the vertices are locked.
	 * @param tm Transformation palette. Used in skinning, tm 0 is normal world transform.
	 * @param tmcount Number of transformations in the palette. Used only in skinning.
	 * @param posttm Transformation to apply to world space vertices.
	 * @param v [out] Receives transformed vertices.
	 * @param vcount Number of vertices to get.
	 * @param mostSignifigantBoneOnly If true then only the most signifigantly influencing bone is used.
	 */
	virtual void		getTransformedVertexPositions( const math::Matrix4x4* tm, int tmcount,
							const math::Matrix4x4& posttm,
							math::Vector3* v, int vcount, 
							bool mostSignifigantBoneOnly ) const = 0;

	/** Returns vertex normals. Requires that the vertices are locked. */
	virtual void		getVertexNormals( int firstVertex, 
							math::Vector3* normals, int count=1 ) const = 0;

	/** Returns vertex diffuse colors. Requires that the vertices are locked. */
	virtual void		getVertexDiffuses( int firstVertex, 
							pix::Color* colors, int count=1 ) const = 0;

	/** Returns vertex specular colors. Requires that the vertices are locked. */
	virtual void		getVertexSpeculars( int firstVertex, 
							pix::Color* colors, int count=1 ) const = 0;

	/** Returns vertex texture coordinates. Requires that the vertices are locked. */
	virtual void		getVertexTextureCoordinates( int firstVertex, 
							int layer, int coordSize, float* coord, int count=1 ) const = 0;

	/** 
	 * Returns vertex blending weights. Requires that the vertices are locked. 
	 * @param vertexIndex Index of the vertex.
	 * @param boneIndices [out] Array of bone transform indices.
	 * @param boneWeights [out] Weights of the bone transforms.
	 * @param maxBones Maximum number of bones that can be stored to the arrays.
	 * @return Number of bones influencing to the vertex.
	 */
	virtual int			getVertexWeights( int vertexIndex, 
							int* boneIndices, float* boneWeights, int maxBones ) const = 0;

	/** Returns n index values. Requires that indices are locked. */
	virtual void		getIndices( int firstIndex, int* vertices, int count=1 ) const = 0;

	/** Returns vertex format description. */
	virtual const VertexFormat&		vertexFormat() const = 0;

	/** Returns number of active vertices in the mesh. */
	virtual int			vertices() const = 0;

	/** Returns number of active indices in the mesh. */
	virtual int			indices() const = 0;

	/** Returns true if vertices are locked. */
	virtual bool		verticesLocked() const = 0;

	/** Returns true if indices are locked. */
	virtual bool		indicesLocked() const = 0;

protected:
	Primitive() {}
	virtual ~Primitive() {}

private:
	Primitive( const Primitive& );
	Primitive& operator=( const Primitive& );
};


} // gd


#endif // _GD_PRIMITIVE_H
