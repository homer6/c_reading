#include "StdAfx.h"
#include "toDx8.h"
#include <math/Vector4.h>
#include "config.h"

//-----------------------------------------------------------------------------

void toDx8( const math::Vector3& s, D3DVECTOR& d )
{
	d.x = s.x;
	d.y = s.y;
	d.z = s.z;
}

void toDx8( const math::Matrix4x4& s, D3DMATRIX& d )
{
	for ( int j = 0 ; j < 4 ; ++j )
		for ( int i = 0 ; i < 4 ; ++i )
			d.m[i][j] = s(j,i);
}
