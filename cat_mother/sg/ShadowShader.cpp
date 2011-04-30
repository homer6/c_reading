#include "ShadowShader.h"
#include "Material.h"
#include <pix/Colorf.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;

//-----------------------------------------------------------------------------

namespace sg
{


ShadowShader::ShadowShader() :
	m_mat( 0 ),
	m_flip( false ),
	m_old( false )
{
	m_mat = new Material;

	m_mat->setBlend( Material::BLEND_ZERO, Material::BLEND_ONE );
	m_mat->setLighting( false );
	m_mat->setDepthWrite( false );
	m_mat->setFogDisabled( true );
	m_mat->setZBias( 1 );
	m_mat->setVertexFormat( VertexFormat() );

	// stencil states
	m_mat->setStencil( true );
	m_mat->setStencilFunc( Material::CMP_ALWAYS );
	m_mat->setStencilRef( 1 );
	m_mat->setStencilMask( 0xFFFFFFFF );

	setPass( DEFAULT_SHADOW_VOLUME_PASS );
	setName( "ShadowShader" );
}

ShadowShader::ShadowShader( const ShadowShader& other ) :
	m_mat( other.m_mat ),
	m_flip( other.m_flip ),
	m_old( other.m_old )
{
}

ShadowShader::~ShadowShader()
{
}

Shader* ShadowShader::clone() const
{
	return new ShadowShader( *this );
}

void ShadowShader::destroy()
{
}

void ShadowShader::load()
{
	if ( m_mat )
		m_mat->load();
}

void ShadowShader::unload()
{
	if ( m_mat )
		m_mat->unload();
}

void ShadowShader::apply( int pass )
{
	if ( m_old )
	{
		// old stencil method
		if ( 0 == pass )
		{
			m_mat->setCull( Material::CULL_CCW );
			m_mat->setStencilPass( Material::STENCILOP_INCR );
			m_mat->setDepthFunc( Material::CMP_LESS );
		}
		else if ( 1 == pass )
		{
			m_mat->setCull( Material::CULL_CW );
			m_mat->setStencilPass( Material::STENCILOP_DECR );
			m_mat->setDepthFunc( Material::CMP_LESS );
		}
	}
	else
	{
		// new stencil method
		if ( 0 == pass )
		{
			if ( m_flip )
				m_mat->setCull( Material::CULL_CCW );
			else
				m_mat->setCull( Material::CULL_CW );
			m_mat->setStencilPass( Material::STENCILOP_INCR );
			m_mat->setDepthFunc( Material::CMP_GREATER );
		}
		else if ( 1 == pass )
		{
			if ( m_flip )
				m_mat->setCull( Material::CULL_CW );
			else
				m_mat->setCull( Material::CULL_CCW );
			m_mat->setStencilPass( Material::STENCILOP_DECR );
			m_mat->setDepthFunc( Material::CMP_GREATER );
		}
	}

	if ( pass == 1 )
		m_mat->end();

	m_mat->begin();
	m_mat->apply( 0 );
}

void ShadowShader::setFlip( bool flip )
{
	m_flip = flip;
}

void ShadowShader::setOld( bool old )
{
	m_old = old;
}

int	ShadowShader::begin()
{
	return 2;
}

void ShadowShader::end()
{
	m_mat->end();
}

void ShadowShader::setVertexFormat( const VertexFormat& vf )
{
	m_mat->setVertexFormat( vf );
}

VertexFormat ShadowShader::vertexFormat() const
{
	return m_mat->vertexFormat();
}


} // sg
