#include "BSPStorage.h"
#include "BSPNode.h"
#include "BSPPolygon.h"
#include "config.h"

//-----------------------------------------------------------------------------

using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


BSPStorage::BSPStorage() :
	indexData( Allocator<int>(__FILE__) ),
	vertexData( Allocator<Vector3>(__FILE__) ),
	edgePlaneData( Allocator<Vector4>(__FILE__) ),
	nodePolygonData( Allocator<BSPPolygon*>(__FILE__) ),
	m_polyArrays( Allocator<P(BSPPolygonArray)>(__FILE__) ),
	m_polyCount( 0 ),
	m_polyAllocUnit( 200 ),
	m_nodeArrays( Allocator<P(BSPNodeArray)>(__FILE__) ),
	m_nodeCount( 0 ),
	m_nodeAllocUnit( 100 )
{
}

BSPPolygon* BSPStorage::createPolygon()
{
	if ( m_polyArrays.size() == 0 || m_polyArrays.lastElement()->used >= m_polyArrays.lastElement()->capacity )
		m_polyArrays.add( new BSPPolygonArray(m_polyAllocUnit) );

	BSPPolygonArray* a = m_polyArrays.lastElement();
	BSPPolygon* poly = &a->polys[a->used++];
	++m_polyCount;
	return poly;
}

BSPNode* BSPStorage::createNode( const Vector4& plane,
	const Vector<BSPPolygon*>& polys,
	BSPNode* pos, BSPNode* neg )
{
	if ( m_nodeArrays.size() == 0 || m_nodeArrays.lastElement()->used >= m_nodeArrays.lastElement()->capacity )
		m_nodeArrays.add( new BSPNodeArray(m_nodeAllocUnit) );

	BSPNodeArray* a = m_nodeArrays.lastElement();
	BSPNode* node = &a->nodes[a->used++];
	node->create( plane, polys, pos, neg, this );
	++m_nodeCount;
	return node;
}

void BSPStorage::setPolygonAllocationUnit( int size )
{
	if ( size > 1 )
		m_polyAllocUnit = size;
	else
		m_polyAllocUnit = 1;
}

void BSPStorage::setNodeAllocationUnit( int size )
{
	if ( size > 1 )
		m_nodeAllocUnit = size;
	else
		m_nodeAllocUnit = 1;
}

BSPNode* BSPStorage::getNode( int i ) const
{
	for ( int k = 0 ; k < m_nodeArrays.size() ; ++k )
	{
		const BSPNodeArray& a = *m_nodeArrays[k];
		if ( i >= 0 && i < a.used )
			return &a.nodes[i];
		i -= a.used;
	}

	assert( false ); // node not found
	return 0;
}

BSPPolygon* BSPStorage::getPolygon( int i ) const
{
	for ( int k = 0 ; k < m_polyArrays.size() ; ++k )
	{
		const BSPPolygonArray& a = *m_polyArrays[k];
		if ( i >= 0 && i < a.used )
			return &a.polys[i];
		i -= a.used;
	}

	assert( false ); // poly not found
	return 0;
}

int BSPStorage::getPolygonIndex( const BSPPolygon* poly ) const
{
	int baseIndex = 0;
	for ( int k = 0 ; k < m_polyArrays.size() ; ++k )
	{
		const BSPPolygonArray& a = *m_polyArrays[k];
		const BSPPolygon* begin = a.polys;
		const BSPPolygon* end = a.polys + a.used;
		
		if ( poly >= begin && poly < end )
			return baseIndex + poly-begin;

		baseIndex += a.used;
	}

	assert( false ); // poly not found
	return 0;
}

int BSPStorage::polygons() const
{
	return m_polyCount;
}

int BSPStorage::nodes() const
{
	return m_nodeCount;
}

//-----------------------------------------------------------------------------

BSPStorage::BSPPolygonArray::BSPPolygonArray( int newCapacity ) :
	polys( new BSPPolygon[newCapacity] ),
	used( 0 ),
	capacity( newCapacity )
{
	assert( newCapacity > 0 );
}

BSPStorage::BSPPolygonArray::~BSPPolygonArray()
{
	delete[] polys;
}

//-----------------------------------------------------------------------------

BSPStorage::BSPNodeArray::BSPNodeArray( int newCapacity ) :
	nodes( new BSPNode[newCapacity] ),
	used( 0 ),
	capacity( newCapacity )
{
	assert( newCapacity > 0 );
}

BSPStorage::BSPNodeArray::~BSPNodeArray()
{
	delete[] nodes;
}


} // bsp
