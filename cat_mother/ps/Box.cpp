#include "Box.h"
#include <lang/Math.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace ps
{


Box::Box( const math::Vector3& minCorner, const math::Vector3& maxCorner )
{
	m_min = minCorner;
	m_max = maxCorner;
}

void Box::getRandomPoint( math::Vector3* point )
{
	Vector3 d = m_max - m_min;
	point->x = Math::random() * d.x + m_min.x;
	point->y = Math::random() * d.y + m_min.y;
	point->z = Math::random() * d.z + m_min.z;
}


} // ps
