#ifndef _SG_TRIANGLELIST_H
#define _SG_TRIANGLELIST_H


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


class VertexFormat;
class TriangleListLock;


/** 
 * Triangle list primitive. 
 * Polygons are stored as vertex triplets.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class TriangleList :
	public Primitive
{
public:
	/** 
	 * Data lock mode.
	 * @see TriangleListLock
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
	 * Creates a primitive with space allocated for specified amount of vertices. 
	 * Number of triangles defined by the vertices is 1/3 of the number of vertices.
	 */
	TriangleList( int vertices, const VertexFormat& vf,	
		UsageType usage=USAGE_STATIC );

	/** Creates  clone of other primitive. */
	TriangleList( const TriangleList& other, int shareFlags );

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
	 * Default equals to the maximum number.
	 */
	void	setVertices( int count );

	/**
	 * Grows maximum number of vertices if needed.
	 * Invalidates old device object.
	 */
	void	setMaxVertices( int count );

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

	/** Sets vertex diffuse colors. Requires that the vertices are locked. */
	void	setVertexDiffuseColors( int firstVertex, const pix::Color* colors, int count=1 );

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
	 * Computes vertex normals.
	 * Takes cross product of the last and the first edge of each vertex. 
	 * If there is less than 3 vertices in a polygon then the normal is (0,0,0).
	 * Requires that vertices are locked for reading and writing.
	 */
	void	computeVertexNormals();

	/**
	 * Returns pointer to vertex position data and offset to next vertex.
	 * Requires that the vertices are locked.
	 * For optimization use only, normally use set/getVertexPositions.
	 */
	void	getVertexPositionData( float** data, int* pitch );

	/** Sets bounding sphere radius. */
	void	setBoundSphere( float r );

	/** 
	 * Computes sphere bounding all vertex positions.
	 * Requires that the model vertices are not locked.
	 */
	float	boundSphere() const;

	/** 
	 * Computes box bounding all vertex positions.
	 * Requires that the model vertices are not locked.
	 */
	const math::OBBox& boundBox() const;

	/** Returns vertex positions. Requires that the vertices are locked. */
	void	getVertexPositions( int firstVertex, math::Vector3* positions, int count=1 ) const;

	/** 
	 * Returns vertex positions with reciprocal homogenous W. 
	 * Requires that the vertices are locked. 
	 */
	void	getVertexPositionsRHW( int firstVertex, math::Vector4* positions, int count=1 ) const;

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

	/** Returns number of active vertices in the model. */
	int		vertices() const;

	/** Returns maximum number of vertices in the model. */
	int		maxVertices() const;

	/** Returns true if vertices are locked. */
	bool	verticesLocked() const;

	/** Returns vertex format of the geometry. */
	VertexFormat vertexFormat() const;

private:
	friend class TriangleListLock;

	void	lockDefaults();
	void	getLockedData();
	bool	canReadVertices() const;
	bool	canWriteVertices() const;

	class TriangleListImpl;
	P(TriangleListImpl) m_this;

	float*	m_posData;
	int		m_posPitch;

	TriangleList();
	TriangleList( const TriangleList& other );
	TriangleList&	operator=( const TriangleList& other );
};


#include "TriangleList.inl"


} // sg


#endif // _SG_TRIANGLELIST_H
