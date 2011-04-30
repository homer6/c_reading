#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"
#include "Context.h"
#include "Primitive.h"
#include <gd/GraphicsDevice.h>
#include <dev/Profile.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <util/Vector.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace dev;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


static Vector<Matrix4x4>		s_boneMatrices( Allocator<Matrix4x4>(__FILE__) ); // WARNING: not thread safe
static Vector<const Matrix4x4*> s_boneMatrixPtrs( Allocator<const Matrix4x4*>(__FILE__) ); // WARNING: not thread safe
static int						s_meshes = 0;

//-----------------------------------------------------------------------------

class Bone
{
public:
	Node*		node;
	int			index;
	Matrix4x4	inverseRestTransform;

	Bone()
	{
		node = 0;
		index = -1;
	}

	Bone( Node* boneNode, int boneIndex, const Matrix4x4& restTransform )
	{
		node					= boneNode;
		index					= boneIndex;
		inverseRestTransform	= restTransform.inverse();
	}
};

//-----------------------------------------------------------------------------

class Mesh::MeshImpl :
	public lang::Object
{
public:
	Vector<P(Primitive)>	primitives;
	Vector<Bone>			bones;
	float					boundSphere;
	bool					boundDirty;

	MeshImpl() :
		primitives( Allocator<P(Primitive)>(__FILE__,__LINE__) ),
		bones( Allocator<Bone>(__FILE__,__LINE__) ),
		boundSphere(0.f),
		boundDirty(false)
	{
	}

	void refreshBoundSphere()
	{
		float maxr = 0.f;
		for ( int i = 0 ; i < primitives.size() ; ++i )
		{
			float r = primitives[i]->boundSphere();
			if ( r > maxr )
				maxr = r;
		}
		boundSphere = maxr;
		boundDirty = false;
	}
};

//-----------------------------------------------------------------------------

Mesh::Mesh()
{
	m_this = new MeshImpl;
	++s_meshes;
}

Mesh::Mesh( const Mesh& other ) :
	Node(other)
{
	m_this = new MeshImpl( *other.m_this );

	for ( int i = 0 ; i < m_this->primitives.size() ; ++i )
	{
		P(Primitive) prim = m_this->primitives[i]->clone( Primitive::SHARE_GEOMETRY );
		m_this->primitives[i] = prim;
	}
	++s_meshes;
}

Mesh::~Mesh()
{
	if ( --s_meshes == 0 )
	{
		s_boneMatrices.clear();
		s_boneMatrices.trimToSize();
		s_boneMatrixPtrs.clear();
		s_boneMatrixPtrs.trimToSize();
	}
}

Node* Mesh::clone() const
{
	return new Mesh( *this );
}

bool Mesh::updateVisibility( Camera* camera )
{
	//Profile pr( "Mesh.updateVisibility" );

	// skinned object always visible
	if ( m_this->bones.size() > 0 )
		return true;

	// test for visible primitives
	Matrix4x4 modelToCamera = camera->cachedWorldToCamera() * cachedWorldTransform();
	for ( int i = 0 ; i < m_this->primitives.size() ; ++i )
	{
		Primitive* prim = m_this->primitives[i];
		if ( prim->updateVisibility( modelToCamera, camera->viewFrustum() ) )
			return true;
	}

	return false;
}

void Mesh::render( Camera* camera, int pass )
{
	//dev::Profile pr( "Mesh.render" );
	assert( m_this );
	assert( parent() );

	// build bone matrices
	if ( m_this->bones.size() > 0 )
	{
		s_boneMatrices.setSize( m_this->bones.size()+1 );
		getBoneMatrix4x3Array( s_boneMatrices.begin(), s_boneMatrices.size() );
	}
	else
	{
		s_boneMatrices.setSize( 0 );
		s_boneMatrixPtrs.setSize( 0 );
	}

	// model->world->view->proj transform
	Matrix4x4 totalTm;
	bool totalTmValid = false;

	// draw primitives
	Matrix4x4 modelToCamera;
	bool modelToCameraValid = false;
	gd::GraphicsDevice* dev = Context::device();

	for ( int i = 0 ; i < m_this->primitives.size() ; ++i )
	{
		Primitive* prim = m_this->primitives[i];
		Shader* shader = prim->shader();
		//assert( shader ); // primitive has to have shader set

		if ( shader && 0 != (shader->pass() & pass) )
		{
			// compute model->camera transform only when needed
			if ( !modelToCameraValid )
			{
				modelToCamera = camera->cachedWorldToCamera() * cachedWorldTransform();
				modelToCameraValid = true;
			}

			// render visible primitives
			assert( prim->vertexFormat() == prim->shader()->vertexFormat() );
			bool vis = prim->updateVisibility( modelToCamera, camera->viewFrustum() );
			int usedBones = prim->usedBones();
			if ( vis || usedBones > 0 )
			{
				// set model-to-world transform (used by fixed pipeline and shadow volumes)
				if ( 0 == usedBones )
					dev->setWorldTransform( 0, cachedWorldTransform() );

				// set bone-to-world transforms (used by fixed pipeline and shadow volumes)
				const int* usedBoneArray = prim->usedBoneArray();
				for ( int k = 0 ; k < usedBones ; ++k )
				{
					int ix = usedBoneArray[k];
					assert( ix >= 0 && ix <= m_this->bones.size() );
					if ( 0 == ix )
					{
						dev->setWorldTransform( k, cachedWorldTransform() );
					}
					else if ( ix <= m_this->bones.size() )
					{
						const Bone& bone = m_this->bones[ix-1];
						dev->setWorldTransform( k, bone.node->cachedWorldTransform() * bone.inverseRestTransform );
					}
				}

				// set shader parameter (used by models using shaders)
				if ( shader->hasParameter("mWorld") )
				{
					shader->setMatrix4x4( "mWorld", cachedWorldTransform() );
				}
				if ( shader->hasParameter("mWorldA") )
				{
					int usedBones = prim->usedBones();
					const int* usedBoneArray = prim->usedBoneArray();
					s_boneMatrixPtrs.setSize( usedBones );
					for ( int k = 0 ; k < usedBones ; ++k )
						s_boneMatrixPtrs[k] = &s_boneMatrices[ usedBoneArray[k] ];
					shader->setMatrix4x4PointerArray( "mWorldA", s_boneMatrixPtrs.begin(), s_boneMatrixPtrs.size() );
				}
				if ( shader->hasParameter("mTotal") )
				{
					if ( !totalTmValid )
					{
						totalTm = camera->cachedViewProjectionTransform() * cachedWorldTransform();
						totalTmValid = true;
					}
					shader->setMatrix4x4( "mTotal", totalTm );
				}
				if ( shader->hasParameter("mView") )
				{
					shader->setMatrix4x4( "mView", camera->cachedViewTransform() );
				}
				if ( shader->hasParameter("mProjection") )
				{
					shader->setMatrix4x4( "mProjection", camera->cachedProjectionTransform() );
				}
				if ( shader->hasParameter("mViewProjection") )
				{
					shader->setMatrix4x4( "mViewProjection", camera->cachedViewProjectionTransform() );
				}
				if ( shader->hasParameter("pCamera") )
				{
					Vector3 worldCameraPos = camera->cachedWorldTransform().translation();
					shader->setVector4( "pCamera", Vector4(worldCameraPos.x,worldCameraPos.y,worldCameraPos.z,1.f) );
				}

				prim->draw();
			}
		}
	}
}

void Mesh::addPrimitive( Primitive* primitive )
{
	assert( m_this );
	assert( primitive );
	
	m_this->primitives.add( primitive );
	m_this->boundDirty = true;
}

void Mesh::removePrimitive( int index )
{
	assert( m_this );
	assert( index >= 0 && index < primitives() );
	
	m_this->primitives.remove( index );
	m_this->boundDirty = true;
}

void Mesh::removePrimitives()
{
	assert( m_this );

	m_this->primitives.clear();
	m_this->boundDirty = true;
}

Primitive* Mesh::getPrimitive( int index ) const
{
	assert( m_this );
	assert( index >= 0 && index < primitives() );

	m_this->boundDirty = true;
	return m_this->primitives[index];
}

int	Mesh::primitives() const
{
	assert( m_this );
	
	return m_this->primitives.size();
}

void Mesh::addBone( Node* bone, const Matrix4x4& restTransform )
{
	assert( m_this );
	
	m_this->bones.add( Bone(bone,bones(),restTransform) );
}

void Mesh::setBones( const Mesh* other )
{
	assert( m_this );
	assert( other->m_this );

	m_this->bones = other->m_this->bones;
	m_this->boundDirty = true;
}

void Mesh::removeBone( int index )
{
	assert( m_this );
	assert( index >= 0 && index < bones() );

	m_this->bones.remove( index );
}

Node* Mesh::getBone( int index ) const
{
	assert( m_this );
	assert( index >= 0 && index < bones() );

	return m_this->bones[index].node;
}

int Mesh::bones() const
{
	assert( m_this );

	return m_this->bones.size();
}

float Mesh::boundSphere() const
{
	if ( m_this->boundDirty )
		m_this->refreshBoundSphere();
	return m_this->boundSphere;
}

void Mesh::restoreBones( Node* root )
{
	for ( int i = 0 ; i < m_this->bones.size() ; ++i )
	{
		Node* node = 0;
		for ( node = root ; node ; node = node->nextInHierarchy() )
		{
			if ( node->name() == m_this->bones[i].node->name() )
			{
				m_this->bones[i].node = node;
				break;
			}
		}
		if ( !node )
			throw Exception( Format("Bone {0} not found from the {1}", m_this->bones[i].node->name(), root->name()) );
	}
}

void Mesh::getBoneMatrix4x3Array( Matrix4x4* tm, int count ) const
{
	assert( cachedWorldTransformValid() );
	assert( count == bones()+1 );
	assert( count > 0 );

	Matrix4x4& m = tm[0];
	m = cachedWorldTransform();
	m.setRow( 3, m.getColumn(3) );	// convert -> 4x3 column-major
	m.setColumn( 3, Vector4(0,0,0,0) );

	int boneCount = m_this->bones.size();
	if ( boneCount >= count )
		boneCount = count-1;

	for ( int i = 0 ; i < boneCount ; ++i )
	{
		const Bone& bone = m_this->bones[i];
		Matrix4x4& m = tm[i+1];
		m = bone.node->cachedWorldTransform() * bone.inverseRestTransform;
		m.setRow( 3, m.getColumn(3) );	// convert -> 4x3 column-major
		m.setColumn( 3, Vector4(0,0,0,0) );
	}
}

const Matrix4x4& Mesh::getBoneInverseRestTransform( int index ) const
{
	assert( m_this );
	assert( index >= 0 && index < bones() );

	return m_this->bones[index].inverseRestTransform;
}


} // sg
