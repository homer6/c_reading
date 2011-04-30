#include "Primitive.h"
#include <sg/Shader.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


Primitive::Primitive() :
	m_shader(0)
{
}

Primitive::~Primitive()
{
}

Primitive::Primitive( const Primitive& other, int shareFlags )
{
	if ( shareFlags & SHARE_SHADER )
		m_shader = other.m_shader;
	else if ( other.m_shader )
		m_shader = other.m_shader->clone();
}

bool Primitive::updateVisibility( const math::Matrix4x4& /*modelToCamera*/, 
	const ViewFrustum& /*viewFrustum*/ )
{
	return true;
}

void Primitive::setShader( Shader* shader )
{
	assert( shader->vertexFormat() == vertexFormat() );

	m_shader = shader;
}

float Primitive::boundSphere() const
{
	return 0.f;
}

Shader* Primitive::shader() const
{
	return m_shader;
}

int Primitive::usedBones() const
{
	return 0;
}

const int* Primitive::usedBoneArray() const
{
	return 0;
}


} // sg
