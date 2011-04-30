#include "LookAtControl.h"
#include <sg/Node.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <math/Vector3.h>
#include <math/Quaternion.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace sgu
{


LookAtControl::LookAtControl( Node* source, Node* target )
{
	assert( target );
	assert( source );
	assert( target != source );

	//Debug::println( "LookAtControl: {0} looks at {1}", source->name(), target->name() );

	m_source = source;
	m_target = target;
	m_up[0] = 0.f;
	m_up[1] = 1.f;
	m_up[2] = 0.f;
}

LookAtControl::LookAtControl( Node* source, Node* target, const math::Vector3& up )
{
	assert( target );
	assert( source );
	assert( target != source );

	m_source = source;
	m_target = target;
	m_up[0] = up.x;
	m_up[1] = up.y;
	m_up[2] = up.z;
}

int LookAtControl::getValue( float time, float* value, int size, int hint ) const
{
	assert( m_target );
	assert( m_source );
	assert( m_target != m_source );
	assert( size == 4 ); size = size;

	Quaternion quat( 1.f, 0.f, 0.f, 0.f );

	// make sure transforms are up-to-date
	for ( Node* parent = m_target ; parent ; parent = parent->parent() )
		if ( parent != m_source )
			parent->setState( time );
	for ( Node* parent = m_source->parent() ; parent ; parent = parent->parent() )
		parent->setState( time );
	if ( m_source->positionController() )
	{
		float v[3];
		hint = m_source->positionController()->getValue( time, v, 3, hint );
		m_source->setPosition( Vector3(v[0], v[1], v[2]) );
	}

	// up direction and target position (world space)
	Vector3 up( m_up[0], m_up[1], m_up[2] );
	Vector3 target = m_target->worldTransform().translation();
	assert( Math::abs(up.length()-1.f) < 1e-3f ); // Up direction must be normalized

	// src parent->world space
	Matrix4x4 parentToWorld(1);
	const Node* parent = m_source->parent();
	while ( parent )
	{
		parentToWorld = parent->transform() * parentToWorld;
		parent = parent->parent();
	}

	// src->world space
	Matrix4x4 sourceToWorld = parentToWorld * m_source->transform();

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
		quat = Quaternion( rot );
	}

	value[0] = quat.x;
	value[1] = quat.y;
	value[2] = quat.z;
	value[3] = quat.w;
	return hint;
}

int LookAtControl::channels() const
{
	return 4;
}

Node* LookAtControl::target() const
{
	return m_target;
}


} // sgu
