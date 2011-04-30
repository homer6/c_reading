#include "SpotLight.h"
#include "GdUtil.h"
#include "Context.h"
#include <gd/GraphicsDevice.h>
#include <gd/LightState.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


SpotLight::SpotLight()
{
	m_atten[0]	= 1.f;
	m_atten[1]	= 0.f;
	m_atten[2]	= 0.f;
	m_range		= maxRange();
	m_inner		= maxConeAngle();
	m_outer		= maxConeAngle();
}

SpotLight::SpotLight( const SpotLight& other ) :
	Light(other) 
{
	assign( other );
}

Node* SpotLight::clone() const
{
	return new SpotLight( *this );
}
	
void SpotLight::setInnerCone( float angle )
{
	assert( angle >= 0.f && angle <= maxConeAngle() );
	m_inner = angle;
}
	
void SpotLight::setOuterCone( float angle )
{
	assert( angle >= 0.f && angle <= maxConeAngle() );
	m_outer = angle;
}

void SpotLight::setRange( float range )
{
	assert( range >= 0.f && range <= maxRange() );
	m_range = range;
}

void SpotLight::setAttenuation( float a0, float a1, float a2 )
{
	m_atten[0] = a0;
	m_atten[1] = a1;
	m_atten[2] = a2;
}

void SpotLight::assign( const SpotLight& other )
{
	for ( int i = 0 ; i < 3 ; ++i ) 
		m_atten[i] = other.m_atten[i];

	m_range = other.m_range;

	m_inner = other.m_inner;
	m_outer = other.m_outer;
}

float SpotLight::range() const															
{
	return m_range;
}

void SpotLight::getAttenuation( float* a0, float* a1, float* a2 ) const					
{
	*a0 = m_atten[0]; 
	*a1 = m_atten[1]; 
	*a2 = m_atten[2];
}

float SpotLight::maxRange()															
{
	return MAX_RANGE;
}

void SpotLight::apply()
{
	const math::Matrix4x4&	wt		= cachedWorldTransform();
	math::Vector3			wrotz	= math::Vector3( wt(0,2),wt(1,2),wt(2,2) );

	float wrotzlen = wrotz.length();
	if ( wrotzlen > Float::MIN_VALUE )
	{
		wrotz *= 1.f / wrotzlen;
		float intens = intensity();

		gd::LightState ls;
		GdUtil::setLightState( ls,
			gd::LightState::LIGHT_SPOT,
			diffuseColor()*intens, specularColor()*intens, ambientColor()*intens,
			wt.translation(), wrotz,
			m_range, m_atten[0], m_atten[1], m_atten[2],
			1.f, m_inner, m_outer );

		gd::GraphicsDevice*	dev = Context::device();
		dev->addLight( ls );
	}
}

float SpotLight::maxConeAngle()
{
	return MAX_CONE_ANGLE;
}

float SpotLight::innerCone() const
{
	return m_inner;
}
	
float SpotLight::outerCone() const
{
	return m_outer;
}


} // sg
