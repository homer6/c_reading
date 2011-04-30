#include "GamePortal.h"
#include <math/Intersection.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

GamePortal::GamePortal( const math::Vector3* corners, GameCell* target ) :
	m_plane( 0, 0, 0, 0 ),
	m_normal( 0, 0, 0 ),
	m_target( target )
{
	for ( int i = 0 ; i < NUM_CORNERS ; ++i )
		m_corners[i] = corners[i];
	
	const Vector3& v0 = corners[0];
	const Vector3& v1 = corners[1];
	const Vector3& v2 = corners[2];
	Vector3 normal = ( v1-v0 ).cross( v2-v0 ).normalize();
	m_plane = Vector4( normal.x, normal.y, normal.z, -v0.dot(normal) );
	m_normal = normal;
}

bool GamePortal::isOnPositiveSide( const math::Vector3& position ) const
{
	float startDist = m_plane.x*position.x + m_plane.y*position.y + m_plane.z*position.z + m_plane.w;
	return startDist >= 0.f;
}

bool GamePortal::isCrossing( const math::Vector3& position, const math::Vector3& delta ) const 
{
	if ( delta.dot(m_normal) < 0.f )
	{
		const float MARGIN = 0.01f;
		Vector3 pos = position + m_normal*MARGIN;
		Vector3 d = delta - m_normal*(MARGIN*2.f);

		if ( isOnPositiveSide(pos) )
		{
			bool iscrossing1 = Intersection::findLineTriangleIntersection( pos, d, m_corners[0], m_corners[1], m_corners[3], 0 );
			bool iscrossing2 = Intersection::findLineTriangleIntersection( pos, d, m_corners[1], m_corners[2], m_corners[3], 0 );

			return iscrossing1 || iscrossing2;
		}
	}
	return false;
}

GameCell* GamePortal::target() const 
{
	return m_target;
}

void GamePortal::getCorners( math::Vector3* corners ) const 
{
	assert( corners );

	for ( int i = 0 ; i < NUM_CORNERS ; ++i )
		corners[i] = m_corners[i];
}
