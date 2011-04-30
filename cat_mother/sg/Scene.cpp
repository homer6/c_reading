#include "Scene.h"
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;

//-----------------------------------------------------------------------------

namespace sg
{


Scene::Scene()
{
	m_fog			= FOG_NONE;
	m_fogStart		= 0.f;
	m_fogEnd		= 1.f;
	m_fogDensity	= 0.5f;
	m_fogColor		= Color(0,0,0);
	m_ambientColor	= Color(0,0,0);
	m_animEnd		= 0.f;
}

Scene::Scene( const Scene& other ) :
	Node( other )
{
	m_fog			= other.m_fog;
	m_fogStart		= other.m_fogStart;
	m_fogEnd		= other.m_fogEnd;		
	m_fogDensity	= other.m_fogDensity;
	m_fogColor		= other.m_fogColor;		
	m_ambientColor	= other.m_ambientColor;
	m_animEnd		= other.m_animEnd;
}

Node* Scene::clone() const
{
	return new Scene( *this );
}

void Scene::setFog( FogMode mode )
{
	m_fog = mode;
}

void Scene::setFogColor( const Color& color )
{
	m_fogColor = color;
}

void Scene::setFogStart( float start )
{
	m_fogStart = start;
}

void Scene::setFogEnd( float end )
{
	m_fogEnd = end;
}

void Scene::setFogDensity( float density )
{
	m_fogDensity = density;
}

Scene::FogMode Scene::fog() const
{
	return m_fog;
}

const Color& Scene::fogColor() const
{
	return m_fogColor;
}

float Scene::fogStart() const
{
	return m_fogStart;
}

float Scene::fogEnd() const
{
	return m_fogEnd;
}

float Scene::fogDensity() const
{
	return m_fogDensity;
}

void Scene::setAmbientColor( const Color& color )
{
	m_ambientColor = color;
}

const Color& Scene::ambientColor() const
{
	return m_ambientColor;
}

void Scene::setAnimationEnd( float time )
{
	m_animEnd = time;
}

float Scene::animationEnd() const
{
	return m_animEnd;
}


} // sg
