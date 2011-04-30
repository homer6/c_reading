#include "PolygonAdjacency.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace util;

//-----------------------------------------------------------------------------

namespace sg
{


PolygonAdjacency::PolygonAdjacency() :
	m_adj( Allocator<int>(__FILE__,__LINE__) )
{
	m_edges = 0;
}

PolygonAdjacency::~PolygonAdjacency()
{
}

void PolygonAdjacency::setPolygons( int polys, int edgesPerPolygon )
{
	assert( edgesPerPolygon >= 3 );
	assert( polys >= 0 );

	m_adj.clear();
	m_adj.setSize( polys*edgesPerPolygon, -1 );
	m_edges = edgesPerPolygon;
}

void PolygonAdjacency::setAdjacent( int poly, const int* adj, int edges )
{
	assert( poly >= 0 && poly*m_edges < m_adj.size() );
	assert( edges == m_edges );

	int* p = m_adj.begin() + poly*edges;
	for ( int i = 0 ; i < edges ; ++i )
		p[i] = adj[i];
}

void PolygonAdjacency::clear()
{
	m_adj.clear();
	m_adj.trimToSize();
}

int PolygonAdjacency::polygons() const
{
	if ( m_edges > 0 )
		return m_adj.size()/m_edges;
	else
		return 0;
}


} // sg
