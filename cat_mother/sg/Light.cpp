#include "Light.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;

//-----------------------------------------------------------------------------

namespace sg
{


const float	Light::MAX_RANGE		= 1e9f;
const float	Light::MAX_CONE_ANGLE	= 3.13845106f;

//-----------------------------------------------------------------------------

Light::Light()
{
	m_diffuse			= Colorf(1,1,1);
	m_specular			= Colorf(1,1,1);
	m_ambient			= Colorf(0,0,0);
	m_intensity			= 1.f;
}

Light::~Light()
{
}

Light::Light( const Light& other ) :
	Node(other)
{
	assign( other );
}

void Light::setDiffuseColor( const Colorf& diffuse )
{
	m_diffuse = diffuse;
}

void Light::setSpecularColor( const Colorf& specular )
{
	m_specular = specular;
}

void Light::setAmbientColor( const Colorf& ambient )
{
	m_ambient = ambient;
}

const Colorf& Light::diffuseColor() const
{
	return m_diffuse;
}

const Colorf& Light::specularColor() const
{
	return m_specular;
}

const Colorf& Light::ambientColor() const
{
	return m_ambient;
}

void Light::assign( const Light& other )
{
	m_diffuse		= other.m_diffuse;
	m_specular		= other.m_specular;
	m_ambient		= other.m_ambient;
	m_intensity		= other.m_intensity;
}

void Light::setIntensity( float value )
{
	m_intensity = value;
}

float Light::intensity() const
{
	return m_intensity;
}


} // sg
