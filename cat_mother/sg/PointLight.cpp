#include "PointLight.h"
#include "GdUtil.h"
#include "Context.h"
#include <gd/GraphicsDevice.h>
#include <gd/LightState.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace sg
{


PointLight::PointLight()
{
	m_atten[0]	= 1.f;
	m_atten[1]	= 0.f;
	m_atten[2]	= 0.f;
	m_range		= maxRange();
}

PointLight::PointLight( const PointLight& other ) :
	Light(other) 
{
	assign(other);
}

Node* PointLight::clone() const
{
	return new PointLight( *this );
}

void PointLight::setRange( float range )
{
	assert( range >= 0.f && range <= maxRange() );
	m_range = range;
}

void PointLight::setAttenuation( float a0, float a1, float a2 )
{
	m_atten[0] = a0;
	m_atten[1] = a1;
	m_atten[2] = a2;
}

void PointLight::assign( const PointLight& other )
{
	for ( int i = 0 ; i < 3 ; ++i ) 
		m_atten[i] = other.m_atten[i];

	m_range = other.m_range;
}

float PointLight::range() const															
{
	return m_range;
}

void PointLight::getAttenuation( float* a0, float* a1, float* a2 ) const					
{
	*a0 = m_atten[0]; 
	*a1 = m_atten[1]; 
	*a2 = m_atten[2];
}

float PointLight::maxRange()															
{
	return MAX_RANGE;
}

void PointLight::apply()
{
	float intens = intensity();

	gd::LightState ls;
	GdUtil::setLightState( ls,
		gd::LightState::LIGHT_POINT,
		diffuseColor()*intens, specularColor()*intens, ambientColor()*intens,
		cachedWorldTransform().translation(), math::Vector3(1,0,0),
		m_range, m_atten[0], m_atten[1], m_atten[2], 
		1.f, MAX_CONE_ANGLE, MAX_CONE_ANGLE );

	gd::GraphicsDevice*	dev = Context::device();
	dev->addLight( ls );
}


} // sg
