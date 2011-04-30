#ifndef _SG_MODEL_H
#define _SG_MODEL_H


#include <sg/Primitive.h>
#include <math/Vector3.h>
#include <assert.h>


namespace pix {
	class Color;}

namespace lang {
	class String;}

namespace math {
	class OBBox;}


namespace sg
{


class ModelLock;
class VertexFormat;
class PolygonAdjacency;


/** 
 * Indexed triangle primitive. 
 * Polygons are stored as index triplets to vertex list.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Model :
	public Primitive
{
public:
	/** 
	 * Data lock mode.
	 * @see ModelLock
	 */
	enum LockType
	{
		/** Lock data for reading only. */
		LOCK_READ,
		/** Lock data for writing only. */
		LOCK_WRITE,
		/** Lock data for both reading and writing. */
		LOCK_READWRITE
	};

	/** Hints the driver how the primitive will be used. */
	enum UsageType
	{
		/** The primitive will be used mainly for non-frequently changing  content. */
		USAGE_STATIC,
		/** The primitive will be used for content changing in every rendering. */
		USAGE_DYNAMIC
	};

	/** 
	 * Creates a primitive with space allocated for specified amount of vertices and indices. 
	 * Number of triangles defined by the indices is 1/3 of the number of indices.
	 */
	Model( int vertices, int indices, const VertexFormat& vf,
		UsageType usage=USAGE_STATIC );

	/** Creates  clone of other primitive. */
	Model( const Model& other, int shareFlags );

	/** Returns clone of this primitive. */
	Primitive* clone( int shareFlags ) const;

	/** Destroys the object. */
	void	destroy();

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** Computes object visibility in the view frustum. */
	bool	updateVisibility( const math::Matrix4x4& modelToCamera, 
				const ViewFrustum& viewFrustum );

	/** Draws the model to the active rendering device. */
	void	draw();

	/**
	 * Sets number of active vertices.
	 * The function can invalidate device objects.
	 */
	void	setVertices( int count );

	/**
	 * Sets number of active indices.
	 * The function can invalidate device objects.
	 */
	void	setIndices( int count );

	/**
	 * Grows maximum number of vertices if needed.
	 * Invalidates old device object.
	 */
	void	setMaxVertices( int count );

	/**
	 * Grows maximum number of indices if needed.
	 * Invalidates old device object.
	 */
	void	setMaxIndices( int count );

	/** 
	 * Copies vertices from another primitive. 
	 * Requires that the models have identical vertex formats.
	 * Requires that the vertices of this object are locked for writing
	 * and vertices of the other object are locked for reading.
	 */
	void	copyVertices( int firstVertex, const Model* other, int otherFirstVertex, int count );

	/** 
	 * Copies indices from another primitive. 
	 * Requires that the indices of this object are locked for writing
	 * and indices of the other object are locked for reading.
	 */
	void	copyIndices( int firstIndex, const Model* other, int otherFirstIndex, int count );

	/** Sets vertex positions. Requires that the vertices are locked. */
	void	setVertexPositions( int firstVertex, const math::Vector3* positions, int count=1 );

	/** 
	 * Sets vertex positions with reciprocal homogenous W. 
	 * Requires that the vertices are locked. 
	 */
	void	setVertexPositionsRHW( int firstVertex, const math::Vector4* positions, int count=1 );

	/** Sets vertex normal. Requires that the vertices are locked. */
	void	setVertexNormals( int firstVertex, const math::Vector3* normals, int count=1 );

	/** Sets vertex texture coordinates. Requires that the vertices are locked. */
	void	setVertexTextureCoordinates( int firstVertex, int layer, int coordSize, const float* coord, int count=1 );

	/** 
	 * Sets vertex blending weights. Requires that the vertices are locked. 
	 * The bone index 0 is always the mesh transformation and
	 * actual bone transform indices start at 1.
	 * @param vertexIndex Index of the vertex.
	 * @param boneIndices Array of bone transform indices.
	 * @param boneWeights Weights of the bone transforms.
	 * @param bones Number of bones influencing to the vertex.
	 */
	void	setVertexWeights( int vertexIndex, const int* boneIndices, const float* boneWeights, int bones );

	/** Sets vertex diffuse colors. Requires that the vertices are locked. */
	void	setVertexDiffuseColors( int firstVertex, const pix::Color* colors, int count=1 );

	/** Sets n index values. Requires that indices are locked. */
	void	setIndices( int firstIndex, const int* vertices, int count=1 );

	/** 
	 * Locks vertex data. Don't use this method directly,
	 * but use VertexLock instead. (which automatically releases 
	 * the lock at the end of the scope).
	 * @see VertexLock
	 * @exception LockException
	 */
	void	lockVertices( LockType lock );

	/** 
	 * Unlocks vertex data. 
	 * @see lockVertices 
	 */
	void	unlockVertices();

	/** 
	 * Locks index data. Don't use this method directly,
	 * but use IndexLock instead. (which automatically releases 
	 * the lock at the end of the scope).
	 * @see IndexLock
	 */
	void	lockIndices( LockType lock );

	/** 
	 * Unlocks vertex data. 
	 * @see lockVertices 
	 */
	void	unlockIndices();

	/** 
	 * Resets model adjacency information.
	 */
	void	clearPolygonAdjancency();

	/** 
	 * Computes vertex normals.
	 * Takes cross product of the last and the first edge of each vertex. 
	 * If there is less than 3 vertices in a polygon then the normal is (0,0,0).
	 * Requires that vertices and indices are locked for reading and writing.
	 */
	void	computeVertexNormals();

	/**
	 * Computes vertex tangent space basis used in bump mapping.
	 * Requires that vertices and indices are locked for reading and writing.
	 * @param srcLayer Source layer for texture coordinates.
	 * @param dstLayer Destination texture coordinate layer for tangent space U-vector.
	 */
	void	computeVertexTangents( int srcLayer, int dstLayer );

	/**
	 * Optimizes bone indices used by vertices so that vertices refer only
	 * to bones in range [0,usedBones) and usedBoneArray() must be used to set
	 * correct bone transforms. optimizeBoneIndices() updates usedBoneArray() list.
	 * Requires that vertices and indices are locked for reading and writing.
	 *
	 * Example, before optimizeBoneIndices:<br>
	 * vertexBoneIndices0 = {0,2,2,3,4,6}<br>
	 * And after optimizeBoneIndices:<br>
	 * vertexBoneIndices0 = {0,1,1,2,3,4}<br>
	 */
	void	optimizeBoneIndices();

	/**
	 * Returns pointer to vertex position data and offset to next vertex.
	 * Requires that the vertices are locked.
	 * For optimization use only, normally use set/getVertexPositions.
	 */
	void	getVertexPositionData( float** data, int* pitch );

	// planned support for multiple materials in same Model -- needed for pmeshes and useful for morphing?
	//void	setIndexGroup( int groupId, int firstIndex, int lastIndex );
	//void	setVertexGroup( int groupId, int firstVertex, int lastVertex );
	//void	setGroupShader( int groupId, Shader* shader );

	/** Sets model bounding sphere manually. Any call to lockVertices invalidates any set bounding sphere. */
	void	setBoundSphere( float r );

	/** Sets model bounding box manually. Any call to lockVertices invalidates any set bounding boxes. */
	void	setBoundBox( const math::OBBox& box );

	/** 
	 * Returns number of used bones by this primitive.
	 * The function is valid only if optimizeBoneIndices() has been called.
	 */
	int		usedBones() const;

	/** 
	 * Returns array of used bones by this primitive.
	 * If the primitive has no bones the return value is 0.
	 * The function is valid only if optimizeBoneIndices() has been called.
	 */
	const int*	usedBoneArray() const;

	/** 
	 * Computes sphere bounding all vertex positions.
	 * Requires that the vertices are not locked.
	 */
	float	boundSphere() const;

	/** 
	 * Computes box bounding all vertex positions.
	 * Requires that the vertices are not locked.
	 */
	const math::OBBox& boundBox() const;

	/** Returns vertex positions. Requires that the vertices are locked. */
	void	getVertexPositions( int firstVertex, math::Vector3* positions, int count=1 ) const;

	/** 
	 * Returns vertex positions with reciprocal homogenous W. 
	 * Requires that the vertices are locked. 
	 */
	void	getVertexPositionsRHW( int firstVertex, math::Vector4* positions, int count=1 ) const;

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
	void	getTransformedVertexPositions( const math::Matrix4x4* tm, int tmcount,
				const math::Matrix4x4& posttm, math::Vector3* v, int vcount, bool mostSignifigantBoneOnly ) const;

	/** Returns vertex normals. Requires that the vertices are locked. */
	void	getVertexNormals( int firstVertex, math::Vector3* normals, int count=1 ) const;

	/** Returns vertex texture coordinates. Requires that the vertices are locked. */
	void	getVertexTextureCoordinates( int firstVertex, int layer, int coordSize, float* coord, int count=1 ) const;

	/** 
	 * Returns vertex blending weights. Requires that the vertices are locked. 
	 * The bone index 0 is always the mesh transformation and
	 * actual bone transform indices start at 1.
	 * @param vertexIndex Index of the vertex.
	 * @param boneIndices [out] Array of bone transform indices.
	 * @param boneWeights [out] Weights of the bone transforms.
	 * @param maxBones Maximum number of bones that can be stored to the arrays.
	 * @return Number of bones influencing to the vertex.
	 */
	int		getVertexWeights( int vertexIndex, int* boneIndices, float* boneWeights, int maxBones ) const;

	/** Returns vertex diffuse colors. Requires that the vertices are locked. */
	void	getVertexDiffuseColors( int firstVertex, pix::Color* colors, int count=1 ) const;

	/** Returns n index values. Requires that indices are locked. */
	void	getIndices( int firstIndex, int* vertices, int count=1 ) const;

	/** 
	 * Returns pointer to index data and byte offset to next index.
	 * Requires that the indices are locked.
	 * Warning: Use this only for optimization.
	 */
	void	getIndexData( void** data, int* indexSize );

	/** Returns number of active vertices in the model. */
	int		vertices() const;

	/** Returns number of active indices in the model. */
	int		indices() const;

	/** Returns maximum number of vertices in the model. */
	int		maxVertices() const;

	/** Returns maximum number of indices in the model. */
	int		maxIndices() const;

	/** Returns true if vertices are locked. */
	bool	verticesLocked() const;

	/** Returns true if indices are locked. */
	bool	indicesLocked() const;

	/** Returns vertex format of the geometry. */
	VertexFormat	vertexFormat() const;

	/** Returns model usage type. */
	UsageType		usage() const;

	/** 
	 * Returns polygon adjacency information.
	 * Requires that vertices and indices are locked for reading.
	 * @param zeroDistance Maximum distance between vertices to be considered equal.
	 */
	const PolygonAdjacency&	getPolygonAdjacency( float zeroDistance ) const;

	/** 
	 * Sorts weights to descending order. 
	 * @param sortedBoneIndices [out] Receives indices of sorted weights.
	 * @param sortedBoneWeights [out] Receives weights of sorted weights.
	 * @return Number of weights written to output.
	 */
	static int	sortVertexWeights( const int* boneIndices, const float* boneWeights, int bones,
					int* sortedBoneIndices, float* sortedBoneWeights );

private:
	friend class ModelLock;
	typedef unsigned short IndexType;

	class ModelImpl;
	P(ModelImpl) m_this;

	float*		m_posData;
	int			m_posPitch;
	IndexType*	m_indexData;

	bool	canReadVertices() const;
	bool	canWriteVertices() const;
	bool	canReadIndices() const;
	bool	canWriteIndices() const;
	void	lockDefaults();
	void	getLockedData();

	Model();
	Model( const Model& );
	Model& operator=( const Model& other );
};


#include "Model.inl"


} // sg


#endif // _SG_MODEL_H
