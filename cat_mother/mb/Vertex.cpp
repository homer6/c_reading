#include "Vertex.h"
#include "Polygon.h"
#include "VertexMap.h"
#include "MeshBuilder.h"
#include "PolygonReference.h"
#include "DiscontinuousVertexMap.h"
#include <lang/Float.h>
#include <lang/Math.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace mb
{


Vertex::Vertex()
{
	m_index			= -1;
	m_mesh			= 0;
	m_lastPolygon	= 0;
	m_polygons		= 0;
}

Vertex::~Vertex()
{
	destroy();
}

Vertex* Vertex::clone() const
{
	assert( m_mesh );

	Vertex* newVertex = m_mesh->addVertex();
	
	newVertex->setPosition( m_position[0], m_position[1], m_position[2] );

	int i;
	for ( i = 0 ; i < m_mesh->vertexMaps() ; ++i )
	{
		VertexMap* vmap = m_mesh->getVertexMap(i);
		int dim = vmap->dimensions();
		float data[16];

		if ( dim <= 16 && vmap->getValue( index(), data, dim ) )
			vmap->addValue( newVertex->index(), data, dim );
	}

	for ( i = 0 ; i < m_mesh->discontinuousVertexMaps() ; ++i )
	{
		DiscontinuousVertexMap* vmad = m_mesh->getDiscontinuousVertexMap(i);
		int dim = vmad->dimensions();
		float data[16];

		for ( int j = 0 ; j < polygons() ; ++j )
		{
			const Polygon* poly = getPolygon(j);
			int polyIndex = poly->index();

			if ( dim <= 16 && vmad->getValue( index(), polyIndex, data, dim ) )
				vmad->addValue( newVertex->index(), polyIndex, data, dim );
		}
	}

	return newVertex;
}

void Vertex::setPosition( float x, float y, float z )
{
	m_position[0] = x;
	m_position[1] = y;
	m_position[2] = z;
}

/*void Vertex::setIndex( int i )
{
	m_index = i;
}*/

void Vertex::getPosition( float* x, float* y, float* z ) const
{
	*x = m_position[0];
	*y = m_position[1];
	*z = m_position[2];
}

void Vertex::create( MeshBuilder* mesh, int index )
{
	m_index			= index;
	m_mesh			= mesh;
	m_lastPolygon	= 0;
	m_polygons		= 0;
}

void Vertex::destroy()
{
	removePolygons();
}

void Vertex::addPolygon( Polygon* polygon )
{
	assert( m_mesh );

	PolygonReference* pref = m_mesh->allocatePolygonReference();
	pref->polygon = polygon;

	if ( m_lastPolygon ) 
		m_lastPolygon->append( pref );
	m_lastPolygon = pref;

	++m_polygons;
}

void Vertex::removePolygons()
{
	for ( int i = polygons()-1 ; i >= 0 ; --i )
		removePolygon( getPolygon(i) );
}

void Vertex::removePolygon( Polygon* polygon )
{
	assert( m_mesh );
	//assert( !polygon->isUsingVertex(this) );	// vertex must be removed first from the polygon

	PolygonReference* pref = findRef( polygon );

	if ( m_lastPolygon == pref ) 
		m_lastPolygon = static_cast<PolygonReference*>( pref->previous() );
	pref->unlink();
	--m_polygons;

	m_mesh->freePolygonReference( pref );
}

PolygonReference* Vertex::findRef( Polygon* polygon )
{
	PolygonReference* pref = m_lastPolygon;
	for ( int i = m_polygons-1 ; i >= 0 ; )
	{
		if ( pref->polygon == polygon )
			break;

		--i;
		pref = static_cast<PolygonReference*>( pref->previous() );
	}
	return pref;
}

Polygon* Vertex::getPolygon( int index )
{
	assert( m_mesh );
	assert( index >= 0 && index < m_polygons );

	PolygonReference* pref = m_lastPolygon;
	for ( int i = m_polygons-1 ; i != index ; --i )
		pref = static_cast<PolygonReference*>( pref->previous() );

	return pref->polygon;
}

const Polygon* Vertex::getPolygon( int index ) const
{
	assert( m_mesh );
	assert( index >= 0 && index < m_polygons );

	PolygonReference* pref = m_lastPolygon;
	for ( int i = m_polygons-1 ; i != index ; --i )
		pref = static_cast<PolygonReference*>( pref->previous() );

	return pref->polygon;
}

int Vertex::polygons() const
{
	assert( m_mesh );

	return m_polygons;
}

int Vertex::index() const
{
	return m_index;
}

void Vertex::getNormal( float* x, float* y, float* z ) const
{
	float n[3] = {0,0,0};

	PolygonReference* pref = m_lastPolygon;
	while ( pref ) 
	{
		float n1[3];
		pref->polygon->getNormal( n1, n1+1, n1+2 );
		for ( int i = 0 ; i < 3 ; ++i )
			n[i] += n1[i];
		
		pref = static_cast<PolygonReference*>( pref->previous() );
	}

	float lensqr = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
	if ( lensqr > Float::MIN_VALUE )
	{
		float s = 1.f / Math::sqrt(lensqr);
		for ( int i = 0 ; i < 3 ; ++i )
			n[i] *= s;
	}
	else
	{
		for ( int i = 0 ; i < 3 ; ++i )
			n[i] = 0.f;
	}

	*x = n[0];
	*y = n[1];
	*z = n[2];
}

bool Vertex::operator==( const Vertex& other ) const
{
	return 
		m_position[0] == other.m_position[0] &&
		m_position[1] == other.m_position[1] &&
		m_position[2] == other.m_position[2];
}

bool Vertex::operator!=( const Vertex& other ) const
{
	return !(*this == other);
}


} // mb
