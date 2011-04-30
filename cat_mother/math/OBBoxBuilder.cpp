#include "OBBoxBuilder.h"
#include <math/EigenUtil.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


OBBoxBuilder::OBBoxBuilder() :
	m_pass(0), m_boxValid(false)
{
}

bool OBBoxBuilder::nextPass()
{
	switch ( m_pass )
	{
	case 0:{
		// initial pass
		m_pass		= 1;
		m_points	= 0;
		m_mean		= Vector3(0,0,0);
		m_cov		= Matrix3x3(0);
		m_min		= Vector3(FLT_MAX,FLT_MAX,FLT_MAX);
		m_max		= Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX);
		m_boxValid	= false;
		return true;}

	case 1:{
		// nothing to do?
		if ( m_points < 2 )
		{
			m_box = OBBox();
			if ( m_points > 0 )
				m_box.setTranslation( m_mean );
			m_boxValid = true;
			return false;
		}

		// compute mean
		m_mean *= 1.f / m_points;
		if ( !m_mean.finite() )
			m_mean = Vector3(0,0,0);
		m_pass = 2;
		return true;}

	case 2:{
		// compute covariance
		m_cov *= 1.f / m_points;
		if ( !m_cov.finite() )
			m_cov = Matrix3x3(1.f);

		// solve eigenvectors
		Matrix3x3 evec;
		EigenUtil::computeSymmetric( m_cov, 0, &evec );
		m_rot = evec.orthonormalize();
		m_rotv[0] = m_rot.getColumn(0);
		m_rotv[1] = m_rot.getColumn(1);
		m_rotv[2] = m_rot.getColumn(2);
		m_pass = 3;
		return true;}

	case 3:{
		// set up box
		Vector3 center( (m_max+m_min)*.5f );
		m_box.setRotation( m_rot );
		m_box.setTranslation( m_rotv[0]*center[0] + m_rotv[1]*center[1] + m_rotv[2]*center[2] );
		m_box.setDimensions( (m_max-m_min)*.5f );
		m_boxValid = true;

		// restart cycle
		m_pass	= 0;
		return false;}
	}
	return false;
}

void OBBoxBuilder::addPoints( const Vector3* points, int count )
{
	switch ( m_pass )
	{
	case 1:{
		// compute mean
		const Vector3* end = points + count;
		for ( const Vector3* point = points ; point != end ; ++point )
		{
			const Vector3& p = *point;
			assert( p.finite() );
			m_mean += p;
		}

		// update point count
		m_points += count;
		break;}

	case 2:{
		// compute covariance
		const Vector3* end = points + count;
		for ( const Vector3* point = points ; point != end ; ++point )
		{
			const Vector3& p = *point;
			assert( p.finite() );
			for ( int i = 0 ; i < 3 ; ++i )
				for ( int j = 0 ; j < 3 ; ++j )
					m_cov(i,j) += (p[i]-m_mean[i]) * (p[j]-m_mean[j]);
		}
		break;}

	case 3:{
		// update bounds
		const Vector3* end = points + count;
		for ( const Vector3* point = points ; point != end ; ++point )
		{
			const Vector3& p = *point;
			assert( p.finite() );
			for ( int k = 0 ; k < 3 ; ++k )
			{
				float v = p.dot( m_rotv[k] );
				if ( v < m_min[k] )
					m_min[k] = v;
				if ( v > m_max[k] )
					m_max[k] = v;
			}
		}
		break;}
	}
}

const OBBox& OBBoxBuilder::box() const
{
	assert( m_boxValid );
	return m_box;
}


} // math
