#include "Node.h"
#include "Camera.h"
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Object.h>
#include <math/Vector3.h>
#include <math/Matrix3x3.h>
#include <math/Quaternion.h>
#include <anim/Control.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;
using namespace anim;

//-----------------------------------------------------------------------------

namespace sg
{


Node::Node() :
	m_tmHints( Allocator<TransformHint>(__FILE__) )
{
	defaults();
}

Node::Node( const Node& other ) :
	Animatable( other ),
	m_tmHints( other.m_tmHints )
{
	defaults();
	assign( other );

	for ( Node* child = other.m_child ; child ; child = child->m_next )
		child->clone()->linkTo( this );
}

Node::~Node()
{
	unlink();

	while ( m_child )
		m_child->unlink();
}

Node* Node::clone() const
{
	return new Node( *this );
}

void Node::setName( const String& name )
{
	m_name = name;
}

const String& Node::name() const
{
	return m_name;
}

void Node::setTransform( const Matrix4x4& transform )
{
	assert( transform.getColumn(0).finite() );
	assert( transform.getColumn(1).finite() );
	assert( transform.getColumn(2).finite() );
	assert( transform.getColumn(3).finite() );

	m_localTransform = transform;
	m_flags |= NODE_WORLDTMDIRTY;
}

void Node::setPosition( const Vector3& position )									
{
	assert( position.finite() );

	m_localTransform.setTranslation( position );
	m_flags |= NODE_WORLDTMDIRTY;
}

void Node::setRotation( const Matrix3x3& rotation )
{
	assert( rotation.getColumn(0).finite() );
	assert( rotation.getColumn(1).finite() );
	assert( rotation.getColumn(2).finite() );
	assert( ( rotation.getColumn(0).cross(rotation.getColumn(1)) ).dot( rotation.getColumn(2) ) > 0.f ); // Left handed coordinate system?

	m_localTransform.setRotation( rotation );
	m_flags |= NODE_WORLDTMDIRTY;
}

const Matrix4x4& Node::worldTransform() const
{
	if ( !cachedWorldTransformValid() )
		root()->validateHierarchy();
	return m_worldTransform;
}

void Node::validateHierarchy( bool forceUpdate ) const
{
	if ( (m_flags & NODE_WORLDTMDIRTY) || forceUpdate )
	{
		// compute model->world transform
		Node* parent = this->parent();
		if ( parent )
			m_worldTransform = parent->m_worldTransform * m_localTransform;
		else
			m_worldTransform = m_localTransform;
		
		m_flags &= ~NODE_WORLDTMDIRTY;
		forceUpdate = true;
	}

	Node* child = m_child;
	for ( ; child ; child = child->m_next )
		child->validateHierarchy( forceUpdate );
}

void Node::lookAt( const math::Vector3& target, const math::Vector3& up )
{
	assert( Math::abs(up.length()-1.f) < 1e-3f ); // Up direction must be normalized

	// src parent->world space
	Matrix4x4 parentToWorld = Matrix4x4(1);
	const Node* parent = this->parent();
	while ( parent )
	{
		parentToWorld = parent->transform() * parentToWorld;
		parent = parent->parent();
	}

	// src->world space
	Matrix4x4 sourceToWorld = parentToWorld * transform();

	// src -> target (world space)
	Vector3 sourceRotZ = target - sourceToWorld.translation();
	if ( sourceRotZ.lengthSquared() > Float::MIN_VALUE )
	{
		// src->target direction (world space)
		sourceRotZ = sourceRotZ.normalize();

		// src rotation (world space)
		Vector3 sourceRotX = up.cross( sourceRotZ );
		if ( sourceRotX.lengthSquared() > Float::MIN_VALUE )
			sourceRotX = sourceRotX.normalize();
		else
			sourceRotX = Vector3(1,0,0);
		Vector3 sourceRotY = sourceRotZ.cross( sourceRotX );
		Matrix3x3 sourceRot;
		sourceRot.setColumn( 0, sourceRotX );
		sourceRot.setColumn( 1, sourceRotY );
		sourceRot.setColumn( 2, sourceRotZ );

		// src world space rotation back to src parent space
		Matrix3x3 parentToWorldRot = parentToWorld.rotation();
		Matrix3x3 rot = sourceRot * parentToWorldRot.inverse();
		setRotation( rot );
	}
}

void Node::lookAt( const Node* other, const Vector3& up )
{
	lookAt( other->worldTransform().translation(), up );
}

void Node::linkTo( Node* parent )
{
	assert( parent );					// parent node must exist
	assert( !parent->hasParent(this) ); // cannot handle cyclic hierarchies
	assert( parent != this );			// immediate cyclic hierarchy

	unlink();

	m_next = parent->m_child;
	if ( m_next ) 
		m_next->m_previous = this;

	m_parent = parent;
	m_parent->m_child = this;

	m_flags |= NODE_WORLDTMDIRTY;
}

void Node::unlink()
{
	if ( m_parent )
	{
		// keep reference for safety (avoid premature destruction)
		P(Node) thisNode = this;
		
		if ( m_parent->m_child == this )
		{
			// this is the first child
			assert( !m_previous );
			m_parent->m_child = m_next;
			if ( m_next )
				m_next->m_previous = 0;
		}
		else
		{
			// this is not the first child
			assert( m_previous );
			if ( m_previous )
				m_previous->m_next = m_next;
			if ( m_next )
				m_next->m_previous = m_previous;
		}

		m_next = m_previous = m_parent = 0;
	}
}

Node* Node::root() const
{
	Node* root = const_cast<Node*>(this);
	while ( root->parent() )
		root = root->parent();
	return root;
}

Node* Node::nextInHierarchy( int childFlags ) const
{
	if ( m_child && (m_flags&childFlags) == childFlags )
	{
		// child
		return m_child; 
	}
	else if ( m_next ) 
	{
		// no child but sibling
		return m_next; 
	}
	else 
	{
		// no child no sibling
		for ( Node* node = m_parent ; node ; node = node->m_parent )
			if ( node->m_next )
				return node->m_next;
	}

	return 0;
}

bool Node::updateVisibility( Camera* /*camera*/ )
{
	return false;
}

void Node::render( Camera*, int )
{
}

void Node::defaults()
{
	m_name					= "";
	m_localTransform		= Matrix4x4(1);
	m_flags					= NODE_DEFAULTS;

	m_distanceToCamera		= 0.f;

	m_parent				= 0;
	m_child					= 0;
	m_next					= 0;
	m_previous				= 0;

	m_posCtrl				= 0;
	m_rotCtrl				= 0;
	m_scaleCtrl				= 0;
}

bool Node::hasParent( const Node* other ) const
{
	const Node* parent = this->parent();
	while ( parent )
	{
		if ( other == parent )
			return true;

		parent = parent->parent();
	}
	return false;
}

void Node::assign( const Node& other )
{
	if ( &other != this )
	{
		m_name					= other.m_name;
		m_localTransform		= other.m_localTransform;		
		m_flags					= other.m_flags;
		m_flags					|= NODE_WORLDTMDIRTY;

		m_posCtrl				= other.m_posCtrl;
		m_rotCtrl				= other.m_rotCtrl;
		m_scaleCtrl				= other.m_scaleCtrl;
	}
}

const Matrix4x4& Node::transform() const										
{
	return m_localTransform;
}

Matrix3x3 Node::rotation() const
{
	return m_localTransform.rotation();
}

Vector3 Node::position() const
{
	return m_localTransform.translation();
}

Node* Node::parent() const											
{
	return m_parent;
}

Node* Node::firstChild() const											
{
	return m_child;
}

Node* Node::getNextChild( const Node* child ) const
{	
	return child->m_next;
}

const math::Matrix4x4& Node::cachedWorldTransform() const
{
	assert( cachedWorldTransformValid() );
	return m_worldTransform;
}

void Node::setEnabled( bool enabled )
{
	m_flags &= ~NODE_ENABLED;
	if ( enabled )
		m_flags |= NODE_ENABLED;
}

void Node::setRenderable( bool enabled )
{
	m_flags &= ~NODE_RENDERABLE;
	if ( enabled )
		m_flags |= NODE_RENDERABLE;
}

void Node::setPositionController( Control* control )
{
	assert( !control || control->channels() == 3 );
	m_posCtrl = control;
}

void Node::setRotationController( Control* control )
{
	assert( !control || control->channels() == 4 );	// quaternion (x,y,z,w)
	m_rotCtrl = control;
}

void Node::setScaleController( Control* control )
{
	assert( !control || control->channels() == 3 );
	m_scaleCtrl = control;
}

Control* Node::positionController() const
{
	return m_posCtrl;
}

Control* Node::rotationController() const
{
	return m_rotCtrl;
}

Control* Node::scaleController() const
{
	return m_scaleCtrl;
}

void Node::blendState( Animatable** anims, 
	const float* times, const float* weights, int n )
{
	//dev::Profile pr( "Node.blendState" );
	m_tmHints.setSize( n );

	// position
	{
		//dev::Profile pr( "Node.blendState.pos" );
		Vector3 pos( 0.f, 0.f, 0.f );
		for ( int i = 0 ; i < n ; ++i )
		{
			assert( dynamic_cast<Node*>( anims[i] ) );
			Node* anim = static_cast<Node*>( anims[i] );
			assert( anim );
			if ( anim )
			{
				if ( anim->m_posCtrl )
				{
					float v[3];
					m_tmHints[i].posHint = anim->m_posCtrl->getValue( times[i], v, 3, m_tmHints[i].posHint );
					float w = weights[i];
					pos.x += v[0] * w;
					pos.y += v[1] * w;
					pos.z += v[2] * w;
				}
				else
				{
					pos += anim->m_localTransform.translation() * weights[i];
				}
			}
		}
		setPosition( pos );
	}

	// scaling
	Vector3 scale( 0.f, 0.f, 0.f );
	bool scaling = false;
	{
		//dev::Profile pr( "Node.blendState.scl" );
		for ( int i = 0 ; i < n ; ++i )
		{
			assert( dynamic_cast<Node*>( anims[i] ) );
			Node* anim = static_cast<Node*>( anims[i] );
			assert( anim );
			if ( anim )
			{
				if ( anim->m_scaleCtrl )
				{
					float v[3];
					m_tmHints[i].scaleHint = anim->m_scaleCtrl->getValue( times[i], v, 3, m_tmHints[i].scaleHint );
					float w = weights[i];
					scale.x += v[0] * w;
					scale.y += v[1] * w;
					scale.z += v[2] * w;
					scaling = true;
				}
				else
				{
					float w = weights[i];
					scale.x += w;
					scale.y += w;
					scale.z += w;
				}
			}
		}
	}

	// rotation
	{
		//dev::Profile pr( "Node.blendState.rot" );
		Quaternion rot( 0.f, 0.f, 0.f, 1.f );
		float rotWeight = 0.f;
		for ( int i = 0 ; i < n ; ++i )
		{
			assert( dynamic_cast<Node*>( anims[i] ) );
			Node* anim1 = static_cast<Node*>( anims[i] );
			assert( anim1 );
			if ( anim1 )
			{
				Quaternion rot1;
				if ( anim1->m_rotCtrl )
				{
					float v[4];
					m_tmHints[i].rotHint =	anim1->m_rotCtrl->getValue( times[i], v, 4, m_tmHints[i].rotHint );
					rot1.x = v[0];
					rot1.y = v[1];
					rot1.z = v[2];
					rot1.w = v[3];
				}
				else
				{
					rot1 = Quaternion( anim1->m_localTransform.rotation().orthonormalize() );
				}

				if ( rotWeight < Float::MIN_VALUE )
				{
					rot = rot1;
					rotWeight = weights[i];
				}
				else
				{
					if ( rot.dot(rot1) < 0.f )
						rot1 = -rot1;
					float totalWeight = rotWeight + weights[i];
					float w = weights[i];
					float w0 = w / totalWeight;
					rot = rot.slerp( w0, rot1 );
					rotWeight += w;
				}
			}
		}

		// apply scaling to rotation
		Matrix3x3 rotm( rot );
		if ( scaling )
		{
			rotm.setColumn( 0, rotm.getColumn(0)*scale.x );
			rotm.setColumn( 1, rotm.getColumn(1)*scale.y );
			rotm.setColumn( 2, rotm.getColumn(2)*scale.z );
		}
		setRotation( rotm );
	}
}

float Node::boundSphere() const
{
	return 0.f;
}


} // sg
