#include "LineList.h"
#include "Context.h"
#include "VertexLock.h"
#include "Material.h"
#include "VertexFormat.h"
#include "GdUtil.h"
#include "LockException.h"
#include <gd/LockMode.h>
#include <gd/Primitive.h>
#include <gd/VertexFormat.h>
#include <gd/GraphicsDriver.h>
#include <pix/Color.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Exception.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


static gd::LockMode togd( LineList::LockType lock )
{
	switch ( lock )
	{
	case LineList::LOCK_READ:		return gd::LockMode::LOCK_READ;
	case LineList::LOCK_WRITE:		return gd::LockMode::LOCK_WRITE;
	default:						return gd::LockMode::LOCK_READWRITE;
	};
}

//-----------------------------------------------------------------------------

LineList::LineList( int maxLines, LinesType type ) :
	m_lines( 0 ),
	m_count( 0 ),
	m_lock( (LockType)-1 ),
	m_maxLines( maxLines ),
	m_type( type )
{
	assert( maxLines > 0 );

	VertexFormat vf;
	if ( type == LINES_2D )
		vf.addRHW();
	vf.addDiffuse();
	gd::VertexFormat gdvf;
	GdUtil::togd( vf, &gdvf );

	m_lines = Context::driver()->createPrimitive();
	int err = m_lines->create( Context::device(), gd::Primitive::PRIMITIVE_LINELIST, 2*maxLines, 0, gdvf, gd::Primitive::USAGE_STATIC );
	if ( err )
		throw Exception( Format("Failed to create rendering device line list") );

	P(Material) mat = new Material;
	mat->setDiffuseColorSource( sg::Material::MCS_COLOR1 );
	mat->setVertexColor( true );
	mat->setBlend( sg::Material::BLEND_SRCALPHA, sg::Material::BLEND_INVSRCALPHA );
	mat->setLighting( false );
	mat->setVertexFormat( vf );
	if ( type == LINES_2D )
		mat->setDepthFunc( Material::CMP_ALWAYS );
	setShader( mat );
}

LineList::LineList( const LineList& other, int shareFlags ) :
	Primitive( other, shareFlags ),
	m_lines( other.m_lines ),
	m_count( other.m_count ),
	m_lock( other.m_lock ),
	m_maxLines( other.m_maxLines ),
	m_type( other.m_type )
{
	assert( shareFlags & SHARE_GEOMETRY ); shareFlags;
}

Primitive* LineList::clone( int shareFlags ) const
{
	return new LineList( *this, shareFlags );
}

LineList::~LineList()
{
}

void LineList::destroy()
{
	m_lines = 0;
	Primitive::destroy();
}

void LineList::load()
{
	if ( m_lines && Context::device() )
		m_lines->load( Context::device() );
}

void LineList::unload()
{
	if ( m_lines )
		m_lines->unload();
}

void LineList::draw()
{
	assert( m_lines );
	assert( shader() );

	if ( 0 == m_count )
		return;

	Shader*	shader = this->shader();
	const int passes = shader->begin();
	for ( int pass = 0 ; pass < passes ; ++pass )
	{
		shader->apply( pass );
		m_lines->draw( Context::device(), 0, m_count*2, 0, 0 );
	}
	shader->end();
}
	
void LineList::addLine( const math::Vector3& start, const math::Vector3& end, const pix::Color& color )
{
	addLine( start, end, color, color );
}

void LineList::addLine( const Vector3& start, const Vector3& end, const pix::Color& startColor, const pix::Color& endColor )
{
	assert( m_lines );
	assert( m_lock == LOCK_WRITE || m_lock == LOCK_READWRITE );
	//assert( m_count+1 <= m_maxLines );

	if ( m_count+1 <= m_maxLines )
	{
		if ( m_type == LINES_2D )
		{
			Vector4 points[2];
			points[0] = Vector4( start.x, start.y, 0, 1 );
			points[1] = Vector4( end.x, end.y, 0, 1 );;
			m_lines->setVertexPositionsRHW( m_count*2, points, 2 );
		}
		else
		{
			Vector3 points[2];
			points[0] = start;
			points[1] = end;
			m_lines->setVertexPositions( m_count*2, points, 2 );
		}

		Color colors[2];
		colors[0] = startColor;
		colors[1] = endColor;
		m_lines->setVertexDiffuseColors( m_count*2, colors, 2 );

		++m_count;
	}
}

void LineList::getLine( int i, math::Vector3* start, math::Vector3* end ) const
{
	assert( m_lines );
	assert( m_lock == LOCK_READ || m_lock == LOCK_READWRITE );
	assert( i >= 0 && i < lines() );

	if ( m_type == LINES_2D )
	{
		Vector4 points[2];
		m_lines->getVertexPositionsRHW( i*2, points, 2 );
		*start = Vector3( points[0].x, points[0].y, points[0].z );
		*end = Vector3( points[1].x, points[1].y, points[1].z );
	}
	else
	{
		Vector3 points[2];
		m_lines->getVertexPositions( i*2, points, 2 );
		*start = Vector3( points[0].x, points[0].y, points[0].z );
		*end = Vector3( points[1].x, points[1].y, points[1].z );
	}
}

void LineList::addLines( const Vector3* positions, const pix::Color* colors, int count )
{
	assert( m_lines );
	assert( m_lock == LOCK_WRITE || m_lock == LOCK_READWRITE );
	assert( m_count+count <= maxLines() );

	for ( int i = 0 ; i < count ; i += 2 )
		addLine( positions[i], positions[i+1], colors[i], colors[i+1] );
}

void LineList::removeLines()
{
	assert( m_lines );
	m_count = 0;
}

float LineList::boundSphere() const
{
	assert( m_lines );
	assert( !verticesLocked() );

	const int lines = this->lines();
	if ( 0 == lines )
		return 0.f;

	VertexLock<LineList> lock( const_cast<LineList*>(this), LOCK_READ );
	
	float maxr2 = 0.f;
	const int buffsize = 32;
	Vector3 buff[buffsize*2];

	for ( int i = 0 ; i < lines ; )
	{
		int count = lines-i;
		if ( count > buffsize )
			count = buffsize;

		m_lines->getVertexPositions( i*2, buff, count*2 );

		Vector3* v1 = buff + count*2;
		for ( Vector3* v = buff ; v < v1 ; ++v )
		{
			float r2 = v->lengthSquared();
			if ( r2 > maxr2 )
				maxr2 = r2;
		}
		
		i += count;
	}

	return Math::sqrt( maxr2 );
}

int LineList::lines() const
{
	assert( m_lines );
	return m_count;
}

int LineList::maxLines() const
{
	assert( m_lines );
	return m_maxLines;
}

void LineList::lockVertices( LockType lock )
{
	assert( m_lines );

	if ( !m_lines->lockVertices( Context::device(), togd(lock) ) )
		throw LockException( "LineList" );
	m_lock = lock;
}

void LineList::unlockVertices()
{
	assert( m_lines );

	m_lines->unlockVertices();
	m_lock = (LockType)(-1);
}

bool LineList::verticesLocked() const
{
	assert( m_lines );
	return m_lines->verticesLocked();
}

VertexFormat LineList::vertexFormat() const
{
	VertexFormat vf;
	if ( m_lines )
		GdUtil::tosg( m_lines->vertexFormat(), &vf );
	return vf;
}


} // sg
