#include "DrvObject.h"
#include <gd/Primitive.h>
#include <gd/VertexFormat.h>
#include <stdint.h>


class Dx9GraphicsDevice;


/**
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9Primitive :
	public gd::Primitive,
	public DrvObject
{
public:
	Dx9Primitive();
	~Dx9Primitive();

	void				addReference();
	void				release();
	int					create( gd::GraphicsDevice* device, PrimitiveType type, int vertices, int indices, const gd::VertexFormat& format, UsageType usage );
	void				destroy();
	void				reallocate( gd::GraphicsDevice* device, int vertices, int indices );
	void				load( gd::GraphicsDevice* device );
	void				unload();
	void				lock( const gd::LockMode& mode );
	void				unlock();
	bool				lockVertices( gd::GraphicsDevice* device, const gd::LockMode& mode );
	void				unlockVertices();
	bool				lockIndices( gd::GraphicsDevice* device, const gd::LockMode& mode );
	void				unlockIndices();
	void				draw( gd::GraphicsDevice* device );
	void				draw( gd::GraphicsDevice* device, int firstVertex, int vertices, int firstIndex, int indices );
	void				copyVertices( int firstVertex, gd::Primitive* other, int otherFirstVertex, int count );
	void				copyIndices( int firstIndex, gd::Primitive* other, int otherFirstIndex, int count );
	void				getVertexPositionData( float** data, int* pitch );
	void				setVertexPositions( int firstVertex, const math::Vector3* positions, int count=1 );
	void				setVertexPositionsRHW( int firstVertex, const math::Vector4* positions, int count=1 );
	void				setVertexNormals( int firstVertex, const math::Vector3* normals, int count=1 );
	void				setVertexDiffuseColors( int firstVertex, const pix::Color* colors, int count=1 );
	void				setVertexSpecularColors( int firstVertex, const pix::Color* colors, int count=1 );
	void				setVertexTextureCoordinates( int firstVertex, int layer, int coordSize, const float* coord, int count=1 );
	void				setVertexWeights( int vertexIndex, const int* boneIndices, const float* boneWeights, int bones );
	void				setIndices( int firstIndex, const int* vertices, int count=1 );
	void				getIndexData( void** data, int* offset );
	void				getVertexPositions( int firstVertex, math::Vector3* positions, int count=1 ) const;
	void				getVertexPositionsRHW( int firstVertex, math::Vector4* positions, int count=1 ) const;
	void				getTransformedVertexPositions( const math::Matrix4x4* tm, int tmcount, const math::Matrix4x4& posttm, math::Vector3* v, int vcount, bool mostSignifigantBoneOnly ) const;
	void				getVertexNormals( int firstVertex, math::Vector3* normals, int count=1 ) const;
	void				getVertexDiffuses( int firstVertex, pix::Color* colors, int count=1 ) const;
	void				getVertexSpeculars( int firstVertex, pix::Color* colors, int count=1 ) const;
	void				getVertexTextureCoordinates( int firstVertex, int layer, int coordSize, float* coord, int count=1 ) const;
	int					getVertexWeights( int vertexIndex, int* boneIndices, float* boneWeights, int maxBones ) const;
	void				getIndices( int firstIndex, int* vertices, int count=1 ) const;
	int					vertices() const;
	int					indices() const;
	bool				verticesLocked() const;
	bool				indicesLocked() const;
	const gd::VertexFormat&	vertexFormat() const;

private:
	long					m_refs;
	PrimitiveType			m_type;
	UsageType				m_usage;

	int						m_vertexSize;
	int						m_vertexUsageFlags;
	int						m_vertices;
	BYTE*					m_vertexData;
	gd::VertexFormat		m_vertexFormat;
	bool					m_verticesDirty;
	bool					m_verticesLocked;
	IDirect3DVertexBuffer9*	m_vb;

	int						m_indexSize;
	int						m_indexUsageFlags;
	int						m_indices;
	BYTE*					m_indexData;
	bool					m_indicesDirty;
	bool					m_indicesLocked;
	IDirect3DIndexBuffer9*	m_ib;

	void					loadIndexBuffer( gd::GraphicsDevice* device );
	void					loadVertexBuffer( gd::GraphicsDevice* device );

	void					destroyDeviceObject();
	void					refreshBoneCount();

	/** Returns internal system-memory vertex format size in bytes. */
	int		getLocalVertexSize( const gd::VertexFormat& format ) const;

	/** Copies vertex data to device format. */
	void	copyVertexData( Dx9GraphicsDevice* dev, void* devData, int devVertexSize, int devDataSize ) const;

	Dx9Primitive( const Dx9Primitive& );
	Dx9Primitive& operator=( const Dx9Primitive& );
};
