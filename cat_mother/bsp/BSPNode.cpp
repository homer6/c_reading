#include "BSPNode.h"
#include <lang/Math.h>
#include <lang/Debug.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


const float BSPNode::PLANE_THICKNESS = 0.01f;

//-----------------------------------------------------------------------------

BSPNode::BSPNode() :
	m_plane(0,0,0,0),
	m_firstPoly(0),
	m_polyCount(0),
	m_pos(0),
	m_neg(0),
	m_storage(0)
{
}

void BSPNode::create( const Vector4& plane,
	const Vector<BSPPolygon*>& polys,
	BSPNode* pos, BSPNode* neg, BSPStorage* storage )
{
	m_plane = plane;
	m_firstPoly = storage->nodePolygonData.size();
	m_polyCount = polys.size();
	m_pos = pos;
	m_neg = neg;
	m_storage = storage;

	// add polygons
	for ( int i = 0 ; i < polys.size() ; ++i )
		m_storage->nodePolygonData.add( polys[i] );

	// compute maximum polygon distance to the plane
	/*BSPPolygon** nodepolys = m_storage->nodePolygonData.begin() + m_firstPoly;
	for ( int i = 0 ; i < m_polyCount ; ++i )
	{
		const BSPPolygon& poly = *nodepolys[i];
		for ( int k = 0 ; k < poly.vertices() ; ++k )
		{
			const Vector3& p = poly.getVertex(k);
			float polyDist = Math::abs( plane.x*p.x + plane.y*p.y + plane.z*p.z + plane.w );
			if ( polyDist > m_maxPolyDist )
				m_maxPolyDist = polyDist;
		}
	}*/
}

BSPNode::~BSPNode()
{
}

int BSPNode::depth() const
{
	int posDepth = 0;
	int negDepth = 0;

	if ( m_pos )
		posDepth = m_pos->depth();

	if ( m_neg )
		negDepth = m_neg->depth();

	int subDepth = posDepth;
	if ( subDepth < negDepth )
		subDepth = negDepth;
	
	return subDepth + 1;
}

int BSPNode::nodes() const
{
	int count = 1;

	if ( m_pos )
		count += m_pos->nodes();

	if ( m_neg )
		count += m_neg->nodes();

	return count;
}


} // bsp
