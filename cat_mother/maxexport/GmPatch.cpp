#include "StdAfx.h"
#include "GmPatch.h"

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

GmPatch::GmPatch()
{
	for ( int i = 0 ; i < 4 ; ++i )
		for ( int j = 0 ; j < 4 ; ++j )
			m_points[i][j] = Vector3(0,0,0);
}

void GmPatch::setControlPoint( int i, int j, const Vector3& point )
{
	require( i >= 0 && i < 4 );
	require( j >= 0 && j < 4 );
	
	m_points[i][j] = point;
}

const Vector3& GmPatch::getControlPoint( int i, int j ) const
{
	require( i >= 0 && i < 4 );
	require( j >= 0 && j < 4 );
	
	return m_points[i][j];
}
