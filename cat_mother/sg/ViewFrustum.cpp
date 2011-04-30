#include "ViewFrustum.h"
#include <lang/Math.h>
#include <math/Matrix4x4.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define MIN_FOV 1.f
#define MAX_FOV 179.f
#define MIN_FRUSTUM_DISTANCE 1e-3f
#define MAX_FRUSTUM_DISTANCE 1e6f

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


/** Makes plane (a,b,c,d) from plane normal and distance to origin. */
static inline void makePlane( const Vector3& normal, float distanceToOrigin, 
	Vector4* plane )
{
	plane->x = normal.x;
	plane->y = normal.y;
	plane->z = normal.z;
	plane->w = distanceToOrigin;
}

//-----------------------------------------------------------------------------

const int ViewFrustum::PLANE_COUNT = 6;

//-----------------------------------------------------------------------------

ViewFrustum::ViewFrustum() :
	m_front( 0.1f ),
	m_back( 1000.f ),
	m_horzFov( 1.f ),
	m_planesDirty( true )
{
}

void ViewFrustum::setHorizontalFov( float horzFov )
{
	assert( horzFov >= Math::toRadians(MIN_FOV) && horzFov <= Math::toRadians(MAX_FOV) );

	m_horzFov = horzFov;
	m_planesDirty = true;
}

void ViewFrustum::setFront( float front )
{
	assert( front >= MIN_FRUSTUM_DISTANCE && front <= MAX_FRUSTUM_DISTANCE );
	
	m_front = front;
	m_planesDirty = true;
}

void ViewFrustum::setBack( float back )
{
	assert( back >= MIN_FRUSTUM_DISTANCE && back <= MAX_FRUSTUM_DISTANCE );

	m_back = back;
	m_planesDirty = true;
}

void ViewFrustum::getViewDimensions( float* x, float* y, float* z ) const
{
    float w = 1.f / Math::tan( m_horzFov * .5f );
	if ( w < 0.f )
		w = -w;
	const float INVERSE_ASPECT_RATIO = 0.75f;
	float vw = 2.f*m_front / w;
	float vh = vw * INVERSE_ASPECT_RATIO;

	*x = vw;
	*y = vh;
	*z = m_front;
}

const Vector4* ViewFrustum::planes() const
{
	if ( m_planesDirty )
		refreshPlanes();

	return m_planes;
}

float ViewFrustum::verticalFov() const
{
	float vx, vy, front;
	getViewDimensions( &vx, &vy, &front );
	float tana = (vy*.5f) / front;
	return 2.f * Math::atan( tana );
}

void ViewFrustum::refreshPlanes() const
{
	if ( m_planesDirty )
	{
		float	halfFovHorz				= m_horzFov * 0.5f;
		float	halfFovVert				= verticalFov() * 0.5f;
		Vector3 rightPlaneNormalOut		= Vector3(1,0,0).rotate( Vector3(0,1,0), halfFovHorz );
		Vector3 leftPlaneNormalOut		= Vector3(-1,0,0).rotate( Vector3(0,1,0), -halfFovHorz );
		Vector3 topPlaneNormalOut		= Vector3(0,1,0).rotate( Vector3(1,0,0), -halfFovVert );
		Vector3 bottomPlaneNormalOut	= Vector3(0,-1,0).rotate( Vector3(1,0,0), halfFovVert );

		assert( PLANE_COUNT == 6 );
		makePlane( rightPlaneNormalOut, 0.f, m_planes+0 );
		makePlane( topPlaneNormalOut, 0.f, m_planes+1 );
		makePlane( leftPlaneNormalOut, 0.f, m_planes+2 );
		makePlane( bottomPlaneNormalOut, 0.f, m_planes+3 );
		makePlane( Vector3(0,0,-1), m_front, m_planes+4 );
		makePlane( Vector3(0,0,1), -m_back, m_planes+5 );

		m_planesDirty = false;
	}
}


} // sg
