#include "Model.h"
#include "Context.h"
#include "Edge.h"
#include "EdgeHash.h"
#include "Shader.h"
#include "GdUtil.h"
#include "VertexAndIndexLock.h"
#include "PolygonAdjacency.h"
#include "VertexFormat.h"
#include "BoundVolume.h"
#include "ViewFrustum.h"
#include "LockException.h"
#include "Material.h"
#include <gd/LockMode.h>
#include <gd/Primitive.h>
#include <gd/VertexFormat.h>
#include <gd/GraphicsDriver.h>
#include <gd/GraphicsDevice.h>
#include <dev/Profile.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/Array.h>
#include <lang/Math.h>
#include <lang/Object.h>
#include <lang/Exception.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/OBBox.h>
#include <math/OBBoxBuilder.h>
#include <util/Vector.h>
#include <util/Hashtable.h>
#include <algorithm>
#include <assert.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


const int MAX_BONES_PER_VERTEX = 32;
const int MAX_BONES_PER_PRIMITIVE = 256;

//-----------------------------------------------------------------------------

/** Class for sorting bones affecting the vertex by importance. */
class BoneWeight
{
public:
	float	weight;
	int		index;

	bool operator<( const BoneWeight& other ) const									{return weight > other.weight;}
};

/** Used in vertex tangent space computation. */
class Dot3Vertex
{
public:
	Vector3 pos;
	Vector3 normal;
	Vector3	uv;
	Vector3 s;
	Vector3 t;
    Vector3 sxt;
};

class Triangle
{
public:
	Vector3 v[3];

	/** 
	 * Returns true if triangle has specified edge. 
	 */
	bool hasEdge( const Vector3& v0, const Vector3& v1 ) const
	{
		int k = 2;
		for ( int j = 0 ; j < 3 ; k = j++ )
		{
			if ( v[k] == v0 && v[j] == v1 )
				return true;
		}
		return false;
	}
};

//-----------------------------------------------------------------------------

class Model::ModelImpl :
	public Object
{
public:
	P(gd::Primitive)					mesh;
	int									vertices;
	int									indices;
	int									maxVertices;
	int									maxIndices;
	gd::LockMode						vertexlock;
	gd::LockMode						indexlock;
	float								boundSphere;
	bool								boundSphereDirty;
	OBBox								boundBox;
	bool								boundBoxDirty;
	VertexFormat						vf;
	gd::Primitive::UsageType			usage;
	PolygonAdjacency					adj;
	float								adjZeroDistance;
	bool								weightsDirty;
	util::Vector<int>					usedBoneArray;	// update if weightsDirty

	ModelImpl( int vertexCount, int indexCount, const VertexFormat& vf,
		gd::Primitive::UsageType usage ) :
		mesh( 0 ),
		vertices( vertexCount ),
		indices( indexCount ),
		maxVertices( vertexCount ),
		maxIndices( indexCount ),
		vertexlock( gd::LockMode::LOCK_NONE ),
		indexlock( gd::LockMode::LOCK_NONE ),
		boundSphere( 0.f ),
		boundSphereDirty( true ),
		boundBox( OBBox() ),
		boundBoxDirty( true ),
		vf( vf ),
		usage( usage ),
		adj(),
		adjZeroDistance( -1.f ),
		weightsDirty( true ),
		usedBoneArray( Allocator<int>(__FILE__) )
	{
		createMesh();
	}

	~ModelImpl()
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

	bool canReadVertices() const
	{
		return mesh->verticesLocked() && vertexlock.canRead();
	}

	void refreshUsedBones()
	{
		usedBoneArray.clear();
	
		if ( vf.weights() > 0 )
		{
			bool locked = !canReadVertices();
			if ( locked )
				mesh->lockVertices( Context::device(), gd::LockMode::LOCK_READ );

			// mark used bones
			Array<bool> usedBones( MAX_BONES_PER_PRIMITIVE );
			for ( int i = 0 ; i < MAX_BONES_PER_PRIMITIVE ; ++i )
				usedBones[i] = false;
			int boneIndices[MAX_BONES_PER_VERTEX];
			float boneWeights[MAX_BONES_PER_VERTEX];
			for ( int i = 0 ; i < vertices ; ++i )
			{
				int bones = mesh->getVertexWeights( i, boneIndices, boneWeights, MAX_BONES_PER_VERTEX );
				for ( int k = 0 ; k < bones ; ++k )
				{
					if ( boneWeights[k] > 0.f )
					{
						int ix = boneIndices[k];
						assert( boneIndices[k] >= 0 && boneIndices[k] < MAX_BONES_PER_PRIMITIVE );
						usedBones[ix] = true;
					}
				}
			}
			
			// count used bones
			int usedBoneCount = 0;
			for ( int i = 0 ; i < MAX_BONES_PER_PRIMITIVE ; ++i )
			{
				if ( usedBones[i] )
					usedBoneCount += 1;
			}

			// list used bones
			usedBoneArray.setSize( usedBoneCount );
			usedBoneArray.clear();
			for ( int i = 0 ; i < MAX_BONES_PER_PRIMITIVE ; ++i )
				if ( usedBones[i] )
					usedBoneArray.add( i );

			if ( locked )
				mesh->unlockVertices();
		}

		weightsDirty = false;
	}

private:
	void createMesh()
	{
		assert( !mesh );

		gd::VertexFormat gdvf;
		GdUtil::togd( vf, &gdvf );

		mesh = Context::driver()->createPrimitive();
		int err = mesh->create( Context::device(), gd::Primitive::PRIMITIVE_INDEXEDTRIANGLELIST, maxVertices, maxIndices, gdvf, usage );
		if ( err )
			throw Exception( Format("Failed to create rendering device indexed triangle list") );

		GdUtil::tosg( mesh->vertexFormat(), &vf ); // get back actual vertex format
	}

	ModelImpl( const ModelImpl& );
	ModelImpl& operator=( const ModelImpl& );
};

//-----------------------------------------------------------------------------

/** Converts Model UsageType to gd UsageType. */
static gd::Primitive::UsageType togd( Model::UsageType usage )
{
	switch ( usage )
	{
	case Model::USAGE_STATIC:	return gd::Primitive::USAGE_STATIC;
	case Model::USAGE_DYNAMIC:	return gd::Primitive::USAGE_DYNAMIC;
	default:					return gd::Primitive::USAGE_STATIC;
	};
}

/** Converts gd UsageType to Model UsageType. */
static Model::UsageType tosg( gd::Primitive::UsageType usage )
{
	switch ( usage )
	{
	case gd::Primitive::USAGE_STATIC:	return Model::USAGE_STATIC;
	case gd::Primitive::USAGE_DYNAMIC:	return Model::USAGE_DYNAMIC;
	default:							return Model::USAGE_STATIC;
	};
}

/** Converts Model LockType to gd LockMode. */
static gd::LockMode togd( Model::LockType lock )
{
	switch ( lock )
	{
	case Model::LOCK_READ:		return gd::LockMode::LOCK_READ;
	case Model::LOCK_WRITE:		return gd::LockMode::LOCK_WRITE;
	default:					return gd::LockMode::LOCK_READWRITE;
	};
}

/**
 * Takes a vertex buffer with vertices of type Dot3Vertex.
 *
 * This finds vertices within epsilon in position and averages their tangent
 * bases to make a smooth tangent space across the model.  This is useful for
 * lathed objects or authored models which duplicate vertices along material
 * boundaries.
 * Tangent Basis must have already been computed for this to work! =)
 *
 * @param unifyNormals If true then the vertex normals are averaged too
 */
static void findAndFixDegenerateVertexBasis( Dot3Vertex* vertexData, int vertices, uint16_t* /*indexData*/, int /*indices*/, bool unifyNormals )
{
	typedef uint16_t* uint16p_t;

	assert( vertexData != 0 );

	float epsilon = 1.0e-5f;
	float x,y,z,dist;

	////////////////////////////////////////////////////////////////

	int i,j;

	////////////////////////////////////////////////////////////////
	// Sloppy, but allocate a pointer and char for each vertex
	// As overlapping vertices are found, increment their duplicate_count
	//   and allocate an array of MAX_OVERLAP vertex indices to reference
	//   which vertices overlap.

	const int MAX_OVERLAP = 50;

	uint8_t * duplicate_count = new uint8_t[ vertices ];
		// duplicate_index is array of pointers to bins.  Each bin is
		// allocated when a match is found.
	uint16_t ** duplicate_index = new uint16p_t[ vertices ]; //(uint16_t**) calloc( vertices, sizeof(uint16_t*) );

	memset( duplicate_count, 0, vertices * sizeof( uint8_t  ) );
	memset( duplicate_index, 0, vertices * sizeof( uint16_t* ) );


	// Need to search the mesh for vertices with the same spatial coordinate
	// These are vertices duplicated for lathed/wrapped objects to make a
	//   2nd set of texture coordinates at the point in order to avoid bad
	//   texture wrapping
	// In short:  For each vertex, find any other vertices that share it's 
	//   position.  "Average" the tangent space basis calculated above for
	//   these duplicate vertices.  This is not rigorous, but works well 
	//   to fix up authored models.  ** Models should not have t juntions! **

	// Check each vert with every other.  There's no reason to check
	//   j with i after doing i with j, so start j from i ( so we test
	//   1 with 2 but never 2 with 1 again).
	// This is a model pre-processing step and only done once.  For large
	//   models, compute this off-line if you have to and store the resultant
	//   data.
	// The whole thing could be made much more efficient (linked list, etc)

	bool once = true;
	for( i=0; i < vertices; i++ )
	{
		for(j=i+1; j < vertices; j++ )
		{
			x = vertexData[i].pos.x - vertexData[j].pos.x;
			y = vertexData[i].pos.y - vertexData[j].pos.y;
			z = vertexData[i].pos.z - vertexData[j].pos.z;

			dist = x*x + y*y + z*z;
			
			if( dist < epsilon )
			{
				// if i matches j and k, just record into i.  j will be 
				//  half full as it will only match k, but this is
				//  taken care of when i is processed.
				if( duplicate_count[i] == 0 )
				{
					// allocate bin
					duplicate_index[i] = new uint16_t[MAX_OVERLAP];
				}
				if( duplicate_count[i] < MAX_OVERLAP )
				{
					assert( j <= 32767 );
					duplicate_index[i][duplicate_count[i]] = (uint16_t) j;
					duplicate_count[i] ++;
				}
				else
				{
					//FDebug("Ran out of bin storage!!\n");
					if ( once ) 
					{
						Debug::printlnError( "findAndFixDegenerateVertexBasis ran out of bin storage!" );
						once = false;
					}
					//assert( false );
				}
			}
		}

		/*
		if( duplicate_count[i] > 0 )
		{
			FDebug("Vertex %d\t matches: ", i );
			for(j=0; j < duplicate_count[i]; j++ )
			{
				FDebug("%d\t", duplicate_index[i][j] );
			}
			FDebug("\n");
		}
		*/
	}

	// Now average the tangent spaces & write the new result to
	//  each duplicated vertex


	Vector3	S_temp, T_temp, SxT_temp, N_temp;


	for( i = 0; i < vertices; i++ )
	{
		// do < 10 check to not average the basis at poles of sphere or
		//  other ridiculous geometry with too many degenerate vertices

		if( duplicate_count[i] > 0 && duplicate_count[i] < 10 )
		{
			//	FDebug("Averaging vert prop at %d for %d vertices\n", i, duplicate_count[i]);

			// If there are more than 5 vertices sharing this point then
			//  the point is probably some kind of lathe axis node.  No need to
			//  process it here

			// Set accumulator to value for vertex in question

			S_temp		= vertexData[i].s;
			T_temp		= vertexData[i].t;
			SxT_temp	= vertexData[i].sxt;
			N_temp		= vertexData[i].normal;

			// add in basis vectors for all other vertices which
			//  have the same positon (found above)

			for( j=0; j < duplicate_count[i]; j++ )
			{
				S_temp		= S_temp   + vertexData[duplicate_index[i][j]].s;
				T_temp		= T_temp   + vertexData[duplicate_index[i][j]].t;
				SxT_temp	= SxT_temp + vertexData[duplicate_index[i][j]].sxt;

				N_temp		= N_temp   + vertexData[duplicate_index[i][j]].normal;
			}

			// Normalize the basis vectors
			// Note that sxt might not be perpendicular to s and t
			//  anymore.  Not absolutely necessary to re-do the 
			//  cross product.

			float S_temp_len = S_temp.length();
			if ( S_temp_len > Float::MIN_VALUE )
				S_temp *= 1.f / S_temp_len;
			float T_temp_len = T_temp.length();
			if ( T_temp_len > Float::MIN_VALUE )
				T_temp *= 1.f / T_temp_len;
			float SxT_temp_len = SxT_temp.length();
			if ( SxT_temp_len > Float::MIN_VALUE )
				SxT_temp *= 1.f / SxT_temp_len;
			float N_temp_len = N_temp.length();
			if ( N_temp_len > Float::MIN_VALUE )
				N_temp *= 1.f / N_temp_len;

			// Write the average basis to the first vertex for which
			//   the duplicates were found

			vertexData[i].s = S_temp;
			vertexData[i].t = T_temp;
			vertexData[i].sxt = SxT_temp;

			if( unifyNormals )
				vertexData[i].normal = N_temp;

			// Now write to all later vertices with the same position
				
			for(j=0; j < duplicate_count[i]; j++ )
			{
				// Set the vertices in the same position to
				//  the average basis.

				vertexData[duplicate_index[i][j]].s = S_temp;
				vertexData[duplicate_index[i][j]].t = T_temp;
				vertexData[duplicate_index[i][j]].sxt = SxT_temp;

				if( unifyNormals )
					vertexData[duplicate_index[i][j]].normal = N_temp;


				// Kill the duplicate index lists of all vertices of
				//  higher index which overlap this one.  This is so
				//  higher index vertices do not average a smaller 
				//  subset of bases.
				// Arrays are de-allocated later

				duplicate_count[ duplicate_index[i][j] ] = 0;

			}

		}

		if( duplicate_index[i] != 0 )
		{
			delete [] duplicate_index[i];
			duplicate_index[i] = 0;
			duplicate_count[i] = 0;
		}
	}

	delete [] duplicate_count;
	delete[] duplicate_index; //free( duplicate_index );
}

/**
 * Creates basis vectors, based on a vertex and index list.
 *
 * NOTE: Assumes an indexed triangle list, with vertices of type Dot3Vertex
 * Does not check for degenerate vertex positions - ie vertices with same 
 * position but different texture coords or normals.  Another function 
 * can do this to average the basis vectors & "smooth" the tangent space
 * for those duplicated vertices.
 */
static void createBasisVectors( Dot3Vertex* vertexData, uint16_t* indexData, int vertices, int indices, bool smoothNormals )
{
	int i;

	// Clear the basis vectors
	for (i = 0; i < vertices; i++)
	{
		vertexData[i].s = Vector3(0.0f, 0.0f, 0.0f);
		vertexData[i].t = Vector3(0.0f, 0.0f, 0.0f);
	}

	// Walk through the triangle list and calculate gradiants for each triangle.
	// Sum the results into the s and t components.
    for( i = 0; i < indices; i += 3 )
    {       
		int TriIndex[3];
		Vector3 du, dv;
		Vector3 edge01;
		Vector3 edge02;
		Vector3 cp;
		
		TriIndex[0] = indexData[i];
		TriIndex[1] = indexData[i+1];
		TriIndex[2] = indexData[i+2];

		assert((TriIndex[0] < vertices) && (TriIndex[1] < vertices) && (TriIndex[2] < vertices));

		Dot3Vertex& v0 = vertexData[TriIndex[0]];
		Dot3Vertex& v1 = vertexData[TriIndex[1]];
		Dot3Vertex& v2 = vertexData[TriIndex[2]];

		// x, s, t
		edge01 = Vector3( v1.pos.x - v0.pos.x, v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y );
		edge02 = Vector3( v2.pos.x - v0.pos.x, v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y );

		cp = edge01.cross( edge02 );
		if ( fabs(cp.x) > Float::MIN_VALUE )
		{
			v0.s.x += -cp.y / cp.x;
			v0.t.x += -cp.z / cp.x;

			v1.s.x += -cp.y / cp.x;
			v1.t.x += -cp.z / cp.x;
			
			v2.s.x += -cp.y / cp.x;
			v2.t.x += -cp.z / cp.x;
		}

		// y, s, t
		edge01 = Vector3( v1.pos.y - v0.pos.y, v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y );
		edge02 = Vector3( v2.pos.y - v0.pos.y, v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y );

		cp = edge01.cross( edge02 );
		if ( fabs(cp.x) > Float::MIN_VALUE )
		{
			v0.s.y += -cp.y / cp.x;
			v0.t.y += -cp.z / cp.x;

			v1.s.y += -cp.y / cp.x;
			v1.t.y += -cp.z / cp.x;
			
			v2.s.y += -cp.y / cp.x;
			v2.t.y += -cp.z / cp.x;
		}

		// z, s, t
		edge01 = Vector3( v1.pos.z - v0.pos.z, v1.uv.x - v0.uv.x, v1.uv.y - v0.uv.y );
		edge02 = Vector3( v2.pos.z - v0.pos.z, v2.uv.x - v0.uv.x, v2.uv.y - v0.uv.y );

		cp = edge01.cross( edge02 );
		if ( fabs(cp.x) > Float::MIN_VALUE )
		{
			v0.s.z += -cp.y / cp.x;
			v0.t.z += -cp.z / cp.x;

			v1.s.z += -cp.y / cp.x;
			v1.t.z += -cp.z / cp.x;
			
			v2.s.z += -cp.y / cp.x;
			v2.t.z += -cp.z / cp.x;
		}
    }

	findAndFixDegenerateVertexBasis( vertexData, vertices, indexData, indices, smoothNormals );

    // Calculate the sxt vector
  	for(i = 0; i < vertices; i++)
  	{		
  		// Normalize the s, t vectors
		float len = vertexData[i].s.length();
		//assert( len > Float::MIN_VALUE );
		if ( len > Float::MIN_VALUE )
			vertexData[i].s *= 1.f/len;
		len = vertexData[i].t.length();
		//assert( len > Float::MIN_VALUE );
		if ( len > Float::MIN_VALUE )
			vertexData[i].t *= 1.f/len;
  
  		// Get the cross of the s and t vectors
		vertexData[i].sxt = vertexData[i].s.cross( vertexData[i].t );
  
  		// Need a normalized normal
		len = vertexData[i].normal.length();
		if ( len > Float::MIN_VALUE )
			vertexData[i].normal *= 1.f/len;
    		
  		// Get the direction of the sxt vector
		if ( vertexData[i].sxt.dot(vertexData[i].normal) < 0.f )
  			vertexData[i].sxt = -vertexData[i].sxt;
  	}
}

//-----------------------------------------------------------------------------

Model::Model( int vertices, int indices, const VertexFormat& vf, UsageType usage ) :
	m_posData(0),
	m_indexData(0)
{
	m_this = new ModelImpl( vertices, indices, vf, togd(usage) );
	lockDefaults();
}

Model::Model( const Model& other, int shareFlags ) :
	Primitive( other, shareFlags ),
	m_posData(0),
	m_indexData(0)
{
	assert( !other.verticesLocked() && !other.indicesLocked() );

	if ( 0 == (shareFlags & SHARE_GEOMETRY) )
	{
		m_this = new ModelImpl( other.vertices(), other.indices(), other.vertexFormat(), togd(other.usage()) );
		lockDefaults();

		VertexAndIndexLock<Model> lkbase( const_cast<Model*>(&other), Model::LOCK_READ );
		VertexAndIndexLock<Model> lktgt( this, Model::LOCK_WRITE );

		copyVertices( 0, &other, 0, other.vertices() );
		copyIndices( 0, &other, 0, other.indices() );

		m_this->usedBoneArray = other.m_this->usedBoneArray;
		m_this->weightsDirty = other.m_this->weightsDirty;
	}
	else
	{
		m_this = other.m_this;
	}
}

Primitive* Model::clone( int shareFlags ) const
{
	return new Model( *this, shareFlags );
}

void Model::destroy()
{
	m_this = 0;
	Primitive::destroy();
}

void Model::load()
{
	if ( m_this->mesh && Context::device() )
		m_this->mesh->load( Context::device() );
}

void Model::unload()
{
	if ( m_this->mesh )
		m_this->mesh->unload();
}

void Model::lockVertices( LockType lock )
{
	assert( m_this );

	gd::LockMode vertexlock = togd(lock);
	if ( !m_this->mesh->lockVertices( Context::device(), vertexlock ) )
		throw LockException( "Model" );
	m_this->vertexlock = vertexlock;

	if ( lock == LOCK_WRITE || lock == LOCK_READWRITE )
	{
		m_this->boundSphereDirty = true;
		m_this->boundBoxDirty = true;
	}

	getLockedData();
}

void Model::unlockVertices()
{
	assert( m_this );
	assert( m_this->mesh );

	m_this->mesh->unlockVertices();
	lockDefaults();
}

void Model::lockIndices( LockType lock )
{
	assert( m_this );

	gd::LockMode indexlock = togd(lock);
	if ( !m_this->mesh->lockIndices( Context::device(), indexlock ) )
		throw LockException( "Model" );
	m_this->indexlock = indexlock;

	if ( lock != LOCK_READ )
		m_this->adjZeroDistance = -1.f;

	int indexSize;
	m_this->mesh->getIndexData( (void**)&m_indexData, &indexSize );
	assert( indexSize == 2 );
}

void Model::unlockIndices()
{
	assert( m_this );
	assert( m_this->mesh );
	m_this->mesh->unlockIndices();
	m_indexData = 0;
}

void Model::setVertexPositionsRHW( int firstVertex, const Vector4* positions, int count )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	m_this->mesh->setVertexPositionsRHW( firstVertex, positions, count );
}

void Model::setVertexNormals( int firstVertex, const Vector3* normals, int count )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	m_this->mesh->setVertexNormals( firstVertex, normals, count );
}

void Model::setVertexTextureCoordinates( int firstVertex, int layer, int coordSize, const float* coord, int count )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	m_this->mesh->setVertexTextureCoordinates( firstVertex, layer, coordSize, coord, count );
}

void Model::getVertexPositionsRHW( int firstVertex, Vector4* positions, int count ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	m_this->mesh->getVertexPositionsRHW( firstVertex, positions, count );
}

void Model::getVertexNormals( int firstVertex, Vector3* normals, int count ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canReadVertices() );
	m_this->mesh->getVertexNormals( firstVertex, normals, count );
}

void Model::getVertexTextureCoordinates( int firstVertex, int layer, int coordSize, float* coord, int count ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canReadVertices() );
	m_this->mesh->getVertexTextureCoordinates( firstVertex, layer, coordSize, coord, count );
}

int Model::vertices() const
{
	assert( m_this );
	return m_this->vertices;
}

int Model::indices() const
{
	assert( m_this );
	return m_this->indices;
}

bool Model::verticesLocked() const
{
	assert( m_this );
	return m_this->mesh->verticesLocked();
}

bool Model::indicesLocked() const
{
	assert( m_this );
	return m_this->mesh->indicesLocked();
}

void Model::draw()
{
	assert( m_this );
	assert( m_this->mesh );
	assert( !indicesLocked() && !verticesLocked() );
	assert( shader() );
	assert( vertexFormat() == shader()->vertexFormat() );

	if ( 0 == indices() )
		return;

	//dev::Profile pr( "Model.draw" );

	gd::GraphicsDevice* dev = Context::device();
	Shader*	shader = this->shader();
	const int passes = shader->begin();
	for ( int pass = 0 ; pass < passes ; ++pass )
	{
		shader->apply( pass );
		m_this->mesh->draw( dev, 0, m_this->vertices, 0, m_this->indices );
	}
	shader->end();
}

int Model::sortVertexWeights( const int* boneIndices, const float* boneWeights, int bones,
	int* sortedBoneIndices, float* sortedBoneWeights )
{
	// absolute maximum number of affecting bones per vertex
	if ( bones > MAX_BONES_PER_VERTEX )
		bones = MAX_BONES_PER_VERTEX;

	// sort bones affecting the vertex by importance
	BoneWeight bone[MAX_BONES_PER_VERTEX];
	int i;
	for ( i = 0 ; i < bones ; ++i )
	{
		BoneWeight& bw = bone[i];
		bw.weight = boneWeights[i];
		bw.index = boneIndices[i];
	}
	std::sort( bone, bone+bones );

	// build sorted index and weight lists
	for ( i = 0 ; i < bones ; ++i )
	{
		sortedBoneWeights[i] = bone[i].weight;
		sortedBoneIndices[i] = bone[i].index;
	}

	return bones;
}

void Model::setVertexWeights( int vertexIndex, const int* boneIndices, const float* boneWeights, int bones )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );

	int sortedBoneIndices[MAX_BONES_PER_VERTEX];
	float sortedBoneWeights[MAX_BONES_PER_VERTEX];
	bones = sortVertexWeights( boneIndices, boneWeights, bones, sortedBoneIndices, sortedBoneWeights );

	m_this->mesh->setVertexWeights( vertexIndex, sortedBoneIndices, sortedBoneWeights, bones );
	m_this->weightsDirty = true;
}

void Model::setVertexDiffuseColors( int firstVertex, const pix::Color* colors, int count )
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canWriteVertices() );
	m_this->mesh->setVertexDiffuseColors( firstVertex, colors, count );
}

void Model::computeVertexNormals()
{
	assert( m_this );
	assert( m_this->mesh );
	assert( indicesLocked() );
	assert( verticesLocked() );
	assert( canReadVertices() );
	assert( canWriteVertices() );
	assert( m_this->indexlock.canRead() );

	const int		firstVertex		= 0;
	const int		count			= vertices();
	const Vector3	zero			(0,0,0);
	int				tri[3];

	int i;
	for ( i = firstVertex ; i < firstVertex+count ; ++i )
		setVertexNormals( i, &zero, 1 );

	for ( i = 0 ; i < m_this->indices ; i += 3 )
	{
		getIndices( i, tri, 3 );
		
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
		{
			if ( tri[k] >= firstVertex && tri[k] < firstVertex+count )
			{
				Vector3 v;
				getVertexNormals( tri[k], &v );
				v += pn;
				setVertexNormals( tri[k], &v );
			}
		}
	}

	for ( i = firstVertex ; i < firstVertex+count ; ++i )
	{
		Vector3 v;
		getVertexNormals( i, &v, 1 );
		float lensqr = v.lengthSquared();
		float s = 0.f;
		if ( lensqr > Float::MIN_VALUE )
			s = 1.f / Math::sqrt(lensqr);
		v *= s;
		setVertexNormals( i, &v, 1 );
	}
}

int Model::getVertexWeights( int vertexIndex, int* boneIndices, float* boneWeights, int maxBones ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canReadVertices() );
	return m_this->mesh->getVertexWeights( vertexIndex, boneIndices, boneWeights, maxBones );
}

void Model::getVertexDiffuseColors( int firstVertex, pix::Color* colors, int count ) const
{
	assert( m_this );
	assert( verticesLocked() );
	assert( canReadVertices() );
	m_this->mesh->getVertexDiffuses( firstVertex, colors, count );
}

float Model::boundSphere() const
{
	assert( m_this );
	assert( !verticesLocked() );

	if ( !m_this->boundSphereDirty )
		return m_this->boundSphere;

	const int vertices = this->vertices();
	if ( vertices < 3 || m_this->vf.weights() > 0 )
		return 0.f;

	VertexAndIndexLock<Model> lock( const_cast<Model*>(this), LOCK_READ );
	
	float maxr2 = 0.f;
	const int buffsize = 32;
	Vector3 buff[buffsize];

	for ( int i = 0 ; i < vertices ; )
	{
		int count = vertices-i;
		if ( count > buffsize )
			count = buffsize;

		getVertexPositions( i, buff, count );

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

	return m_this->boundSphere;
}

int Model::maxVertices() const
{
	assert( m_this );
	return m_this->maxVertices;
}

int Model::maxIndices() const
{
	assert( m_this );
	return m_this->maxIndices;
}

void Model::setVertices( int count )
{
	assert( m_this );

	if ( count > m_this->maxVertices )
		setMaxVertices( count );

	m_this->vertices = count;
	m_this->weightsDirty = true;
}

void Model::setIndices( int count )
{
	assert( m_this );

	if ( count > m_this->maxIndices )
		setMaxIndices( count );
	
	m_this->indices = count;
	m_this->weightsDirty = true;
}

void Model::setMaxVertices( int count )
{
	assert( m_this );

	if ( count > m_this->maxVertices )
	{
		m_this->maxVertices = count;
		m_this->mesh->reallocate( Context::device(), m_this->maxVertices, m_this->maxIndices );
		if ( verticesLocked() )
			getLockedData();
	}
}

void Model::setMaxIndices( int count )
{
	assert( m_this );
	assert( count % 3 == 0 );

	if ( count > m_this->maxIndices )
	{
		m_this->maxIndices = count;
		m_this->mesh->reallocate( Context::device(), m_this->maxVertices, m_this->maxIndices );
	}
}

const OBBox& Model::boundBox() const
{
	if ( m_this->boundBoxDirty && 
		0 == m_this->vf.weights() )
	{
		const int					BUFSIZE = 32;
		Vector3						v[BUFSIZE];
		VertexAndIndexLock<Model>	lock( const_cast<Model*>(this), LOCK_READ );
		OBBoxBuilder				builder;

		while ( builder.nextPass() )
		{
			for ( int i = 0 ; i < vertices() ; )
			{
				int count = vertices() - i;
				if ( count > BUFSIZE )
					count = BUFSIZE;
				getVertexPositions( i, v, count );
				builder.addPoints( v, count );
				i += count;
			}
		}

		m_this->boundBox = builder.box();
		m_this->boundBoxDirty = false;
	}
	return m_this->boundBox;
}

bool Model::updateVisibility( const Matrix4x4& modelToCamera, 
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

VertexFormat Model::vertexFormat() const
{
	return m_this->vf;
}

void Model::clearPolygonAdjancency()
{
	m_this->adj.clear();
	m_this->adjZeroDistance = -1.f;
}

const PolygonAdjacency& Model::getPolygonAdjacency( float zeroDistance ) const
{
	assert( zeroDistance >= 0.f );
	assert( m_this->vertexlock == gd::LockMode::LOCK_READ || m_this->vertexlock == gd::LockMode::LOCK_READWRITE );
	assert( m_this->indexlock == gd::LockMode::LOCK_READ || m_this->indexlock == gd::LockMode::LOCK_READWRITE );

	if ( m_this->adjZeroDistance != zeroDistance )
	{
		const int indices = this->indices();

		// get triangles
		Vector<Triangle> triangles( Allocator<Triangle>(__FILE__) );
		triangles.setSize( indices/3 );
		int triangleIndex = 0;
		for ( int i = 0 ; i < indices ; i += 3 )
		{
			// get face vertices
			Triangle& tri = triangles[triangleIndex];
			int face[3];
			getIndices( i, face, 3 );
			for ( int j = 0 ; j < 3 ; ++j )
				getVertexPositions( face[j], &tri.v[j], 1 );

			++triangleIndex;
		}

		// find adjacent edges
		m_this->adj.setPolygons( triangles.size(), 3 );
		for ( int i = 0 ; i < triangles.size() ; ++i )
		{
			const Triangle& tri = triangles[i];
			int adj[3] = {-1,-1,-1};

			int k = 2;
			for ( int j = 0 ; j < 3 ; k = j++ )
			{
				const Vector3& v0 = tri.v[k];
				const Vector3& v1 = tri.v[j];

				// find adjacent edge
				for ( int n = 0 ; n < triangles.size() ; ++n )
				{
					const Triangle& tri2 = triangles[n];
					if ( tri2.hasEdge(v1,v0) )
					{
						//if ( found )
						//	Debug::printlnWarning( "Model.getPolygonAdjacency: poly[{0}] has multiple matching edges (mtl={1})", i, shader()?shader()->name():"" );
						adj[j] = n;
						break;
					}
				}
			}

			for ( int n = 0 ; n < 3 ; ++n )
				if ( adj[n] == -1 )
					Debug::printlnWarning( "Model.getPolygonAdjacency: poly[{0}] has open edge (mtl={1})", i, shader()?shader()->name():"" );

			m_this->adj.setAdjacent( i, adj, 3 );
		}

		// store new adjacency information accuracy
		m_this->adjZeroDistance = zeroDistance;

		/*// refresh adjacency information
		const Model*							model = this;
		const int								indices = model->indices();
		int										face[3];
		Vector3									verts[6];
		int										faceindex = 0;
		Hashtable<Edge,int,EdgeHash,EdgeHash>	edges( indices/2+1, 0.75f, -1, EdgeHash(zeroDistance), EdgeHash(zeroDistance), Allocator< HashtablePair<Edge,int> >(__FILE__,__LINE__) );

		// add edges to the hashtable
		for ( int i = 0 ; i < indices ; i += 3 )
		{
			// get face vertices
			model->getIndices( i, face, 3 );
			int j;
			for ( j = 0 ; j < 3 ; ++j )
				model->getVertexPositions( face[j], &verts[j], 1 );

			// add edges to the hashtable
			int k = 2;
			for ( j = 0 ; j < 3 ; k = j++ )
				edges[ Edge(verts[k], verts[j]) ] = faceindex;

			++faceindex;
		}

		// get polygons adjacent to edges
		m_this->adj.setPolygons( faceindex, 3 );
		faceindex = 0;
		for ( int i = 0 ; i < indices ; i += 3 )
		{
			// get face vertices
			model->getIndices( i, face, 3 );
			int j;
			for ( j = 0 ; j < 3 ; ++j )
				model->getVertexPositions( face[j], &verts[j], 1 );

			// get adjacent polygons
			int adj[3];
			int k = 2;
			for ( int j = 0 ; j < 3 ; k = j++ )
			{
				Edge edge( verts[j], verts[k] );
				adj[j] = edges[edge];
			}

			m_this->adj.setAdjacent( faceindex, adj, 3 );
			++faceindex;
		}

		// store new adjacency information accuracy
		m_this->adjZeroDistance = zeroDistance;*/
	}
	return m_this->adj;
}

const int* Model::usedBoneArray() const
{
	return m_this->usedBoneArray.begin();
}

int Model::usedBones() const
{
	return m_this->usedBoneArray.size();
}

void Model::lockDefaults()
{
	m_posData	= 0;
	m_posPitch	= 0;
}

void Model::getLockedData()
{
	m_this->mesh->getVertexPositionData( &m_posData, &m_posPitch );
}

bool Model::canReadVertices() const
{
	assert( m_this );
	return m_this->mesh->verticesLocked() && m_this->vertexlock.canRead();
}

bool Model::canWriteVertices() const
{
	assert( m_this );
	return m_this->mesh->verticesLocked() && m_this->vertexlock.canWrite();
}

bool Model::canReadIndices() const
{
	assert( m_this );
	return m_this->mesh->indicesLocked() && m_this->indexlock.canRead();
}

bool Model::canWriteIndices() const
{
	assert( m_this );
	return m_this->mesh->indicesLocked() && m_this->indexlock.canWrite();
}

void Model::getVertexPositionData( float** data, int* pitch )
{
	assert( canReadVertices() || canWriteVertices() );
	*data = m_posData;
	*pitch = m_posPitch;
}

void Model::getTransformedVertexPositions( const Matrix4x4* tm, int tmcount,
	const Matrix4x4& posttm, Vector3* v, int vcount, bool mostSignifigantBoneOnly ) const
{
	assert( m_this );
	assert( canReadVertices() );
	m_this->mesh->getTransformedVertexPositions( tm, tmcount, posttm, v, vcount, mostSignifigantBoneOnly );
}

Model::UsageType Model::usage() const
{
	assert( m_this );
	return tosg( m_this->usage );
}

void Model::copyVertices( int firstVertex, const Model* other, int otherFirstVertex, int count )
{
	assert( canWriteVertices() );
	assert( other->canReadVertices() );
	assert( m_this->mesh );

	m_this->mesh->copyVertices( firstVertex, other->m_this->mesh, otherFirstVertex, count );
}

void Model::copyIndices( int firstIndex, const Model* other, int otherFirstIndex, int count )
{
	assert( canWriteIndices() );
	assert( other->canReadIndices() );
	assert( m_this->mesh );

	m_this->mesh->copyIndices( firstIndex, other->m_this->mesh, otherFirstIndex, count );
}

void Model::setBoundSphere( float r )
{
	assert( m_this );

	m_this->boundSphere = r;
	m_this->boundSphereDirty = false;
}

void Model::setBoundBox( const math::OBBox& box )
{
	assert( m_this );

	m_this->boundBox = box;
	m_this->boundBoxDirty = false;
}

void Model::getIndexData( void** data, int* indexSize )
{
	assert( data );
	assert( indexSize );
	assert( m_this );
	assert( canReadIndices() || canWriteIndices() );

	*data = m_indexData;
	*indexSize = 2;
}

void Model::computeVertexTangents( int srcLayer, int dstLayer )
{
	assert( vertexFormat().textureCoordinates() > srcLayer );
	assert( vertexFormat().textureCoordinates() > dstLayer );
	assert( vertexFormat().getTextureCoordinateSize(srcLayer) <= 3 );
	assert( vertexFormat().getTextureCoordinateSize(dstLayer) == 3 );
	assert( vertexFormat().hasNormal() );

	Array<Dot3Vertex> verts( vertices() );
	for ( int i = 0 ; i < vertices() ; ++i )
	{
		Dot3Vertex& v = verts[i];
		getVertexPositions( i, &v.pos, 1 );
		getVertexNormals( i, &v.normal, 1 );
		v.uv = Vector3(0,0,0);
		getVertexTextureCoordinates( i, srcLayer, vertexFormat().getTextureCoordinateSize(srcLayer), v.uv.begin(), 1 );
		v.s = v.t = v.sxt = Vector3(0,0,0);
	}

	uint16_t* indexData;
	int indexSize;
	getIndexData( reinterpret_cast<void**>(&indexData), &indexSize );
	assert( indexSize == 2 );

	bool smoothNormals = false;
	createBasisVectors( verts.begin(), indexData, vertices(), indices(), smoothNormals );

	for ( int i = 0 ; i < vertices() ; ++i )
	{
		setVertexTextureCoordinates( i, dstLayer, 3, verts[i].s.begin(), 1 );
		setVertexNormals( i, &verts[i].normal, 1 );
	}
}

void Model::optimizeBoneIndices()
{
	assert( canWriteVertices() );
	assert( canReadVertices() );

	if ( m_this->vf.weights() > 0 )
	{
		m_this->refreshUsedBones();
		const int* usedBoneArray = this->usedBoneArray();
		const int usedBones = this->usedBones();

		int boneIndices[MAX_BONES_PER_VERTEX];
		float boneWeights[MAX_BONES_PER_VERTEX];
		for ( int i = 0 ; i < m_this->vertices ; ++i )
		{
			int bonesPerVertex = getVertexWeights( i, boneIndices, boneWeights, MAX_BONES_PER_VERTEX );
			for ( int k = 0 ; k < bonesPerVertex ; ++k )
			{
				if ( boneWeights[k] > 0.f )
				{
					int oldix = boneIndices[k];
					int newix = -1;
					for ( int n = 0 ; n < usedBones ; ++n )
					{
						if ( usedBoneArray[n] == oldix )
						{
							newix = n;
							break;
						}
					}
					assert( newix != -1 );
					boneIndices[k] = newix;
				}
			}
			setVertexWeights( i, boneIndices, boneWeights, bonesPerVertex );
		}
	}
}


} // sg
