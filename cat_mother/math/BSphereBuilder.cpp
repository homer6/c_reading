#include "BSphereBuilder.h"
#include <math.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


BSphereBuilder::BSphereBuilder() :
	m_pass(0), m_sphereValid(false)
{
}

bool BSphereBuilder::nextPass()
{
	switch ( m_pass )
	{
	case 0:{
		// initial pass, compute sphere rotation
		m_pass			= 1;
		m_points		= 0;
		m_center		= Vector3( 0.f, 0.f, 0.f );
		m_sphereValid	= false;
		return true;}

	case 1:{
		// nothing to do?
		if ( m_points < 2 )
		{
			m_sphere = BSphere();
			if ( m_points == 1 )
				m_sphere.setTranslation( m_center );
			m_sphereValid = true;
			return false;
		}

		// second pass, compute sphere radius
		m_pass	= 2;
		m_center *= 1.f / m_points;
		m_maxRadius = 0.f;
		return true;}

	case 2:{
		// set up the sphere
		m_sphere.setTranslation( m_center );
		m_sphere.setRadius( sqrtf(m_maxRadius) );
		m_sphereValid = true;

		// restart cycle
		m_pass	= 0;
		return false;}
	}
	return false;
}

void BSphereBuilder::addPoints( const Vector3* points, int count )
{
	switch ( m_pass )
	{
	case 1:{
		const Vector3* end = points + count;
		for ( const Vector3* point = points ; point != end ; ++point )
		{
			const Vector3& p = *point;
			assert( p.finite() );

			// update sum
			m_center += p;
		}

		// update point count
		m_points += count;
		break;}

	case 2:{
		const Vector3* end = points + count;
		for ( const Vector3* point = points ; point != end ; ++point )
		{
			const Vector3& p = *point;
			assert( p.finite() );

			// update radius
			float r = (p-m_center).lengthSquared();
			if ( r > m_maxRadius )
				m_maxRadius = r;
		}
		break;}
	}
}

const BSphere& BSphereBuilder::sphere() const
{
	assert( m_sphereValid );
	return m_sphere;
}


} // math
