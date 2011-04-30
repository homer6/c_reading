#include "BSphere.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


BSphere::BSphere() :
	m_center(0.f,0.f,0.f), m_radius(0.f)
{
}

BSphere::BSphere( const Vector3& center, float radius ) :
	m_center(center), m_radius(radius)
{
}

void BSphere::setTranslation( const Vector3& center )
{
	m_center = center;
}

void BSphere::setRadius( float r )
{
	assert( r >= 0.f );
	m_radius = r;
}

const Vector3& BSphere::translation() const
{
	return m_center;
}

float BSphere::radius() const
{
	return m_radius;
}


} // math
