#include "MeshBuilder.h"
#include "Vertex.h"
#include "Polygon.h"
#include "VertexMap.h"
#include "VertexMapFormat.h"
#include "VertexReference.h"
#include "PolygonReference.h"
#include "DiscontinuousVertexMap.h"
#include <lang/String.h>
#include <util/Vector.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace mb
{


/** Internal representation of MeshBuilder. */
class MeshBuilder::MeshBuilderImpl :
	public Object
{
public:
	Vector<Polygon*>					polygons;
	Vector<Vertex*>						vertices;
	Vector<VertexMap*>					vertexMaps;
	Vector<DiscontinuousVertexMap*>		discontinuousVertexMaps;

	MeshBuilderImpl() :
		polygons( Allocator<Polygon*>(__FILE__,__LINE__) ),
		vertices( Allocator<Vertex*>(__FILE__,__LINE__) ),
		vertexMaps( Allocator<VertexMap*>(__FILE__,__LINE__) ),
		discontinuousVertexMaps( Allocator<DiscontinuousVertexMap*>(__FILE__,__LINE__) )
	{
	}

private:
	MeshBuilderImpl( const MeshBuilderImpl& );
	MeshBuilderImpl& operator=( const MeshBuilderImpl& );
};

//-----------------------------------------------------------------------------

/** Returns true if the floating point sequences are equal. */
inline static bool fequal( const float* a, const float* b, int count )
{
	for ( int i = 0 ; i < count ; ++i )
	{
		if ( a[i] != b[i] )
			return false;
	}
	return true;
}

//-----------------------------------------------------------------------------

MeshBuilder::MeshBuilder()
{
	m_this = new MeshBuilderImpl;
}

MeshBuilder::~MeshBuilder()
{
	clear();
}

void MeshBuilder::splitVertexDiscontinuities( DiscontinuousVertexMap* vmad )
{
	int dim = vmad->dimensions();
	if ( dim > 32 )
		dim = 32;
	float data0[32];
	float data1[32];

	for ( int i = 0 ; i < vertices() ; ++i )
	{
		Vertex* vert = getVertex(i);
		
		if ( vert->polygons() > 1 )
		{
			Polygon* poly0 = vert->getPolygon(0);
			bool poly0ok = vmad->getValue( vert->index(), poly0->index(), data0, dim );
			if ( poly0ok )
			{
				for ( int j = 1; j < vert->polygons() ; ++j )
				{
					Polygon* poly1 = vert->getPolygon(j);
					if ( poly1 != poly0 )
					{
						bool poly1ok = vmad->getValue( vert->index(), poly1->index(), data1, dim );
						if ( poly1ok )
						{
							if ( !fequal(data0,data1,dim) )
							{
								// discontinuity, split vertex
								mb::Vertex* vert2 = vert->clone();
								poly1->setVertex( poly1->getIndex(vert), vert2 );
								--j; // adjust index as vert has one poly less
							}
						}
					} // if ( poly1 != poly0 )
				}
			} // if ( poly0ok )
		}
	}

#ifdef _DEBUG
	// ensure succesful split
	for ( int i = 0 ; i < vertices() ; ++i )
	{
		Vertex* vert = getVertex(i);

		if ( vert->polygons() > 1 )
		{
			Polygon* poly0 = vert->getPolygon(0);
			bool poly0ok = vmad->getValue( vert->index(), poly0->index(), data0, dim );
			if ( poly0ok )
			{
				// check other vertex polys
				for ( int j = 1 ; j < vert->polygons() ; ++j )
				{
					Polygon* poly1 = vert->getPolygon(j);
					bool poly1ok = vmad->getValue( vert->index(), poly1->index(), data1, dim );
					if ( poly1ok )
					{
						assert( fequal(data0,data1,dim) );
					}
				}
			}
		} // if ( vert->polygons() > 1 )
	}
#endif
}

void MeshBuilder::clear()
{
	removeVertexMaps();
	removeDiscontinuousVertexMaps();

	for ( int index = polygons()-1 ; index >= 0 ; --index )
	{
		Polygon* item = getPolygon( index );
		item->destroy();
		freePolygon( item );
	}
	m_this->polygons.clear();

	for ( int index = vertices()-1 ; index >= 0 ; --index )
	{
		Vertex* item = getVertex( index );
		item->destroy();
		freeVertex( item );
	}
	m_this->vertices.clear();
}

VertexMap* MeshBuilder::addVertexMap( int dimensions, const String& name,
	const VertexMapFormat& format )
{
	VertexMap* item = allocateVertexMap();
	item->setDimensions( dimensions );
	item->setName( name );
	item->setFormat( format );
	m_this->vertexMaps.add( item );
	return item;
}

DiscontinuousVertexMap* MeshBuilder::addDiscontinuousVertexMap( int dimensions, 
	const String& name, const VertexMapFormat& format )
{
	DiscontinuousVertexMap* item = allocateDiscontinuousVertexMap();
	item->setDimensions( dimensions );
	item->setName( name );
	item->setFormat( format );
	m_this->discontinuousVertexMaps.add( item );
	return item;
}

Polygon* MeshBuilder::addPolygon()
{
	Polygon* item = allocatePolygon();
	item->create( this, m_this->polygons.size() );
	m_this->polygons.add( item );
	return item;
}

Vertex*	MeshBuilder::addVertex()
{
	Vertex* item = allocateVertex();
	item->create( this, m_this->vertices.size() );
	m_this->vertices.add( item );
	return item;
}

void MeshBuilder::removeVertexMap( int index )
{
	assert( (unsigned)index < (unsigned)m_this->vertexMaps.size() );
	
	VertexMap* vmap = m_this->vertexMaps[index];
	m_this->vertexMaps.remove( index );
	freeVertexMap( vmap );
}

void MeshBuilder::removeDiscontinuousVertexMap( int index )
{
	assert( (unsigned)index < (unsigned)m_this->discontinuousVertexMaps.size() );
	
	DiscontinuousVertexMap* vmad = m_this->discontinuousVertexMaps[index];
	m_this->discontinuousVertexMaps.remove( index );
	freeDiscontinuousVertexMap( vmad );
}

void MeshBuilder::removeVertexMaps()
{
	for ( int i = vertexMaps()-1 ; i >= 0 ; --i )
		removeVertexMap(i);
}

void MeshBuilder::removeDiscontinuousVertexMaps()
{
	for ( int i = discontinuousVertexMaps()-1 ; i >= 0 ; --i )
		removeDiscontinuousVertexMap(i);
}

void MeshBuilder::removeVertices()
{
	for ( int i = vertices()-1 ; i >= 0 ; --i )
		removeVertex(i);
}

void MeshBuilder::removePolygons()
{
	for ( int i = polygons()-1 ; i >= 0 ; --i )
		removePolygon(i);
}

void MeshBuilder::removePolygon( int index )
{
	Polygon* item = getPolygon(index);
	item->destroy();
	m_this->polygons.remove( index );
	freePolygon( item );

	// update polygon indices
	int i;
	for ( i = index ; i < (int)m_this->polygons.size() ; ++i )
		m_this->polygons[i]->m_index = i;

	// update discontinuous vertex map indices
	for ( i = 0 ; i < discontinuousVertexMaps() ; ++i )
		getDiscontinuousVertexMap(i)->polygonRemoved( index );
}

void MeshBuilder::removeVertex( int index )
{
	assert( 0 == getVertex(index)->polygons() ); // no polygon can use the vertex

	Vertex* item = getVertex(index);
	item->destroy();
	m_this->vertices.remove( index );
	freeVertex( item );

	// update vertex indices
	int i;
	for ( i = index ; i < (int)m_this->vertices.size() ; ++i )
		m_this->vertices[i]->m_index = i;

	// update discontinuous vertex map indices
	for ( i = 0 ; i < discontinuousVertexMaps() ; ++i )
		getDiscontinuousVertexMap(i)->vertexRemoved( index );

	// update discontinuous vertex map indices
	for ( i = 0 ; i < vertexMaps() ; ++i )
		getVertexMap(i)->vertexRemoved( index );
}

Polygon* MeshBuilder::getPolygon( int index )
{
	assert( (unsigned)index < (unsigned)m_this->polygons.size() );
	return m_this->polygons[index];
}

Vertex* MeshBuilder::getVertex( int index )
{
	assert( (unsigned)index < (unsigned)m_this->vertices.size() );
	return m_this->vertices[index];
}

VertexMap* MeshBuilder::getVertexMap( int index )
{
	assert( (unsigned)index < (unsigned)m_this->vertexMaps.size() );
	return m_this->vertexMaps[index];
}

DiscontinuousVertexMap* MeshBuilder::getDiscontinuousVertexMap( int index )
{
	assert( (unsigned)index < (unsigned)m_this->discontinuousVertexMaps.size() );
	return m_this->discontinuousVertexMaps[index];
}

VertexMap* MeshBuilder::getVertexMapByName( const String& name )
{
	int i;
	for ( i = 0 ; i < (int)m_this->vertexMaps.size() ; ++i )
	{
		VertexMap* item = m_this->vertexMaps[i];
		if ( item->name() == name )
			return item;
	}

	return 0;
}

DiscontinuousVertexMap* MeshBuilder::getDiscontinuousVertexMapByName( const String& name )
{
	int i;
	for ( i = 0 ; i < (int)m_this->discontinuousVertexMaps.size() ; ++i )
	{
		DiscontinuousVertexMap* item = m_this->discontinuousVertexMaps[i];
		if ( item->name() == name )
			return item;
	}

	return 0;
}

int MeshBuilder::polygons() const
{
	return m_this->polygons.size();
}

int MeshBuilder::vertices() const
{
	return m_this->vertices.size();
}

int MeshBuilder::vertexMaps() const
{
	return m_this->vertexMaps.size();
}

int MeshBuilder::discontinuousVertexMaps() const
{
	return m_this->discontinuousVertexMaps.size();
}

VertexReference* MeshBuilder::allocateVertexReference()
{
	return new VertexReference;
}

void MeshBuilder::freeVertexReference( VertexReference* item )
{
	delete item;
}

PolygonReference* MeshBuilder::allocatePolygonReference()
{
	return new PolygonReference;
}

void MeshBuilder::freePolygonReference( PolygonReference* item )
{
	delete item;
}

VertexMap* MeshBuilder::allocateVertexMap()
{
	return new VertexMap;
}

void MeshBuilder::freeVertexMap( VertexMap* item )
{
	delete item;
}

DiscontinuousVertexMap* MeshBuilder::allocateDiscontinuousVertexMap()
{
	return new DiscontinuousVertexMap;
}

void MeshBuilder::freeDiscontinuousVertexMap( DiscontinuousVertexMap* item )
{
	delete item;
}

Polygon* MeshBuilder::allocatePolygon()
{
	return new Polygon;
}

void MeshBuilder::freePolygon( Polygon* item )
{
	delete item;
}

Vertex* MeshBuilder::allocateVertex()
{
	return new Vertex;
}

void MeshBuilder::freeVertex( Vertex* item )
{
	delete item;
}

void MeshBuilder::removeUnusedVertices()
{
	for ( int k = 0 ; k < vertices() ; )
	{
		Vertex* vert = getVertex( k );
		if ( 0 == vert->polygons() )
			removeVertex( k );
		else
			++k;
	}
}

bool MeshBuilder::operator==( const MeshBuilder& other ) const
{
	if ( vertices() != other.vertices() )
		return false;
	
	if ( polygons() != other.polygons() )
		return false;
	
	if ( vertexMaps() != other.vertexMaps() )
		return false;
	
	if ( discontinuousVertexMaps() != other.discontinuousVertexMaps() )
		return false;
	
	for ( int i = 0 ; i < m_this->vertices.size() ; ++i )
		if ( *m_this->vertices[i] != *other.m_this->vertices[i] )
			return false;

	for ( int i = 0 ; i < m_this->polygons.size() ; ++i )
		if ( *m_this->polygons[i] != *other.m_this->polygons[i] )
			return false;

	for ( int i = 0 ; i < m_this->vertexMaps.size() ; ++i )
		if ( *m_this->vertexMaps[i] != *other.m_this->vertexMaps[i] )
			return false;

	for ( int i = 0 ; i < m_this->discontinuousVertexMaps.size() ; ++i )
		if ( *m_this->discontinuousVertexMaps[i] != *other.m_this->discontinuousVertexMaps[i] )
			return false;

	return true;
}

bool MeshBuilder::operator!=( const MeshBuilder& other ) const
{
	return !(*this == other);
}


} // mb
