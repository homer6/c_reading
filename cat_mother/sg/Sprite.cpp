#include "Sprite.h"
#include <sg/Texture.h>
#include <sg/Material.h>
#include <sg/VertexLock.h>
#include <sg/VertexFormat.h>
#include <sg/TriangleList.h>
#include <lang/Math.h>
#include <math/Vector4.h>
#include <pix/Color.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


Sprite::Sprite( Texture* tex, Material* mat, TriangleList* tri ) :
	m_pos( 0.f, 0.f ),
	m_scale( 1.f, 1.f ),
	m_size( (float)tex->width(), (float)tex->height() ),
	m_rot( 0.f ),
	m_tri( tri )
{
	mat->setTexture( 0, tex );
	setShader( mat );
}

Sprite::Sprite( const Sprite& other, int shareFlags ) :
	Primitive( other, shareFlags ),
	m_pos( other.m_pos ),
	m_scale( other.m_scale ),
	m_size( other.m_size ),
	m_rot( other.m_rot ),
	m_tri( other.m_tri )
{
	assert( shareFlags & SHARE_GEOMETRY );
}

Primitive* Sprite::clone( int shareFlags ) const
{
	return new Sprite( *this, shareFlags );
}

Sprite::~Sprite()
{
}

void Sprite::setPosition( const Vector2& v )
{
	m_pos = v;
}

void Sprite::setScale( const Vector2& v )
{
	m_scale = v;
}

void Sprite::load()
{
}

void Sprite::unload()
{
}

void Sprite::destroy()
{
	Primitive::destroy();
}

void Sprite::draw()
{
	assert( m_tri->maxVertices() >= 6 );

	float dx0 = m_size.x * m_scale.x * .5f;
	float dy0 = m_size.y * m_scale.y * .5f;
	float x = m_pos.x + dx0;
	float y = m_pos.y + dy0;
	float cosr = Math::cos(m_rot);
	float sinr = Math::sin(m_rot);
	float dxU = dx0*cosr;
	float dxV = -dy0*sinr;
	float dyU = dx0*sinr;
	float dyV = dy0*cosr;

	x -= .5f;
	y -= .5f;

	Vector4 pos[6] =
	{
		Vector4(x-dxU-dxV,y-dyU-dyV,0,1), // C-U-V
		Vector4(x+dxU-dxV,y+dyU-dyV,0,1), // C+U-V
		Vector4(x+dxU+dxV,y+dyU+dyV,0,1), // C+U+V
		Vector4(x-dxU-dxV,y-dyU-dyV,0,1), // C-U-V
		Vector4(x+dxU+dxV,y+dyU+dyV,0,1), // C+U+V
		Vector4(x-dxU+dxV,y-dyU+dyV,0,1), // C-U+V
	};

	float x0 = 0.f;
	float y0 = 0.f;
	float x1 = 1.f;
	float y1 = 1.f;

	float uv[2*6] =
	{
		x0, y0,
		x1, y0,
		x1, y1,
		x0, y0,
		x1, y1,
		x0, y1
	};

	{
		VertexLock<TriangleList> lk( m_tri, TriangleList::LOCK_WRITE );
		m_tri->setVertexPositionsRHW( 0, pos, 6 );
		m_tri->setVertexTextureCoordinates( 0, 0, 2, uv, 6 );
	}
	
	m_tri->setShader( shader() );
	m_tri->setVertices( 6 );
	m_tri->draw();
}

float Sprite::width() const
{
	return m_size.x;
}

float Sprite::height() const
{
	return m_size.y;
}

TriangleList* Sprite::createTriangleList( int verts )
{
	assert( verts >= 6 );

	return new TriangleList( verts, defaultVertexFormat(), TriangleList::USAGE_DYNAMIC );
}

Material* Sprite::createMaterial()
{
	Material* mat = new Material;
	mat->setDepthEnabled( false );
	mat->setDepthWrite( false );
	mat->setBlend( Material::BLEND_SRCALPHA, Material::BLEND_INVSRCALPHA );
	mat->setTextureColorCombine( 0, Material::TA_TEXTURE, Material::TOP_SELECTARG1, Material::TA_CURRENT );
	mat->setTextureAlphaCombine( 0, Material::TA_TEXTURE, Material::TOP_SELECTARG1, Material::TA_CURRENT );
	mat->setLighting( false );
	mat->setCull( Material::CULL_NONE );
	mat->setFogDisabled( true );
	mat->setVertexFormat( defaultVertexFormat() );
	return mat;
}

const Vector2& Sprite::position() const
{
	return m_pos;
}

const Vector2& Sprite::scale() const
{
	return m_scale;
}

void Sprite::setRotation( float angle )
{
	m_rot = angle;
}

float Sprite::rotation() const
{
	return m_rot;
}

VertexFormat Sprite::defaultVertexFormat()
{
	VertexFormat vf;
	vf.addRHW().addTextureCoordinate( 2 );
	return vf;
}

VertexFormat Sprite::vertexFormat() const
{
	return defaultVertexFormat();
}


} // sg
