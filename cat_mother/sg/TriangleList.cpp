#include "TriangleList.h"
#include "Context.h"
#include "Shader.h"
#include "VertexLock.h"
#include "VertexFormat.h"
#include "GdUtil.h"
#include "ViewFrustum.h"
#include "BoundVolume.h"
#include "LockException.h"
#include <gd/LockMode.h>
#include <gd/VertexFormat.h>
#include <gd/GraphicsDriver.h>
#include <gd/Primitive.h>
#include <lang/Float.h>
#include <lang/Math.h>
#include <lang/Object.h>
#include <lang/Exception.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/OBBox.h>
#include <math/OBBoxBuilder.h>
#include <algorithm>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


/** Class for sorting bones affecting the vertex by importance. */
class BoneWeight
{
public:
	float	weight;
	int		index;

	bool operator<( const BoneWeight& other ) const									{return weight > other.weight;}
};

//-----------------------------------------------------------------------------

class TriangleList::TriangleListImpl :
	public Object
{
public:
	P(gd::Primitive)			mesh;
	int							vertices;
	int							maxVertices;
	gd::LockMode				vertexlock;
	float						boundSphere;
	bool						boundSphereDirty;
	OBBox						boundBox;
	bool						boundBoxDirty;
	VertexFormat				vf;
	gd::Primitive::UsageType	usage;

	TriangleListImpl( int vertexCount, const VertexFormat& vf,
		gd::Primitive::UsageType usage ) :
		mesh( 0 ),
		vertices( vertexCount ),
		maxVertices( vertexCount ),
		vertexlock( gd::LockMode::LOCK_NONE ),
		boundSphere( 0.f ),
		boundSphereDirty( true ),
		boundBox(),
		boundBoxDirty( true ),
		vf( vf ),
		usage( usage )
	{
		createMesh();
	}

	~TriangleListImpl()
	{
		destroy();
	}
	
	void destroy()
	{
		if ( mesh )
		{
			mesh->destroy();
			mesh = 0;
		}
	}

private:
	void createMesh()
	{
		assert( !mesh );

		gd::VertexFormat gdvf;
		GdUtil::togd( vf, &gdvf );

		mesh = Context::driver()->createPrimitive();
		int err = mesh->create( Context::device(), gd::Primitive::PRIMITIVE_TRIANGLELIST, maxVertices, 0, gdvf, usage );
		if ( err ) 
			throw Exception( Format("Failed to create rendering device triangle list") );

		GdUtil::tosg( mesh->vertexFormat(), &vf ); // get back actual vertex format
	}

	TriangleListImpl( const TriangleListImpl& );
	TriangleListImpl& operator=( const TriangleListImpl& );
};

//-----------------------------------------------------------------------------

/** Converts TriangleList UsageType to gd UsageType. */
static gd::Primitive::UsageType togd( TriangleList::UsageType usage )
{
	switch ( usage )
	{
	case TriangleList::USAGE_STATIC:	return gd::Primitive::USAGE_STATIC;
	case TriangleList::USAGE_DYNAMIC:	return gd::Primitive::USAGE_DYNAMIC;
	default:							return gd::Primitive::USAGE_STATIC;
	};
}

/** Converts TriangleList LockType to gd LockMode. */
static gd::LockMode togd( TriangleList::LockType lock )
{
	switch ( lock )
	{
	case TriangleList::LOCK_READ:		return gd::LockMode::LOCK_READ;
	case TriangleList::LOCK_WRITE:		return gd::LockMode::LOCK_WRITE;
	default:							return gd::LockMode::LOCK_READWRITE;
	};
}

//-----------------------------------------------------------------------------

TriangleList::TriangleList( int vertices, const VertexFormat& vf, UsageType usage )
{
	assert( vf.weights() == 0 );	// skinned TriangleList not supported currently

	m_this = new TriangleListImpl( vertices, vf, togd(usage) );
	lockDefaults();
}

TriangleList::TriangleList( const TriangleList& other, int shareFlags ) :
	Primitive( other, shareFlags )
{
	assert( !other.verticesLocked() );
	assert( shareFlags & SHARE_GEOMETRY );

	m_this = other.m_this;
	lockDefaults();
}

Primitive* TriangleList::clone( int shareFlags ) const
{
	return new TriangleList( *this, shareFlags );
}

void TriangleList::destroy()
{
	m_this = 0;
	Primitive::destroy();
}

void TriangleList::load()
{
	if ( m_this->mesh && Context::device() )
		m_this->mesh->load( Context::device() );
}

void TriangleList::unload()
{
	if ( m_this->mesh )
		m_this->mesh->unload();
}


void TriangleList::lockVertices( LockType lock )
{
	assert( m_this );

	gd::LockMode vertexlock = togd(lock);
	if ( !m_this->mesh->lockVertices( Context::device(), vertexlock ) )
		throw LockException( "TriangleList" );
	m_this->vertexlock = vertexlock;

	if ( lock == LOCK_WRITE || lock == LOCK_READWRITE )
	{
		m_this->boundSphereDirty = true;
		m_this->boundBoxDirty = true;
	}

	getLockedData();
}

void TriangleList::unlockVertices()
{
	assert( m_this );
	assert( m_this->mesh );

	m_this->mesh->unlockVertices();
	lockDefaults();
}

void TriangleList::setVertexPositionsRHW( int firstVertex, const Vector4* positions, int count )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	
	m_this->mesh->setVertexPositionsRHW( firstVertex, positions, count );
}

void TriangleList::setVertexNormals( int firstVertex, const Vector3* normals, int count )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	
	m_this->mesh->setVertexNormals( firstVertex, normals, count );
}

void TriangleList::setVertexTextureCoordinates( int firstVertex, int layer, int coordSize, const float* coord, int count )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	
	m_this->mesh->setVertexTextureCoordinates( firstVertex, layer, coordSize, coord, count );
}

void TriangleList::getVertexPositionsRHW( int firstVertex, Vector4* positions, int count ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	
	m_this->mesh->getVertexPositionsRHW( firstVertex, positions, count );
}

void TriangleList::getVertexNormals( int firstVertex, Vector3* normals, int count ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canReadVertices() );
	
	m_this->mesh->getVertexNormals( firstVertex, normals, count );
}

void TriangleList::getVertexTextureCoordinates( int firstVertex, int layer, int coordSize, float* coord, int count ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canReadVertices() );
	
	m_this->mesh->getVertexTextureCoordinates( firstVertex, layer, coordSize, coord, count );
}

int TriangleList::vertices() const
{
	assert( m_this );
	return m_this->vertices;
}

bool TriangleList::verticesLocked() const
{
	assert( m_this );
	return m_this->mesh->verticesLocked();
}

void TriangleList::draw()
{
	assert( m_this );
	assert( m_this->mesh );
	assert( shader() );

	if ( 0 == vertices() )
		return;

	gd::GraphicsDevice* dev = Context::device();
	Shader* shader = this->shader();
	const int passes = shader->begin();
	for ( int pass = 0 ; pass < passes ; ++pass )
	{
		shader->apply( pass );
		m_this->mesh->draw( dev, 0, m_this->vertices, 0, 0 );
	}
	shader->end();
}

void TriangleList::setVertexDiffuseColors( int firstVertex, const pix::Color* colors, int count )
{
	assert( m_this );
	assert( m_this->mesh );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	
	m_this->mesh->setVertexDiffuseColors( firstVertex, colors, count );
}

void TriangleList::computeVertexNormals()
{
	assert( m_this );
	assert( m_this->mesh );
	assert( verticesLocked() );
	assert( canReadVertices() );
	assert( canWriteVertices() );

	const int		count			= vertices();
	const Vector3	zero			(0,0,0);
	int				tri[3];

	for ( int i = 0 ; i < count ; i += 3 )
	{
		tri[0] = i;
		tri[1] = i+1;
		tri[2] = i+2;
		
		Vector3 v0, v1, v2;
		getVertexPositions( tri[0], &v0 );
		getVertexPositions( tri[1], &v1 );
		getVertexPositions( tri[2], &v2 );
		Vector3 e0 = v1-v0;
		Vector3 e1 = v2-v0;
		Vector3 pn = e0.cross(e1);
		float lensqr = pn.lengthSquared();
		float s = 0.f;
		if ( lensqr > Float::MIN_VALUE )
			s = 1.f / Math::sqrt(lensqr);
		pn *= s;

		for ( int k = 0 ; k < 3 ; ++k )
			setVertexNormals( tri[k], &pn );
	}
}

int TriangleList::getVertexWeights( int vertexIndex, int* boneIndices, float* boneWeights, int maxBones ) const
{
	assert( m_this );
	assert( m_this->mesh );
	assert( verticesLocked() );
	assert( canReadVertices() );
	
	return m_this->mesh->getVertexWeights( vertexIndex, boneIndices, boneWeights, maxBones );
}

void TriangleList::getVertexDiffuseColors( int firstVertex, pix::Color* colors, int count ) const
{
	assert( m_this );
	assert( m_this->mesh );
	assert( verticesLocked() );
	assert( canReadVertices() );
	
	m_this->mesh->getVertexDiffuses( firstVertex, colors, count );
}

float TriangleList::boundSphere() const
{
	assert( m_this );
	assert( !verticesLocked() );

	if ( m_this->boundSphereDirty )
	{
		if ( m_this->usage == gd::Primitive::USAGE_DYNAMIC )
		{
			m_this->boundSphere = 1e6f;
			m_this->boundSphereDirty = false;
		}
		else
		{
			const int vertices = this->vertices();
			if ( vertices < 3 )
				return 0.f;

			VertexLock<TriangleList> lock( const_cast<TriangleList*>(this), LOCK_READ );
			
			float maxr2 = 0.f;
			const int buffsize = 32;
			Vector3 buff[buffsize];

			for ( int i = 0 ; i < vertices ; )
			{
				int count = vertices-i;
				if ( count > buffsize )
					count = buffsize;

				m_this->mesh->getVertexPositions( i, buff, count );

				Vector3* v1 = buff + count;
				for ( Vector3* v = buff ; v < v1 ; ++v )
				{
					float r2 = v->lengthSquared();
					if ( r2 > maxr2 )
						maxr2 = r2;
				}
				
				i += count;
			}

			// cache results
			m_this->boundSphere = Math::sqrt( maxr2 );
			m_this->boundSphereDirty = false;
		}
	}

	return m_this->boundSphere;
}

int TriangleList::maxVertices() const
{
	assert( m_this );
	return m_this->maxVertices;
}

void TriangleList::setVertices( int count )
{
	assert( m_this );
	assert( count % 3 == 0 );
	
	if ( count > m_this->maxVertices )
		setMaxVertices( count );
	m_this->vertices = count;
}

void TriangleList::setMaxVertices( int count )
{
	assert( m_this );
	assert( count % 3 == 0 );

	if ( count > m_this->maxVertices )
	{
		m_this->maxVertices = count;
		m_this->mesh->reallocate( Context::device(), count, 0 );

		if ( verticesLocked() )
			getLockedData();
	}
}

const OBBox& TriangleList::boundBox() const
{
	if ( m_this->boundBoxDirty )
	{
		if ( m_this->usage == gd::Primitive::USAGE_DYNAMIC )
		{
			m_this->boundBox = OBBox();
			m_this->boundBox.setDimensions( Vector3(1e6f, 1e6f, 1e6f) );
			m_this->boundBoxDirty = false;
		}
		else
		{
			const int					BUFSIZE = 32;
			Vector3						v[BUFSIZE];
			VertexLock<TriangleList>	lock( const_cast<TriangleList*>(this), LOCK_READ );
			OBBoxBuilder				builder;

			while ( builder.nextPass() )
			{
				for ( int i = 0 ; i < vertices() ; )
				{
					int count = vertices() - i;
					if ( count > BUFSIZE )
						count = BUFSIZE;
					m_this->mesh->getVertexPositions( i, v, count );
					builder.addPoints( v, count );
					i += count;
				}
			}

			m_this->boundBox = builder.box();
			m_this->boundBoxDirty = false;
		}
	}
	return m_this->boundBox;
}

bool TriangleList::updateVisibility( const Matrix4x4& modelToCamera, 
	const ViewFrustum& viewFrustum )
{
	const int	BUFSIZE = 8;
	Vector3		v[BUFSIZE];

	// overlays are always visible
	if ( m_this->vf.hasRHW() )
		return true;

	// skin visibility is dependent on bones
	if ( m_this->vf.weights() > 0 )
		return false;

	int verts = boundBox().getVertices( modelToCamera, v, BUFSIZE );
	return BoundVolume::testPointsVolume( v, verts, 
		viewFrustum.planes(), ViewFrustum::PLANE_COUNT );
}

void TriangleList::lockDefaults()
{
	m_posData	= 0;
	m_posPitch	= 0;
}

void TriangleList::getLockedData()
{
	m_this->mesh->getVertexPositionData( &m_posData, &m_posPitch );
}

VertexFormat TriangleList::vertexFormat() const
{
	return m_this->vf;
}

bool TriangleList::canReadVertices() const
{
	assert( m_this );
	return m_this->vertexlock.canRead();
}

bool TriangleList::canWriteVertices() const
{
	assert( m_this );
	return m_this->vertexlock.canWrite();
}

void TriangleList::setBoundSphere( float r )
{
	m_this->boundSphere = r;
	m_this->boundSphereDirty = false;
}

void TriangleList::getVertexPositionData( float** data, int* pitch )
{
	assert( canReadVertices() || canWriteVertices() );
	*data = m_posData;
	*pitch = m_posPitch;
}


} // sg