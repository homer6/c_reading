#include "StdAfx.h"
#include "Sort.h"
#include "toString.h"
#include "Dx9Primitive.h"
#include "Dx9GraphicsDevice.h"
#include "error.h"
#include <gd/Errors.h>
#include <gd/LockMode.h>
#include <gd/VertexFormat.h>
#include <pix/Color.h>
#include <math/Vector3.h>
#include <float.h>
#include <string.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace gd;
using namespace pix;
using namespace math;

//-----------------------------------------------------------------------------

Dx9Primitive::Dx9Primitive() :
	m_refs(0)
{
	m_type				= PRIMITIVE_UNDEFINED;
	m_usage				= USAGE_STATIC;

	m_vertexSize		= 0;
	m_vertexUsageFlags	= 0;
	m_vertices			= 0;
	m_vertexData		= 0;
	m_verticesLocked	= false;
	m_verticesDirty		= true;
	m_vb				= 0;

	m_indexSize			= 0;
	m_indexUsageFlags	= 0;
	m_indices			= 0;
	m_indexData			= 0;
	m_indicesLocked		= false;
	m_indicesDirty		= true;
	m_ib				= 0;
}

Dx9Primitive::~Dx9Primitive() 
{
	destroy();
}

void Dx9Primitive::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx9Primitive::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx9Primitive::destroy()
{
	destroyDeviceObject();

	if ( m_vertexData )
	{
		delete[] m_vertexData;
		m_vertexData = 0;
	}
	
	if ( m_indexData )
	{
		delete[] m_indexData;
		m_indexData = 0;
	}
}

void Dx9Primitive::destroyDeviceObject()
{
	assert( !verticesLocked() || m_usage == USAGE_STATIC );
	assert( !indicesLocked() || m_usage == USAGE_STATIC );

	if ( m_vb )
	{
		m_vb->Release();
		m_vb = 0;
		m_verticesDirty = true;
	}

	if ( m_ib )
	{
		m_ib->Release();
		m_ib = 0;
		m_indicesDirty = true;
	}
}

int Dx9Primitive::create( GraphicsDevice* device, PrimitiveType type, 
	int vertices, int indices, const VertexFormat& format, UsageType usage ) 
{
	assert( usage == USAGE_STATIC || device );	// dynamic primitive requires device
	assert( usage == USAGE_STATIC || format.weights() == 0 ); // dynamic primitive cannot be skinned
	assert( vertices > 0 );

	destroy();

	if ( usage == USAGE_DYNAMIC && !device )
	{
		error( "Cannot create dynamic primitive without rendering device" );
		return ERROR_GENERIC;
	}

	const int	vertexSize	= getLocalVertexSize( format );
	const int	indexSize	= sizeof(uint16_t);

	m_type				= type;
	m_usage				= usage;
	m_vertexSize		= vertexSize;
	m_vertices			= vertices;
	m_vertexData		= 0;
	m_vertexFormat		= format;
	m_verticesDirty		= true;
	m_verticesLocked	= false;
	m_indexSize			= indexSize;
	m_indices			= indices;
	m_indexData			= 0;
	m_indicesDirty		= true;
	m_indicesLocked		= false;

	if ( usage == USAGE_STATIC )
	{
		if ( vertices > 0 )
		{
			m_vertexData = new BYTE[ vertexSize * vertices ];
			memset( m_vertexData, 0, vertexSize*vertices );
		}

		if ( indices > 0 )
		{
			m_indexData = new BYTE[ indexSize * indices ];
			memset( m_indexData, 0, indexSize*indices );
		}
	}
	else // ( usage == USAGE_DYNAMIC )
	{
		load( device );
	}

	return ERROR_NONE;
}

void Dx9Primitive::reallocate( GraphicsDevice* device, int vertices, int indices )
{
	assert( indices % 3 == 0 );

	destroyDeviceObject();

	if ( m_usage == USAGE_STATIC )
	{
		// allocate new vertex data
		if ( vertices > m_vertices )
		{
			int oldDataSize = m_vertexSize * m_vertices;
			int newDataSize = m_vertexSize * vertices;
			BYTE* vertexData = new BYTE[ newDataSize ];
			
			// copy old data to new data
			int copySize = oldDataSize;
			if ( copySize > newDataSize )
				copySize = newDataSize;
			if ( copySize > 0 && m_vertexData )
				memcpy( vertexData, m_vertexData, copySize );

			// zero rest of the new data
			int zeroSize = newDataSize - oldDataSize;
			if ( zeroSize > 0 )
				memset( vertexData + oldDataSize, 0, zeroSize );

			// free old data
			if ( m_vertexData )
				delete[] m_vertexData;
			m_vertexData = vertexData;
			m_vertices = vertices;
		}

		// allocate new index data
		if ( indices > m_indices )
		{
			int oldDataSize = m_indexSize * m_indices;
			int newDataSize = m_indexSize * indices;
			BYTE* indexData = new BYTE[ newDataSize ];
			
			// copy old data to new data
			int copySize = oldDataSize;
			if ( copySize > newDataSize )
				copySize = newDataSize;
			if ( copySize > 0 && m_indexData )
				memcpy( indexData, m_indexData, copySize );

			// zero rest of the new data
			int zeroSize = newDataSize - oldDataSize;
			if ( zeroSize > 0 )
				memset( indexData + oldDataSize, 0, zeroSize );

			// free old data
			if ( m_indexData )
				delete[] m_indexData;

			m_indexData = indexData;
			m_indices = indices;
		}
	}
	else // ( usage == USAGE_DYNAMIC )
	{
		m_vertices	= vertices;
		m_indices	= indices;

		load( device );
	}
}

void Dx9Primitive::loadVertexBuffer( GraphicsDevice* device )
{
	Dx9GraphicsDevice* dev = static_cast<Dx9GraphicsDevice*>( device );

	if ( m_vb && m_vertexData && 0 == (m_vertexUsageFlags & D3DUSAGE_DYNAMIC) )
	{
		BYTE*	data		= 0;
		int		vsize		= dev->getDeviceVertexSize( m_vertexFormat );
		int		dataSize	= vsize * m_vertices;
		DWORD	lockFlags	= 0;

		HRESULT hr = m_vb->Lock( 0, dataSize, (void**)&data, lockFlags );
		if ( D3D_OK != hr )
		{
			error( "Failed to lock rendering device vertex buffer." );
			return;
		}

		//memcpy( data, m_vertexData, dataSize );
		copyVertexData( dev, data, vsize, dataSize );

		m_vb->Unlock();
		m_verticesDirty = false;
		static_cast<Dx9GraphicsDevice*>(device)->updateLockStatistics( m_vertices, 0 );
	}
}

void Dx9Primitive::loadIndexBuffer( GraphicsDevice* device )
{
	if ( m_ib && m_indexData )
	{
		BYTE*	data		= 0;
		int		dataSize	= m_indexSize * m_indices;

		DWORD lockFlags = 0;
		if ( m_indexUsageFlags & D3DUSAGE_DYNAMIC )
			lockFlags = D3DLOCK_DISCARD;

		HRESULT hr = m_ib->Lock( 0, dataSize, (void**)&data, lockFlags );
		if ( D3D_OK != hr )
		{
			error( "Failed to lock rendering device index buffer." );
			return;
		}

		memcpy( data, m_indexData, dataSize );

		m_ib->Unlock();
		m_indicesDirty = false;

		static_cast<Dx9GraphicsDevice*>(device)->updateLockStatistics( 0, m_indices );
	}
}

void Dx9Primitive::load( GraphicsDevice* device )
{
	if ( !device )
	{
		error( "Cannot load() primitive without rendering device" );
		return;
	}

	Dx9GraphicsDevice*	dev			= static_cast<Dx9GraphicsDevice*>(device);
	IDirect3DDevice9*	d3ddev		= dev->d3dDevice();

	// vertex usage flags
	int vertexUsageFlags = D3DUSAGE_WRITEONLY;
	if ( USAGE_DYNAMIC == m_usage )
		vertexUsageFlags |= D3DUSAGE_DYNAMIC;
	if ( m_vertexFormat.weights() > 0 && dev->vertexProcessing() != Dx9GraphicsDevice::VERTEXP_HW )
		vertexUsageFlags |= D3DUSAGE_SOFTWAREPROCESSING;
	if ( vertexUsageFlags != m_vertexUsageFlags && m_vb )
	{
		m_vb->Release();
		m_vb = 0;
	}
	m_vertexUsageFlags = vertexUsageFlags;

	// create vertex buffer if none
	if ( !m_vb )
	{
		UINT length = dev->getDeviceVertexSize(m_vertexFormat) * m_vertices;
		DWORD fvf = 0;
		HRESULT hr = d3ddev->CreateVertexBuffer( length, m_vertexUsageFlags, fvf, D3DPOOL_DEFAULT, &m_vb, 0 );
		if ( D3D_OK != hr )
		{
			error( "Failed to create rendering device vertex buffer." );
			return;
		}

		m_verticesDirty = true;
	}

	// refresh dirty vertex buffer
	if ( m_verticesDirty )
		loadVertexBuffer( device );

	if ( m_indices > 0 )
	{
		// index usage flags
		bool polygonSorting = dev->renderingState().polygonSorting != 0;
		int indexUsageFlags = vertexUsageFlags;
		if ( polygonSorting )
			indexUsageFlags |= D3DUSAGE_DYNAMIC;
		if ( indexUsageFlags != m_indexUsageFlags && m_ib )
		{
			m_ib->Release();
			m_ib = 0;
		}
		m_indexUsageFlags = indexUsageFlags;

		// create index buffer if none
		if ( !m_ib )
		{
			UINT		length	= m_indexSize * m_indices;
			D3DFORMAT	fmt		= D3DFMT_INDEX16;

			HRESULT hr = d3ddev->CreateIndexBuffer( length, m_indexUsageFlags, fmt, D3DPOOL_DEFAULT, &m_ib, 0 );
			if ( D3D_OK != hr )
			{
				error( "Failed to create rendering device index buffer." );
				return;
			}

			m_indicesDirty = true;
		}

		// refresh dirty index buffer
		if ( m_indexData && (m_indicesDirty || polygonSorting) )
		{
			// sort polys if needed
			if ( polygonSorting )
			{
				// TODO: does not work if partial index buffer is used?

				//Profile pr( "sorting" );
				assert( sizeof(int) >= sizeof(float) );
				assert( sizeof(int) >= sizeof(uint16_t) );

				// sort tmp buffer format: 
				// [0,polys)						[out] polys in order (int)
				// [polys,polys*2)					[in] distances to polys (float)
				// [polys,polys*2)					reorder buffer (used after sort)
				// [polys*2,polys*4)				temporary buffer for bucket sort
				// [polys*4,polys*4+verts*vecsize]	vertex data buffer
				//
				const int polys = m_indices/3;
				const int capacity = polys + polys + polys*2 + sizeof(Vector3)*(m_vertices+1);
				int* tmp = dev->getTemporaryIntegerBuffer( capacity );
				int*		sorted		= tmp;
				float*		distances	= reinterpret_cast<float*>( tmp + polys );
				uint16_t*	reorder		= reinterpret_cast<uint16_t*>( tmp + polys );	// used after sort
				int*		sortbuf		= tmp + polys*2;
				Vector3*	vertbuf		= reinterpret_cast<Vector3*>(tmp + polys*4);
				#ifdef _DEBUG
				assert( (reinterpret_cast<uint8_t*>(vertbuf)-reinterpret_cast<uint8_t*>(tmp)) % sizeof(Vector3) == 0 );
				int*		end			= reinterpret_cast<int*>(vertbuf + m_vertices);
				assert( end-tmp <= capacity );
				#endif

				// compute view space vertices
				const Matrix4x4* worldTMs = dev->worldTransforms();
				const int worldTMCount = dev->worldTransformCount();
				Matrix4x4 viewtm;
				dev->getViewTransform( &viewtm );
				lockVertices( dev, LockMode::LOCK_READ );
				getTransformedVertexPositions( worldTMs, worldTMCount, viewtm, vertbuf, m_vertices, false );
				unlockVertices();

				// compute polygon distances to the camera
				uint16_t* index16 = reinterpret_cast<uint16_t*>( m_indexData );
				int index = 0;
				for ( int i = 0 ; i < polys ; ++i )
				{
					int vi0 = index16[index];
					int vi1 = index16[index+1];
					int vi2 = index16[index+2];
					
					assert( vi0 >= 0 && vi0 < m_vertices );
					assert( vi1 >= 0 && vi1 < m_vertices );
					assert( vi2 >= 0 && vi2 < m_vertices );

					// closest Z
					/*float d = vertbuf[vi0].z;
					if ( vertbuf[vi1].z < d )
						d = vertbuf[vi1].z;
					if ( vertbuf[vi2].z < d )
						d = vertbuf[vi2].z;
					distances[i] = d;*/

					// closest distance
					/*float d0 = vertbuf[vi0].length();
					float d1 = vertbuf[vi1].length();
					float d2 = vertbuf[vi2].length();
					float d = d0;
					if ( d1 < d )
						d = d1;
					if ( d2 < d )
						d = d2;
					distances[i] = d;*/

					// average distance (BEST)
					float d0 = vertbuf[vi0].length();
					float d1 = vertbuf[vi1].length();
					float d2 = vertbuf[vi2].length();
					float d = (d0 + d1 + d2) * (1.f/3.f);
					distances[i] = d;

					index += 3;
				}

				//Sort::bucketSort( distances, distances+polys, sortbuf, sorted );
				Sort::stdSort( distances, distances+polys, sortbuf, sorted );

				// reorder polygons
				index = 0;
				for ( int i = 0 ; i < polys ; ++i )
				{
					int oldIndex = (unsigned)sorted[i] * 3U;
					assert( oldIndex >= 0 && oldIndex < m_indices );
					
					reorder[index] = index16[oldIndex];
					reorder[index+1] = index16[oldIndex+1];
					reorder[index+2] = index16[oldIndex+2];
					
					index += 3;
				}
				int k = m_indices - 3;
				for ( int i = 0 ; i < m_indices ; i += 3 )
				{
					index16[k] = reorder[i];
					index16[k+1] = reorder[i+1];
					index16[k+2] = reorder[i+2];
					k -= 3;

					assert( index16[i] >= 0 && index16[i] < m_vertices );
					assert( index16[i+1] >= 0 && index16[i+1] < m_vertices );
					assert( index16[i+2] >= 0 && index16[i+2] < m_vertices );
				}
			}

			loadIndexBuffer( device );
		}
	}
}

void Dx9Primitive::unload()
{
	destroyDeviceObject();
}

bool Dx9Primitive::lockVertices( GraphicsDevice* device, const LockMode& mode ) 
{
	assert( !verticesLocked() );
	assert( !mode.canRead() || m_usage == USAGE_STATIC );

	Dx9GraphicsDevice* dev = static_cast<Dx9GraphicsDevice*>( device );

	if ( m_usage == USAGE_DYNAMIC )
	{
		if ( !m_vb )
			load( device );

		int	dataSize = dev->getDeviceVertexSize(m_vertexFormat) * m_vertices;
		HRESULT hr = m_vb->Lock( 0, dataSize, (void**)&m_vertexData, D3DLOCK_DISCARD );
		if ( D3D_OK != hr )
		{
			error( "Failed to lock rendering device vertex buffer." );
			return false;
		}

		#ifdef _DEBUG
		memset( m_vertexData, 0xAB, dataSize );
		#endif

		static_cast<Dx9GraphicsDevice*>(device)->updateLockStatistics( m_vertices, 0 );
	}

	if ( mode.canWrite() )
		m_verticesDirty	= true;

	m_verticesLocked = true;
	return true;
}

void Dx9Primitive::unlockVertices() 
{
	m_verticesLocked = false;

	if ( m_usage == USAGE_DYNAMIC )
	{
		m_vb->Unlock();
		m_vertexData = 0;
	}
}

bool Dx9Primitive::lockIndices( GraphicsDevice* device, const LockMode& mode ) 
{
	assert( !indicesLocked() );

	if ( m_usage == USAGE_DYNAMIC )
	{
		if ( !m_ib )
			load( device );

		int dataSize = m_indexSize * m_indices;
		HRESULT hr = m_ib->Lock( 0, dataSize, (void**)&m_indexData, D3DLOCK_DISCARD );
		if ( D3D_OK != hr )
		{
			error( "Failed to lock rendering device index buffer." );
			return false;
		}

		#ifdef _DEBUG
		memset( m_indexData, 0xAB, dataSize );
		#endif

		static_cast<Dx9GraphicsDevice*>(device)->updateLockStatistics( 0, m_indices );
	}

	if ( mode.canWrite() )
		m_indicesDirty = true;

	m_indicesLocked = true;
	return true;
}

void Dx9Primitive::unlockIndices() 
{
	m_indicesLocked = false;

	if ( m_usage == USAGE_DYNAMIC )
	{
		m_ib->Unlock();
		m_indexData = 0;
	}
}

void Dx9Primitive::draw( GraphicsDevice* device ) 
{
	draw( device, 0, vertices(), 0, indices() );
}

void Dx9Primitive::draw( GraphicsDevice* device, int firstVertex, int vertices, 
	int firstIndex, int indices ) 
{
	assert( !verticesLocked() );
	assert( !indicesLocked() );

	Dx9GraphicsDevice*	dev			= static_cast<Dx9GraphicsDevice*>(device);
	IDirect3DDevice9*	d3ddev		= dev->d3dDevice();

	// ignore request if rendering has been disabled
	if ( !dev->rendering() )
		return;

	// upload if needed
	if ( //USAGE_DYNAMIC != m_usage &&
		(m_verticesDirty || m_indicesDirty || dev->renderingState().polygonSorting) )
		load( device );
	if ( !m_vb )
		return;

	// set vertex stream
	HRESULT hr = d3ddev->SetStreamSource( 0, m_vb, 0, dev->getDeviceVertexSize(m_vertexFormat) );
	if ( D3D_OK != hr )
		error( "Failed to set vertex stream." );
	
	// draw indexed or non-indexed primitive
	int triangles = 0;
	if ( m_ib )
	{
		if ( indices > 0 )
		{
			// draw indexed primitive
			hr = d3ddev->SetIndices( m_ib );
			if ( D3D_OK != hr )
				error( "Failed to set index stream." );

			switch ( m_type )
			{
			case PRIMITIVE_INDEXEDTRIANGLELIST:
				triangles = indices / 3;
				hr = d3ddev->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, firstVertex, 0, vertices, firstIndex, triangles );
				break;
			}

			if ( D3D_OK != hr )
				error( "Failed to draw indexed primitive: %s", toString(hr) );
		}
	}
	else
	{
		if ( vertices > 0 )
		{
			// draw non-indexed primitive
			switch ( m_type )
			{
			case PRIMITIVE_TRIANGLELIST:
				triangles = vertices / 3;
				hr = d3ddev->DrawPrimitive( D3DPT_TRIANGLELIST, firstVertex, triangles );
				break;

			case PRIMITIVE_LINELIST:
				triangles = vertices / 2;
				hr = d3ddev->DrawPrimitive( D3DPT_LINELIST, firstVertex, triangles );
				break;
			}

			if ( D3D_OK != hr )
				error( "Failed to draw non-indexed primitive: %s", toString(hr) );
		}
	}

	dev->updateStatistics( triangles );
}

int Dx9Primitive::vertices() const 
{
	return m_vertices;
}

int Dx9Primitive::indices() const 
{
	return m_indices;
}

bool Dx9Primitive::verticesLocked() const 
{
	return m_verticesLocked;
}

bool Dx9Primitive::indicesLocked() const 
{
	return m_indicesLocked;
}

void Dx9Primitive::setVertexPositions( int firstVertex, const Vector3* positions, int count ) 
{
	assert( verticesLocked() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 0U;

	for ( ; count > 0 ; --count )
	{
		float* vf = reinterpret_cast<float*>(v);
		vf[0] = positions->x;
		vf[1] = positions->y;
		vf[2] = positions->z;
		++positions;
		v += m_vertexSize;
	}
}

void Dx9Primitive::setVertexPositionsRHW( int firstVertex, const Vector4* positions, int count ) 
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasRHW() );
	assert( !m_vertexFormat.hasNormal() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 0U;

	for ( ; count > 0 ; --count )
	{
		float* vf = reinterpret_cast<float*>(v);
		vf[0] = positions->x;
		vf[1] = positions->y;
		vf[2] = positions->z;
		vf[3] = positions->w;
		++positions;
		v += m_vertexSize;
	}
}

void Dx9Primitive::setVertexNormals( int firstVertex, const Vector3* normals, int count ) 
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasNormal() );
	assert( !m_vertexFormat.hasRHW() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0);

	for ( ; count > 0 ; --count )
	{
		float* vf = reinterpret_cast<float*>(v);
		vf[0] = normals->x;
		vf[1] = normals->y;
		vf[2] = normals->z;
		++normals;
		v += m_vertexSize;
	}
}

void Dx9Primitive::setVertexDiffuseColors( int firstVertex, const pix::Color* colors, int count ) 
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasDiffuse() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.hasRHW() ? 4 : 0) +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0) + 
		(m_vertexFormat.hasNormal() ? 12U : 0U);

	for ( ; count > 0 ; --count )
	{
		DWORD* vd = reinterpret_cast<DWORD*>(v);
		vd[0] = colors->toInt32();
		++colors;
		v += m_vertexSize;
	}
}

void Dx9Primitive::setVertexSpecularColors( int firstVertex, const pix::Color* colors, int count ) 
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasSpecular() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.hasRHW() ? 4 : 0) +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0) + 
		(m_vertexFormat.hasNormal() ? 12U : 0U) +
		(m_vertexFormat.hasDiffuse() ? 4U : 0U);

	for ( ; count > 0 ; --count )
	{
		DWORD* vd = reinterpret_cast<DWORD*>(v);
		vd[0] = colors->toInt32();
		++colors;
		v += m_vertexSize;
	}
}

void Dx9Primitive::setVertexTextureCoordinates( int firstVertex, int layer, int coordSize, const float* coord, int count ) 
{
	assert( verticesLocked() );
	assert( layer < m_vertexFormat.textureCoordinates() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	assert( coordSize == m_vertexFormat.getTextureCoordinateSize(layer) );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0) + 
		(m_vertexFormat.hasRHW() ? 4 : 0) +
		(m_vertexFormat.hasNormal() ? 12U : 0U) +
		(m_vertexFormat.hasDiffuse() ? 4U : 0U) +
		(m_vertexFormat.hasSpecular() ? 4U : 0U);

	for ( int i = 0 ; i < layer ; ++i )
		v += sizeof(float) * (unsigned)m_vertexFormat.getTextureCoordinateSize(i);

	for ( ; count > 0 ; --count )
	{
		float* vf = reinterpret_cast<float*>(v);
		
		for ( int i = 0 ; i < coordSize ; ++i )
			vf[i] = *coord++;

		v += m_vertexSize;
	}
}

void Dx9Primitive::setVertexWeights( int vertexIndex, const int* boneIndices, const float* boneWeights, int bones )
{
	assert( verticesLocked() );
	assert( vertexIndex >= 0 && vertexIndex < m_vertices );
	assert( m_vertexFormat.weights() > 0 );
	assert( !m_vertexFormat.hasRHW() );

	BYTE*		v					= m_vertexData + (unsigned)vertexIndex * (unsigned)m_vertexSize + 12U;
	const int	weights				= m_vertexFormat.weights();
	DWORD		combinedBoneIndex	= 0;
	int			boneShift			= 0;
	// weights=1 => {w0,u4}, 2 bones
	// weights=2 => {w0,w1,u4}, 3 bones
	// weights=3 => {w0,w1,w2,u4}, 4 bones

	// normalize weights
	float sumw = 0.f;
	int i;
	for ( i = 0 ; i <= weights ; ++i )
	{
		if ( i < bones )
			sumw += boneWeights[i];
	}
	float invsumw = 1.f;
	if ( sumw > FLT_MIN )
		invsumw = 1.f / sumw;

	for ( i = 0 ; i <= weights ; ++i )
	{
		DWORD	boneIndex	= 0;
		float	boneWeight	= 0.f;

		if ( i < bones )
		{
			boneIndex		= boneIndices[i];
			boneWeight		= boneWeights[i] * invsumw;
		}

		combinedBoneIndex	|= ( (0xFF & boneIndex) << boneShift );
		boneShift			+= 8;

		if ( i < weights )
			reinterpret_cast<float*>(v)[i] = boneWeight;
	}

	if ( weights > 0 ) 
		reinterpret_cast<DWORD*>(v)[weights] = combinedBoneIndex;
}

void Dx9Primitive::getVertexPositions( int firstVertex, Vector3* positions, int count ) const 
{
	assert( verticesLocked() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize;

	for ( ; count > 0 ; --count )
	{
		float* vf = reinterpret_cast<float*>(v);
		positions->x = vf[0];
		positions->y = vf[1];
		positions->z = vf[2];
		++positions;
		v += m_vertexSize;
	}
}

void Dx9Primitive::getVertexPositionsRHW( int firstVertex, Vector4* positions, int count ) const
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasRHW() );
	assert( !m_vertexFormat.hasNormal() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	const BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 0U;

	for ( ; count > 0 ; --count )
	{
		const float* vf = reinterpret_cast<const float*>(v);
		positions->x = vf[0];
		positions->y = vf[1];
		positions->z = vf[2];
		positions->w = vf[3];
		++positions;
		v += m_vertexSize;
	}
}

void Dx9Primitive::getVertexNormals( int firstVertex, Vector3* normals, int count ) const 
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasNormal() );
	assert( !m_vertexFormat.hasRHW() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0);

	for ( ; count > 0 ; --count )
	{
		float* vf = reinterpret_cast<float*>(v);
		normals->x = vf[0];
		normals->y = vf[1];
		normals->z = vf[2];
		++normals;
		v += m_vertexSize;
	}
}

void Dx9Primitive::getVertexDiffuses( int firstVertex, pix::Color* colors, int count ) const 
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasDiffuse() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.hasRHW() ? 4 : 0) +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0) + 
		(m_vertexFormat.hasNormal() ? 12U : 0U);

	for ( ; count > 0 ; --count )
	{
		DWORD* vd = reinterpret_cast<DWORD*>(v);
		*colors = Color(*vd);
		++colors;
		v += m_vertexSize;
	}
}

void Dx9Primitive::getVertexSpeculars( int firstVertex, pix::Color* colors, int count ) const 
{
	assert( verticesLocked() );
	assert( m_vertexFormat.hasSpecular() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.hasRHW() ? 4 : 0) +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0) + 
		(m_vertexFormat.hasNormal() ? 12U : 0U) +
		(m_vertexFormat.hasDiffuse() ? 4U : 0U);

	for ( ; count > 0 ; --count )
	{
		DWORD* vd = reinterpret_cast<DWORD*>(v);
		*colors = Color(*vd);
		++colors;
		v += m_vertexSize;
	}
}

void Dx9Primitive::getVertexTextureCoordinates( int firstVertex, int layer, int coordSize, float* coord, int count ) const 
{
	assert( verticesLocked() );
	assert( layer < m_vertexFormat.textureCoordinates() );
	assert( firstVertex >= 0 && firstVertex < m_vertices );
	assert( firstVertex+count <= m_vertices );
	assert( coordSize == m_vertexFormat.getTextureCoordinateSize(layer) );
	
	BYTE* v = m_vertexData + (unsigned)firstVertex * (unsigned)m_vertexSize + 12U +
		(m_vertexFormat.weights() > 0 ? sizeof(float) * (unsigned)(1+m_vertexFormat.weights()) : 0) + 
		(m_vertexFormat.hasRHW() ? 4 : 0) +
		(m_vertexFormat.hasNormal() ? 12U : 0U) +
		(m_vertexFormat.hasDiffuse() ? 4U : 0U) +
		(m_vertexFormat.hasSpecular() ? 4U : 0U);

	for ( int i = 0 ; i < layer ; ++i )
		v += sizeof(float) * (unsigned)m_vertexFormat.getTextureCoordinateSize(i);

	for ( ; count > 0 ; --count )
	{
		float* vf = reinterpret_cast<float*>(v);
		
		for ( int i = 0 ; i < coordSize ; ++i )
			*coord++ = vf[i];

		v += m_vertexSize;
	}
}

int	Dx9Primitive::getVertexWeights( int vertexIndex, int* boneIndices, float* boneWeights, int maxBones ) const
{
	assert( verticesLocked() );
	assert( vertexIndex >= 0 && vertexIndex < m_vertices );
	assert( m_vertexFormat.weights() > 0 );
	assert( !m_vertexFormat.hasRHW() );

	BYTE*		v					= m_vertexData + (unsigned)vertexIndex * (unsigned)m_vertexSize + 12U;
	const int	weights				= m_vertexFormat.weights();
	DWORD		combinedBoneIndex	= reinterpret_cast<const DWORD*>(v)[weights];
	int			boneShift			= 0;
	float		sumWeight			= 1.f;

	// weights=1 => {w0,u4}, 2 bones
	// weights=2 => {w0,w1,u4}, 3 bones
	// weights=3 => {w0,w1,w2,u4}, 4 bones

	int i;
	for ( i = 0 ; i <= weights ; ++i )
	{
		DWORD	boneIndex	= ( 0xFF & (combinedBoneIndex>>boneShift) );
		float	boneWeight	= sumWeight;

		if ( i < weights )
		{
			boneWeight = reinterpret_cast<const float*>(v)[i];
			sumWeight -= boneWeight;
		}

		if ( i < maxBones )
		{
			boneIndices[i] = (int)boneIndex;
			boneWeights[i] = boneWeight;
		}

		boneShift += 8;
	}

	return i;
}

void Dx9Primitive::setIndices( int firstIndex, const int* vertices, int count ) 
{
	assert( indicesLocked() );
	assert( firstIndex >= 0 && firstIndex < m_indices );
	assert( firstIndex+count <= m_indices );

	uint16_t* index16 = reinterpret_cast<uint16_t*>( m_indexData );
	int i = 0;
	for ( ; count > 0 ; --count )
	{
		int index = vertices[i];
		if ( index >= 0 && index < m_vertices )
			index16[firstIndex+i] = (uint16_t)index;
		++i;
	}
}

void Dx9Primitive::getIndices( int firstIndex, int* vertices, int count ) const 
{
	assert( indicesLocked() );
	assert( firstIndex >= 0 && firstIndex < m_indices );
	assert( firstIndex+count <= m_indices );

	uint16_t* index16 = reinterpret_cast<uint16_t*>( m_indexData );
	int i = 0;
	for ( ; count > 0 ; --count )
	{
		vertices[i] = index16[firstIndex+i];
		++i;
	}
}

const VertexFormat&	Dx9Primitive::vertexFormat() const 
{
	return m_vertexFormat;
}

void Dx9Primitive::getVertexPositionData( float** data, int* pitch )
{
	assert( verticesLocked() );

	BYTE* v = m_vertexData;
	*data = reinterpret_cast<float*>(v);
	*pitch = (unsigned)m_vertexSize / sizeof(float);
}

void Dx9Primitive::getIndexData( void** data, int* indexSize )
{
	assert( indicesLocked() );
	
	*data = m_indexData;
	*indexSize = m_indexSize;
}

void Dx9Primitive::getTransformedVertexPositions( const Matrix4x4* tm, int tmcount,
	const Matrix4x4& posttm, Vector3* v, int vcount, bool mostSignifigantBoneOnly ) const
{
	assert( vcount <= m_vertices );
	assert( verticesLocked() );
	assert( tmcount >= 0 ); tmcount = tmcount;

	float* vpos;
	int vpitch;
	const_cast<Dx9Primitive*>(this)->getVertexPositionData( &vpos, &vpitch );

	if ( m_vertexFormat.weights() > 0 )
	{
		const int weights = m_vertexFormat.weights();
		const int bones = (weights > 0 ? weights + 1 : 0);
		Vector3 v1;

		if ( mostSignifigantBoneOnly )
		{
			const int boneIndexOffset = 3 + weights;
			for ( int i = 0 ; i < vcount ; ++i )
			{
				DWORD combinedBoneIndex = *reinterpret_cast<const DWORD*>(vpos + boneIndexOffset);

				// get bone index
				int boneIndex = combinedBoneIndex & 0xFF;
				assert( boneIndex >= 0 && boneIndex < tmcount );

				// transform vertex with bone tm
				const Matrix4x4& m = tm[boneIndex];
				v1.x = (m(0,0)*vpos[0] + m(0,1)*vpos[1] + m(0,2)*vpos[2] + m(0,3));
				v1.y = (m(1,0)*vpos[0] + m(1,1)*vpos[1] + m(1,2)*vpos[2] + m(1,3));
				v1.z = (m(2,0)*vpos[0] + m(2,1)*vpos[1] + m(2,2)*vpos[2] + m(2,3));

				posttm.transform( v1, v+i );
				vpos += vpitch;
			}
		}
		else
		{
			for ( int i = 0 ; i < vcount ; ++i )
			{
				float* boneWeights = vpos+3;
				DWORD combinedBoneIndex = *reinterpret_cast<const DWORD*>(boneWeights+weights);
				float sumWeight = 0.f;
				int boneShift = 0;

				v1.z = v1.y = v1.x = 0.f;
				for ( int k = 0 ; k < bones ; ++k )
				{
					// get bone index
					int boneIndex = ((combinedBoneIndex>>boneShift) & 0xFF);
					assert( boneIndex >= 0 && boneIndex < tmcount );
					boneShift += 8;

					// get bone weight
					float w;
					if ( k < weights )
					{
						w = boneWeights[k];
						sumWeight += w;
					}
					else
					{
						w = 1.f - sumWeight;
					}

					// transform weighted vertex with bone tm
					const Matrix4x4& m = tm[boneIndex];
					v1.x += (m(0,0)*vpos[0] + m(0,1)*vpos[1] + m(0,2)*vpos[2] + m(0,3))*w;
					v1.y += (m(1,0)*vpos[0] + m(1,1)*vpos[1] + m(1,2)*vpos[2] + m(1,3))*w;
					v1.z += (m(2,0)*vpos[0] + m(2,1)*vpos[1] + m(2,2)*vpos[2] + m(2,3))*w;
				}

				posttm.transform( v1, v+i );
				vpos += vpitch;
			}
		}
	}
	else
	{
		Matrix4x4 combtm = posttm * tm[0];
		for ( int i = 0 ; i < vcount ; ++i )
		{
			Vector3 v1( vpos[0], vpos[1], vpos[2] );
			combtm.transform( v1, v+i );
			vpos += vpitch;
		}
	}
}

void Dx9Primitive::copyVertices( int firstVertex, Primitive* otherPrim, int otherFirstVertex, int count )
{
	Dx9Primitive* other = static_cast<Dx9Primitive*>( otherPrim );
	
	assert( verticesLocked() );
	assert( other->verticesLocked() );
	assert( m_vertexSize == other->m_vertexSize );
	assert( firstVertex >= 0 && count >= 0 );
	assert( firstVertex+count <= m_vertices );
	assert( otherFirstVertex >= 0 && count >= 0 );
	assert( otherFirstVertex+count <= other->m_vertices );
	
	memcpy( m_vertexData+firstVertex*m_vertexSize,
		other->m_vertexData+otherFirstVertex*m_vertexSize,
		count*m_vertexSize );

	m_verticesDirty = true;
}

void Dx9Primitive::copyIndices( int firstIndex, Primitive* otherPrim, int otherFirstIndex, int count )
{
	Dx9Primitive* other = static_cast<Dx9Primitive*>( otherPrim );
	
	assert( indicesLocked() );
	assert( other->indicesLocked() );
	assert( m_indexSize == other->m_indexSize );
	assert( firstIndex >= 0 && count >= 0 );
	assert( firstIndex+count <= m_indices );
	assert( otherFirstIndex >= 0 && count >= 0 );
	assert( otherFirstIndex+count <= other->m_indices );
	
	memcpy( m_indexData+firstIndex*m_indexSize,
		other->m_indexData+otherFirstIndex*m_indexSize,
		count*m_indexSize );

	m_indicesDirty = true;
}

int Dx9Primitive::getLocalVertexSize( const VertexFormat& format ) const
{
	int size = sizeof(float)*3;	// we always have position 3-vector
	int weights = format.weights();

	if ( format.hasRHW() )
		size += sizeof(float);
	if ( format.hasNormal() )
		size += sizeof(float)*3;
	if ( format.hasDiffuse() )
		size += sizeof(float);
	if ( format.hasSpecular() )
		size += sizeof(float);
	if ( weights > 0 )
		size += sizeof(DWORD) + sizeof(float) * weights;

	int textureCoordinates = format.textureCoordinates();
	for ( int i = 0 ; i < textureCoordinates ; ++i )
	{
		int texCoordSize = format.getTextureCoordinateSize( i );
		size += sizeof(float) * texCoordSize;
	}

	return size;
}

void Dx9Primitive::copyVertexData( Dx9GraphicsDevice* dev,
	void* devData, int devVertexSize, int devDataSize ) const
{
	/*assert( firstVertex >= 0 );
	assert( count >= 0 );
	assert( firstVertex+count <= m_vertices );
	assert( (firstVertex+count)*devVertexSize <= devDataSize ); devDataSize;*/
	assert( m_vertices*devVertexSize <= devDataSize ); devDataSize;

	#ifdef _DEBUG
	memset( devData, 0, devDataSize );
	#endif

	const int FLOAT_SIZE = sizeof(float);
	const int COLOR_SIZE = sizeof(DWORD);
	uint8_t* devDataBytes = reinterpret_cast<uint8_t*>( devData );
	int dstOffsetBytes = 0;

	// copy positions
	const int positionBytes = m_vertexFormat.hasRHW() ? 4*FLOAT_SIZE : 3*FLOAT_SIZE;
	{
		uint8_t* dst = devDataBytes + dstOffsetBytes;
		const BYTE* src = m_vertexData;
		for ( int i = 0 ; i < m_vertices ; ++i )
		{
			memcpy( dst, src, positionBytes );
			dst += devVertexSize;
			src += m_vertexSize;
		}
		dstOffsetBytes += positionBytes;
	}

	// copy weights as UBYTE4 (software vertex processing only)
	const int srcWeightBytes = m_vertexFormat.weights() > 0 ? (m_vertexFormat.weights()+1)*FLOAT_SIZE : 0;
	if ( m_vertexFormat.weights() > 0 && dev->vertexProcessing() != Dx9GraphicsDevice::VERTEXP_HW )
	{
		// UBYTE4
		uint8_t* dst = devDataBytes + dstOffsetBytes;
		const BYTE* src = m_vertexData + positionBytes;
		for ( int i = 0 ; i < m_vertices ; ++i )
		{
			memcpy( dst, src, srcWeightBytes );
			dst += devVertexSize;
			src += m_vertexSize;
		}
		dstOffsetBytes += srcWeightBytes;
	}

	// copy vertex normal, diffuse color, specular color and texcoords as is
	int normalBytes = 0;
	if ( m_vertexFormat.hasNormal() )
		normalBytes += 3*FLOAT_SIZE;
	if ( m_vertexFormat.hasDiffuse() )
		normalBytes += COLOR_SIZE;
	if ( m_vertexFormat.hasSpecular() )
		normalBytes += COLOR_SIZE;
	for ( int i = 0 ; i < m_vertexFormat.textureCoordinates() ; ++i )
		normalBytes += m_vertexFormat.getTextureCoordinateSize(i) * FLOAT_SIZE;
	if ( normalBytes > 0 )
	{
		uint8_t* dst = devDataBytes + dstOffsetBytes;
		const BYTE* src = m_vertexData + positionBytes + srcWeightBytes;
		for ( int i = 0 ; i < m_vertices ; ++i )
		{
			memcpy( dst, src, normalBytes );
			dst += devVertexSize;
			src += m_vertexSize;
		}
		dstOffsetBytes += normalBytes;
	}

	// copy weights and indices as two extra layers of texcoords
	if ( m_vertexFormat.weights() > 0 )
	{
		// weights: float1-3 -> float4
		uint8_t* dst = devDataBytes + dstOffsetBytes;
		const BYTE* src = m_vertexData + positionBytes;
		const int srcWeightsPerVertex = m_vertexFormat.weights();
		const int bonesPerVertex = srcWeightsPerVertex + 1;
		for ( int i = 0 ; i < m_vertices ; ++i )
		{
			const float* srcWeights = reinterpret_cast<const float*>( src );
			float* dstWeights = reinterpret_cast<float*>( dst );
			float sumWeight = 1.f;

			int k = 0;
			for ( ; k < srcWeightsPerVertex ; ++k )
			{
				float w = srcWeights[k];
				dstWeights[k] = w;
				sumWeight -= w;
			}
			dstWeights[k++] = sumWeight;
			for ( ; k < 4 ; ++k )
				dstWeights[k] = 0.f;

			dst += devVertexSize;
			src += m_vertexSize;
		}
		dstOffsetBytes += 4*FLOAT_SIZE;

		// bones: UBYTE4 -> float4
		dst = devDataBytes + dstOffsetBytes;
		src = m_vertexData + positionBytes + m_vertexFormat.weights()*FLOAT_SIZE;
		for ( int i = 0 ; i < m_vertices ; ++i )
		{
			float* dstBones = reinterpret_cast<float*>( dst );
			DWORD ub = *reinterpret_cast<const DWORD*>( src );
			int bones[4] = { ((ub)&0xFF), ((ub>>8)&0xFF), ((ub>>16)&0xFF), ((ub>>24)&0xFF) };

			int k = 0;
			for ( ; k < bonesPerVertex ; ++k )
				dstBones[k] = (float)bones[k];
			for ( ; k < 4 ; ++k )
				dstBones[k] = 0.f;

			dst += devVertexSize;
			src += m_vertexSize;
		}
		dstOffsetBytes += 4*FLOAT_SIZE;
	}

	assert( dstOffsetBytes == devVertexSize );
}
