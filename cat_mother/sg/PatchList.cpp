#include "PatchList.h"
#include "Edge.h"
#include "EdgeHash.h"
#include "VertexLock.h"
#include <sg/LineList.h>
#include <pix/Color.h>
#include <math/BezierUtil.h>
#include <util/Hashtable.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define LOCK_NONE LockType(-1)

//-----------------------------------------------------------------------------

using namespace pix;
using namespace math;
using namespace util;

//-----------------------------------------------------------------------------

static bool		s_drawControlPolygons	= false;

//-----------------------------------------------------------------------------

/*static Vector3 getValue( const Vector3* patch, float u, float v )
{
	assert( u >= 0.f && u <= 1.f );
	assert( v >= 0.f && v <= 1.f );

	return BezierUtil<Vector3>::bezier( 
		patch[0*4+0], patch[0*4+1], patch[0*4+2], patch[0*4+3],
		patch[1*4+0], patch[1*4+1], patch[1*4+2], patch[1*4+3],
		patch[2*4+0], patch[2*4+1], patch[2*4+2], patch[2*4+3],
		patch[3*4+0], patch[3*4+1], patch[3*4+2], patch[3*4+3],
		u, v );
}*/

//-----------------------------------------------------------------------------

namespace sg
{


PatchList::PatchList( int vertices ) :
	m_vertices( Allocator<Vector3>(__FILE__,__LINE__) ),
	m_lock( LOCK_NONE ),
	m_adj(),
	m_adjZeroDistance( -1.f )
{
	assert( vertices > 0 && vertices % 16 == 0 );
	m_vertices.setSize( vertices );
}

PatchList::PatchList( const PatchList& other, int shareFlags ) :
	Primitive( other, shareFlags ),
	m_vertices( other.m_vertices ),
	m_lock( other.m_lock ),
	m_adj( other.m_adj ),
	m_adjZeroDistance( other.m_adjZeroDistance )
{
}

Primitive* PatchList::clone( int shareFlags ) const
{
	return new PatchList( *this, shareFlags );
}

void PatchList::destroy()
{
	m_vertices.clear();
	m_vertices.trimToSize();
	Primitive::destroy();
}

void PatchList::load()
{
}

void PatchList::unload()
{
}

void PatchList::draw()
{
	// DEBUG: draw lines
	if ( s_drawControlPolygons )
	{
		static LineList debugLines( 500, LineList::LINES_3D );
		{
			VertexLock<LineList> lkLines( &debugLines, LineList::LOCK_WRITE );
			VertexLock<PatchList> lkPatches( this, LOCK_READ );

			debugLines.removeLines();
			for ( int k = 0 ; k < m_vertices.size() ; k += 16 )
			{
				Color colr(255,0,0);

				// draw control polygon
				Vector3* patch = &m_vertices[k];
				for ( int i = 0 ; i < 4 ; ++i )
				{
					for ( int j = 0 ; j < 4 ; ++j )
					{
						if ( debugLines.lines()+2 <= debugLines.maxLines() )
						{
							if ( i+1 < 4 )
								debugLines.addLine( patch[i*4+j], patch[(i+1)*4+j], colr );
							if ( j+1 < 4 )
								debugLines.addLine( patch[i*4+j], patch[i*4+(j+1)], colr );
						}
					}
				}

				// draw line to adjacent control polygons
				colr = Color(255,255,0);
				int iprev = 4-1;
				int adj[4];
				getPolygonAdjacency(0.1f).getAdjacent( k/16, adj, 4 );
				for ( int i = 0 ; i < 4 ; iprev = i++ )
				{
					if ( adj[i] != -1 && debugLines.lines() < debugLines.maxLines() )
						debugLines.addLine( patch[0], m_vertices[adj[i]*16], colr );
				}
			}
		}
		debugLines.draw();
	}
}

void PatchList::setVertexPositions( int firstVertex, const Vector3* positions, int count )
{
	assert( m_lock == LOCK_WRITE || m_lock == LOCK_READWRITE );
	assert( firstVertex >= 0 && firstVertex < vertices() );
	assert( count > 0 && firstVertex+count <= vertices() );

	Vector3* v = m_vertices.begin() + firstVertex;
	for ( int i = 0 ; i < count ; ++i )
		v[i] = positions[i];
}

void PatchList::getVertexPositions( int firstVertex, Vector3* positions, int count ) const
{
	assert( m_lock == LOCK_READ || m_lock == LOCK_READWRITE );
	assert( firstVertex >= 0 && firstVertex < vertices() );
	assert( count > 0 && firstVertex+count <= vertices() );

	const Vector3* v = m_vertices.begin() + firstVertex;
	for ( int i = 0 ; i < count ; ++i )
		positions[i] = v[i];
}

void PatchList::lockVertices( LockType lock )
{
	assert( !verticesLocked() );

	m_lock = lock;
	if ( lock != LOCK_READ )
		m_adjZeroDistance = -1.f;
}

void PatchList::unlockVertices()
{
	assert( verticesLocked() );
	m_lock = LOCK_NONE;
}

int PatchList::vertices() const
{
	return m_vertices.size();
}

bool PatchList::verticesLocked() const
{
	return m_lock != LOCK_NONE;
}

const PolygonAdjacency&	PatchList::getPolygonAdjacency( float zeroDistance ) const
{
	assert( zeroDistance >= 0.f );
	assert( m_lock == LOCK_READ || m_lock == LOCK_READWRITE );

	if ( m_adjZeroDistance != zeroDistance )
	{
		// add edges to the hashtable
		Hashtable<Edge,int,EdgeHash,EdgeHash> edges( (vertices()/16)*2+1, 0.75f, -1, EdgeHash(zeroDistance), EdgeHash(zeroDistance), Allocator< HashtablePair<Edge,int> >(__FILE__,__LINE__) );
		int faceindex = 0;
		for ( int i = 0 ; i < m_vertices.size() ; i += 16 )
		{
			const Vector3* patch = &m_vertices[i];
			const Vector3& p0 = patch[0*4+0];
			const Vector3& p1 = patch[0*4+3];
			const Vector3& p2 = patch[3*4+3];
			const Vector3& p3 = patch[3*4+0];

			edges[ Edge(p3, p0) ] = faceindex;
			edges[ Edge(p0, p1) ] = faceindex;
			edges[ Edge(p1, p2) ] = faceindex;
			edges[ Edge(p2, p3) ] = faceindex;
			
			++faceindex;
		}

		// get polygons adjacent to edges
		m_adj.setPolygons( faceindex, 4 );
		faceindex = 0;
		for ( int i = 0 ; i < m_vertices.size() ; i += 16 )
		{
			const Vector3* patch = &m_vertices[i];
			const Vector3& p0 = patch[0*4+0];
			const Vector3& p1 = patch[0*4+3];
			const Vector3& p2 = patch[3*4+3];
			const Vector3& p3 = patch[3*4+0];

			int adj[4];
			adj[0] = edges[ Edge(p0, p3) ];
			adj[1] = edges[ Edge(p1, p0) ];
			adj[2] = edges[ Edge(p2, p1) ];
			adj[3] = edges[ Edge(p3, p2) ];

			m_adj.setAdjacent( faceindex, adj, 4 );
			++faceindex;
		}

		// store new adjacency information accuracy
		m_adjZeroDistance = zeroDistance;
	}
	return m_adj;
}

VertexFormat PatchList::vertexFormat() const
{
	return VertexFormat();
}


} // sg
