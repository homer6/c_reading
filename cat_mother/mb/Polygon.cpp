#include "Polygon.h"
#include "Vertex.h"
#include "MeshBuilder.h"
#include "VertexReference.h"
#include "DiscontinuousVertexMap.h"
#include "Vec3.h"
#include <lang/Float.h>
#include <lang/Math.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace mb
{


Polygon::Polygon()
{
	m_mesh			= 0;
	m_index			= -1;
	m_lastVertex	= 0;
	m_vertices		= 0;
	m_normalValid	= false;
	m_matid			= 0;
}

Polygon::~Polygon()
{
	destroy();
}

Polygon* Polygon::clone() const
{
	assert( m_mesh );

	Polygon* newPolygon = m_mesh->addPolygon();
	
	newPolygon->m_matid = m_matid;
	
	int i;
	for ( i = 0 ; i < vertices() ; ++i )
		newPolygon->addVertex( const_cast<Vertex*>( getVertex(i) ) );

	for ( i = 0 ; i < m_mesh->discontinuousVertexMaps() ; ++i )
	{
		DiscontinuousVertexMap* vmad = m_mesh->getDiscontinuousVertexMap(i);
		int dim = vmad->dimensions();
		float data[16];

		for ( int j = 0 ; j < vertices() ; ++j )
		{
			const Vertex* vert = getVertex(j);
			int vertIndex = vert->index();

			if ( dim <= 16 && vmad->getValue( vertIndex, index(), data, dim ) )
				vmad->addValue( vertIndex, newPolygon->index(), data, dim );
		}
	}

	return newPolygon;
}

void Polygon::split( int v0, int v1 )
{
	assert( vertices() > 3 );
	assert( v0 != v1 );
	assert( v0 >= 0 && v0 < vertices() );
	assert( v1 >= 0 && v1 < vertices() );
	assert( (v0+1)%vertices() != v1 && (v1+1)%vertices() != v0 );

	// ensure v0 comes before v1
	if ( v0 > v1 )
	{
		int tmp = v0;
		v0 = v1;
		v1 = tmp;
	}

	Polygon* other = clone();
	other->removeVertices( v0+1, v1 );

	removeVertices( v1+1, vertices() );
	removeVertices( 0, v0 );
}

void Polygon::create( MeshBuilder* mesh, int index )
{
	m_mesh			= mesh;
	m_index			= index;
	m_lastVertex	= 0;
	m_vertices		= 0;
	m_normalValid	= false;
	m_matid			= 0;
}

void Polygon::destroy()
{
	removeVertices();
}

void Polygon::addVertex( Vertex* vertex )
{
	assert( m_mesh );
	//assert( !hasVertex(vertex) );

	VertexReference* vref = m_mesh->allocateVertexReference();
	vref->vertex = vertex;

	if ( m_lastVertex ) 
		m_lastVertex->append( vref );
	m_lastVertex = vref;
	++m_vertices;

	vertex->addPolygon( this );
	m_normalValid = false;
}

void Polygon::removeVertex( Vertex* vertex )
{
	assert( m_mesh );

	VertexReference* vref = findRef( vertex );
	
	if ( m_lastVertex == vref )
		m_lastVertex = static_cast<VertexReference*>( vref->previous() );
	vref->unlink();
	--m_vertices;

	vref->vertex->removePolygon( this );
	m_mesh->freeVertexReference( vref );
	m_normalValid = false;
}

void Polygon::removeVertices( int v0, int v1 )
{
	assert( v0 <= v1 );
	assert( v0 >= 0 );
	assert( v1 >= 0 );
	assert( v1 <= vertices() );

	for ( int i = v1-1 ; i >= v0 ; --i )
		removeVertex( getVertex(i) );
}

void Polygon::removeVertices()
{
	removeVertices( 0, vertices() );
}

Vertex* Polygon::getVertex( int index )
{
	assert( m_mesh );
	assert( index >= 0 && index < m_vertices );

	VertexReference* vref = findRef( index );
	return vref->vertex;
}

const Vertex* Polygon::getVertex( int index ) const
{
	assert( m_mesh );
	assert( index >= 0 && index < m_vertices );

	VertexReference* vref = findRef( index );
	return vref->vertex;
}

int	Polygon::getIndex( const Vertex* vertex ) const
{
	int index = -1;
	findRef( vertex, &index );
	return index;
}

void Polygon::setVertex( int index, Vertex* vertex )
{
	assert( m_mesh );
	assert( index >= 0 && index < m_vertices );

	VertexReference* vref = findRef( index );
	Vertex* oldVertex = vref->vertex;
	vref->vertex = vertex;
	
	if ( oldVertex != vertex )
	{
		oldVertex->removePolygon( this );
		vertex->addPolygon( this );
	}

	m_normalValid = false;
}

int Polygon::vertices() const
{
	assert( m_mesh );

	return m_vertices;
}

int Polygon::index() const
{
	return m_index;
}

bool Polygon::isUsingVertex( const Vertex* vertex ) const
{
	VertexReference* vref = m_lastVertex;
	while ( vref && vref->vertex != vertex )
		vref = static_cast<VertexReference*>( vref->previous() );

	return 0 != vref;
}

VertexReference* Polygon::findRef( int index ) const
{
	VertexReference* vref = m_lastVertex;
	for ( int i = m_vertices-1 ; i != index ; --i )
		vref = static_cast<VertexReference*>( vref->previous() );
	
	assert( vref );
	assert( vref->vertex );
	return vref;
}

VertexReference* Polygon::findRef( const Vertex* vertex, int* index ) const
{
	int vrefIndex = m_vertices - 1;
	VertexReference* vref = m_lastVertex;
	while ( vref && vref->vertex != vertex )
	{
		vref = static_cast<VertexReference*>( vref->previous() );
		--vrefIndex;
	}

	assert( vref );
	assert( vref->vertex );
	if ( index )
		*index = vrefIndex;
	return vref;
}

void Polygon::getNormal( float* x, float* y, float* z ) const
{
	if ( !m_normalValid )
	{
		if ( vertices() < 3 )
		{
			m_normal[0] = 0.f;
			m_normal[1] = 0.f;
			m_normal[2] = 0.f;
		}
		else
		{
			float v0[3]; 
			getVertex(0)->getPosition( &v0[0], &v0[1], &v0[2] );

			float v1[3];
			getVertex(1)->getPosition( &v1[0], &v1[1], &v1[2] );

			float v2[3];
			getVertex( vertices()-1 )->getPosition( &v2[0], &v2[1], &v2[2] );

			float e0[3];
			Vec3::sub( e0, v1, v0 );

			float e1[3];
			Vec3::sub( e1, v2, v0 );

			float pn[3];
			Vec3::cross( pn, e0, e1 );

			float lensqr = Vec3::dot(pn,pn);
			float s = 0.f;
			if ( lensqr > Float::MIN_VALUE )
				s = 1.f / Math::sqrt(lensqr);
			Vec3::scale( m_normal, pn, s );
		}
		m_normalValid = true;
	}
	
	*x = m_normal[0];
	*y = m_normal[1];
	*z = m_normal[2];
}

void Polygon::setMaterial( int id )
{
	m_matid = id;
}

int	Polygon::material() const
{
	return m_matid;
}

bool Polygon::hasVertex( const Vertex* vertex ) const
{
	VertexReference* vref = m_lastVertex;
	while ( vref && vref->vertex != vertex )
		vref = static_cast<VertexReference*>( vref->previous() );
	return 0 != vref;
}

float Polygon::getAngle( const Polygon* poly ) const
{
	float n1[3];
	float n2[3];
	getNormal( n1+0, n1+1, n1+2 );
	poly->getNormal( n2+0, n2+1, n2+2 );

	float d = Vec3::dot( n1, n2 );
	if ( d < -1.f )
		d = -1.f;
	else if ( d > 1.f )
		d = 1.f;

	float an = Math::acos( d );
	return an;
}

bool Polygon::operator==( const Polygon& other ) const
{
	VertexReference* vref1 = m_lastVertex;
	VertexReference* vref2 = other.m_lastVertex;
	while ( vref1 && vref2 )
	{
		if ( vref1->vertex->index() != vref2->vertex->index() )
			return false;

		vref1 = static_cast<VertexReference*>( vref1->previous() );
		vref2 = static_cast<VertexReference*>( vref2->previous() );
	}

	return !vref1 && !vref2;
}

bool Polygon::operator!=( const Polygon& other ) const
{
	return !(*this == other);
}


} // mb
