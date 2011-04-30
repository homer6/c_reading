#include "BSPTreeBuilder.h"
#include "BSPPolygon.h"
#include "BSPNode.h"
#include "BSPSplitSelector.h"
#include "BSPTree.h"
#include "ProgressIndicator.h"
#include <dev/Profile.h>
#include <lang/Float.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <assert.h>
#include <stdlib.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define MAX_ZERO_VERTEX_DISTANCE 1e-5f

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


class BSPTreeBuilder::BSPTreeBuilderImpl :
	public Object, public ProgressIndicator
{
public:
	BSPTreeBuilderImpl() :
		m_progressMutex( Object::OBJECT_INITMUTEX ),
		m_tempIndexBuffer( Allocator<int>(__FILE__) )
	{
		reset();
	}

	~BSPTreeBuilderImpl()
	{
	}

	void reset()
	{
		m_polygons				= new Vector<BSPPolygon*>( Allocator<BSPPolygon*>(__FILE__,__LINE__) );
		m_tree					= new BSPTree;
		m_progress				= 0.f;
		m_lastPrintedProgress	= 0.f;
		m_workDone				= 0;
		m_workTotal				= 0;
	}

	void addPolygon( const Vector3* v, int n, int id, int collisionMask )
	{
		if ( !isPolygonValid(v,n) )
			return;

		// optimize vertices
		m_tempIndexBuffer.setSize( 0 );
		for ( int i = 0 ; i < n ; ++i )
		{
			const Vector3* vert = m_tree->vertexData.begin();
			int vertc = m_tree->vertexData.size();
			for ( int k = 0 ; k < vertc ; ++k )
			{
				float dsqr = ( vert[k] - v[i]  ).lengthSquared();
				if ( dsqr < Float::MIN_VALUE )
				{
					// found old identical vertex position
					m_tempIndexBuffer.add( k );
					vert = 0;
					break;
				}
			}

			// add new vertex?
			assert( m_tempIndexBuffer.size() >= i );
			if ( m_tempIndexBuffer.size() == i )
			{
				m_tempIndexBuffer.add( m_tree->vertexData.size() );
				m_tree->vertexData.add( v[i] );
			}
		}

		BSPPolygon* poly = m_tree->createPolygon();
		poly->create( m_tempIndexBuffer.begin(), n, id, m_tree, collisionMask );
		m_polygons->add( poly );

		// DEBUG: print polygon count
		if ( m_polygons->size() % 1000 == 0 )
			Debug::println( "bsp: Added polygon {0}", m_polygons->size() );
	}

	BSPTree* build( BSPSplitSelector* splitSel )
	{
		setProgress( 0.f );
		
		// guess amount of work to be done
		m_workDone = 0;
		m_workTotal = splitSel->guessWork( polygons() );
		m_lastPrintedProgress = 0.f;

		// build tree
		long time = System::currentTimeMillis();
		BSPNode* root = buildTree( m_polygons, splitSel );
		if ( !root )
		{
			Vector<BSPPolygon*> polys( Allocator<BSPPolygon*>(__FILE__) );
			root = m_tree->createNode( Vector4(0,0,0,0), polys, 0, 0 );
		}
		time = System::currentTimeMillis() - time;
		setProgress( 1.f );

		// print debug info
		if ( root )
			Debug::println( "bsp: BSPTreeBuilder.build done: time {0} seconds, depth {1} levels, total {2} polys, {3} verts", time/1000, root->depth(), polygons(), vertices() );
		else
			Debug::println( "bsp: BSPTreeBuilder.build done: No tree" );

		return m_tree;
	}

	int vertices() const
	{
		return m_tree->vertexData.size();
	}

	int polygons() const
	{
		return m_polygons->size();
	}

	float progress() const
	{
		synchronized( m_progressMutex );
		return m_progress;
	}

private:
	void addProgress( double work )
	{
		assert( m_workTotal > 0 );

		m_workDone += work;
		if ( m_workDone > m_workTotal )
			m_workDone = m_workTotal;
		setProgress( (float)(m_workDone/m_workTotal) );
	}

	void setProgress( float progress )
	{
		synchronized( m_progressMutex );
		m_progress = progress;
		
		// DEBUG: print progress
		if ( m_progress-m_lastPrintedProgress > 0.01f )
		{
			Debug::println( "bsp: BSP building progress {0,#}", m_progress*100.f );
			m_lastPrintedProgress = m_progress;
		}
	}

	/**
	 * Builds BSP tree recursively.
	 * @param polys Polygons in this space.
	 * @param scoreFunc Score function to rank split planes.
	 * @return Root node.
	 */
	BSPNode* buildTree( const Vector<BSPPolygon*>* polys, BSPSplitSelector* splitSel )
	{
		if ( 0 == polys->size() )
			return 0;

		Vector4 splitPlane(0,0,0,0);
		if ( polys->size() > 1 && splitSel->getSplitPlane( *polys, this, &splitPlane ) )
		{
			Vector<BSPPolygon*> neg( Allocator<BSPPolygon*>(__FILE__,__LINE__) );
			Vector<BSPPolygon*> on( Allocator<BSPPolygon*>(__FILE__,__LINE__) );
			Vector<BSPPolygon*> pos( Allocator<BSPPolygon*>(__FILE__,__LINE__) );

			if ( partitionPolygons( polys, splitPlane, &neg, &on, &pos ) )
			{
				BSPNode* posNode = buildTree( &pos, splitSel );
				pos.clear(); pos.trimToSize();

				BSPNode* negNode = buildTree( &neg, splitSel );
				neg.clear(); neg.trimToSize();

				return m_tree->createNode( splitPlane, on, posNode, negNode );
			}
			else
			{
				return m_tree->createNode( splitPlane, *polys, 0, 0 );
			}
		}
		else
		{
			return m_tree->createNode( splitPlane, *polys, 0, 0 );
		}
	}

	/** 
	 * Partitions polygons by a plane. 
	 * @param polys List of all polygons.
	 * @param plane Partition plane.
	 * @param polysNeg [out] Receives polygons on the negative side of the plane.
	 * @param polysOn [out] Receives polygons on the plane.
	 * @param polysPos [out] Receives polygons on the positive side of the plane.
	 * @return true if partition succesful.
	 */
	bool partitionPolygons( const Vector<BSPPolygon*>* polys,
		const Vector4& plane, 
		Vector<BSPPolygon*>* polysNeg,
		Vector<BSPPolygon*>* polysOn,
		Vector<BSPPolygon*>* polysPos )
	{
		int polysFront = 0;
		int polysBehind = 0;
		int polysBothSides = 0;
		
		for ( int i = 0 ; i < polys->size() ; ++i )
		{
			BSPPolygon* poly = polys->get(i);
			
			float minDist = Float::MAX_VALUE;
			float maxDist = -Float::MAX_VALUE;
			for ( int k = 0 ; k < poly->vertices() ; ++k )
			{
				const Vector3& v = poly->getVertex(k);
				float sdist = Vector4(v.x,v.y,v.z,1.f).dot( plane );
				if ( sdist < minDist )
					minDist = sdist;
				if ( sdist > maxDist )
					maxDist = sdist;
			}

			if ( minDist > -BSPNode::PLANE_THICKNESS && maxDist < BSPNode::PLANE_THICKNESS )
			{
				// polygon is on the plane
				polysOn->add( poly );
			}
			else
			{
				bool behind = (minDist <= -BSPNode::PLANE_THICKNESS);
				bool front = (maxDist >= BSPNode::PLANE_THICKNESS);

				if ( behind )
				{
					// polygon is behind the plane
					polysNeg->add( poly );
				}
				if ( front  )
				{
					// polygon is in front of the plane
					polysPos->add( poly );
				}

				// update polygon counters
				switch ( (behind?1:0) + (front?2:0) )
				{
				case 1:		++polysBehind; break;
				case 2:		++polysFront; break;
				case 3:		++polysBothSides; break;
				}
			}
		}

		return polysFront > polysBothSides && polysBehind > polysBothSides;
	}

	/** Returns true if the polygon has at least 3 edges which are all valid. */
	static bool isPolygonValid( const Vector3* v, int n )
	{
		if ( n < 3 )
			return false;

		int k = n - 1;
		for ( int i = 0 ; i < n ; k = i++ )
		{
			if ( (v[i]-v[k]).lengthSquared() <= Float::MIN_VALUE )
				return false;
		}
		return true;
	}

	P(Vector<BSPPolygon*>)	m_polygons;
	P(BSPTree)				m_tree;
	Vector<int>				m_tempIndexBuffer;	// see addPolygon

	Object					m_progressMutex;
	float					m_progress;
	float					m_lastPrintedProgress;
	double					m_workDone;
	double					m_workTotal;
};

//-----------------------------------------------------------------------------

BSPTreeBuilder::BSPTreeBuilder()
{
	m_this = new BSPTreeBuilderImpl;
}

BSPTreeBuilder::~BSPTreeBuilder()
{
}

void BSPTreeBuilder::addPolygon( const Vector3* v, int n, int id, int collisionMask )
{
	m_this->addPolygon( v, n, id, collisionMask );
}

BSPTree* BSPTreeBuilder::build( BSPSplitSelector* splitSel )
{
	return m_this->build( splitSel );
}

int BSPTreeBuilder::polygons() const
{
	return m_this->polygons();
}

float BSPTreeBuilder::progress() const
{
	return m_this->progress();
}

void BSPTreeBuilder::removePolygons()
{
	m_this->reset();
}


} // bsp
