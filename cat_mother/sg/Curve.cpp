#include "Curve.h"
#include <lang/Math.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


Curve::Curve() :
	m_points( Allocator<Vector3>(__FILE__,__LINE__) )
{
}

Curve::Curve( const Curve& other ) :
	m_points( other.m_points )
{
}

void Curve::destroy()
{
	m_points.clear();
	m_points.trimToSize();
	Primitive::destroy();
}

void Curve::load()
{
}

void Curve::unload()
{
}

void Curve::draw()
{
}

void Curve::addPoint( const math::Vector3& point )
{
	m_points.add( point );
}

const math::Vector3& Curve::getPoint( int index ) const
{
	assert( index >= 0 && index < points() );
	return m_points[index];
}

int	Curve::points() const
{
	return m_points.size();
}

float Curve::boundSphere() const
{
	float maxd2 = 0.f;
	for ( int i = 0 ; i < (int)m_points.size() ; ++i )
	{
		float d2 = m_points[i].lengthSquared();
		if ( d2 > maxd2 )
			maxd2 = d2;
	}
	return Math::sqrt( maxd2 );
}


} // sg
