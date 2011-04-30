#include "BSPPolygon.h"
#include "BSPStorage.h"
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


BSPPolygon::BSPPolygon( const math::Vector3* v, int n, int id,
	BSPStorage* storage, int collisionMask ) :
	m_id(id), 
	m_firstIndex( storage->indexData.size() ), 
	m_firstEdgePlane( storage->edgePlaneData.size() ),
	m_vertices(n),
	m_storage(storage),
	m_collisionMask(collisionMask)
{
	assert( n >= 3 );
	assert( storage->indexData.size() == storage->edgePlaneData.size() );

	// add points and indices
	for ( int i = 0 ; i < n ; ++i )
	{
		storage->indexData.add( storage->vertexData.size() );
		storage->vertexData.add( v[i] );
	}

	computePlanes();
}

BSPPolygon::BSPPolygon( const int* indices, int n, int id,
	BSPStorage* storage, int collisionMask )
{
	create( indices, n, id, storage, collisionMask );
}

void BSPPolygon::create( const int* indices, int n, int id,
	BSPStorage* storage, int collisionMask )
{
	assert( n >= 3 );
	assert( storage->indexData.size() == storage->edgePlaneData.size() );

	m_id = id;
	m_firstIndex = storage->indexData.size();
	m_firstEdgePlane = storage->edgePlaneData.size();
	m_vertices = n;
	m_storage = storage;
	m_collisionMask = collisionMask;

	// add indices
	for ( int i = 0 ; i < n ; ++i )
		storage->indexData.add( indices[i] );

	computePlanes();
}

void BSPPolygon::computePlanes()
{
	// compute plane eq
	Vector3 v0 = getVertex( 0 );
	Vector3 v1 = getVertex( 1 );
	Vector3 v2 = getVertex( 2 );

	Vector3 normal = ( v1-v0 ).cross( v2-v0 );
	float nlen = normal.length();
	if ( nlen > Float::MIN_VALUE )
		normal *= 1.f / nlen;
	else
		normal = Vector3(0.f,0.f,0.f);

	m_plane = Vector4( normal.x, normal.y, normal.z, -v0.dot(normal) );

	// add edge planes
	int j = m_vertices - 1;
	for ( int i = 0 ; i < m_vertices ; j = i++ )
	{
		Vector3 v0 = getVertex(j);
		Vector3 edge = getVertex(i) - v0;
		Vector3 edgeNormal = edge.cross( normal ).normalize();
		float edgeDist = -v0.dot(edgeNormal);
		Vector4 edgePlane( edgeNormal.x, edgeNormal.y, edgeNormal.z, edgeDist );
		m_storage->edgePlaneData.add( edgePlane );
	}

	// compute bound sphere
	Vector3 boundSphereCenter(0,0,0);
	for ( int i = 0 ; i < m_vertices ; ++i )
		boundSphereCenter += getVertex(i);
	boundSphereCenter *= 1.f / (float)m_vertices;
	m_boundSphereRadiusSqr = 0.f;
	for ( int i = 0 ; i < m_vertices ; ++i )
	{
		float radiusSqr = (boundSphereCenter - getVertex(i)).lengthSquared();
		if ( radiusSqr > m_boundSphereRadiusSqr )
			m_boundSphereRadiusSqr = radiusSqr;
	}
	for ( int i = 0 ; i < 3 ; ++i )
		m_boundSphereCenter[i] = boundSphereCenter[i];
}

bool BSPPolygon::isPointInPolygon( const Vector3& point ) const
{
	assert( vertices() >= 3 );

	for ( int i = 0 ; i < m_vertices ; ++i )
	{
		const Vector4& ep = m_storage->edgePlaneData.get( m_firstEdgePlane+i );
		float d = point.x*ep.x + point.y*ep.y + point.z*ep.z + ep.w;
		if ( d > 0.f )
			return false;
	}

	return true;
}

bool BSPPolygon::operator==( const BSPPolygon& other ) const
{
	if ( m_vertices != other.m_vertices )
		return false;
	if ( m_collisionMask != other.m_collisionMask )
		return false;
	if ( m_id != other.m_id )
		return false;
	for ( int i = 0 ; i < m_vertices ; ++i )
		if ( getVertex(i) != other.getVertex(i) )
			return false;
	return true;
}


} // bsp
