#include "Matrix3x3.h"
#include "Vector3.h"
#include "Quaternion.h"
#include <math.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//----------------------------------------------------------------------------

namespace math
{


Matrix3x3::Matrix3x3( const Vector3& axis, float angle )
{
	// ensure that Axis isn't null vector
	assert( axis.length() > valueMin() );

	Vector3						unitAxis		= axis.normalize();
	const float					halfAngle		= angle/float(2);
	const float					s				= valueSin(halfAngle);
	const float					c				= valueCos(halfAngle);
	const float					x0				= unitAxis.x * s;
	const float					y0				= unitAxis.y * s;
	const float					z0				= unitAxis.z * s;
	const float					w				= c;
	const float					xx				= x0*x0;
	const float					xy 				= y0*x0;
	const float					xz				= z0*x0;
	const float					yy				= y0*y0;
	const float					yz				= z0*y0;
	const float					zz				= z0*z0;
	const float					wx				= w*x0;
	const float					wy				= w*y0;
	const float					wz				= w*z0;

	this->m[0][0]=float(1)-float(2)*(yy+zz);	this->m[0][1]=float(2)*(xy-wz);				this->m[0][2]=float(2)*(xz+wy);
	this->m[1][0]=float(2)*(xy+wz);				this->m[1][1]=float(1)-float(2)*(xx+zz);	this->m[1][2]=float(2)*(yz-wx);
	this->m[2][0]=float(2)*(xz-wy);				this->m[2][1]=float(2)*(yz+wx);				this->m[2][2]=float(1)-float(2)*(xx+yy);

	assert( finite() );
}

Matrix3x3::Matrix3x3( float diagonal )
{
	for ( int j = 0 ; j < ROWS ; ++j )
		for ( int i = 0 ; i < COLUMNS ; ++i )
			m[j][i] = (i == j ? diagonal : 0.f);

	assert( finite() );
}

Matrix3x3::Matrix3x3( const Quaternion& q )
{
	float len = q.norm();
	assert( len > valueMin() );
	float d = float(2) / len;

    float tx  = d*q.x;
    float ty  = d*q.y;
    float tz  = d*q.z;
    float twx = tx*q.w;
    float twy = ty*q.w;
    float twz = tz*q.w;
    float txx = tx*q.x;
    float txy = ty*q.x;
    float txz = tz*q.x;
    float tyy = ty*q.y;
    float tyz = tz*q.y;
    float tzz = tz*q.z;

    m[0][0] = float(1)-(tyy+tzz);
    m[0][1] = txy-twz;
    m[0][2] = txz+twy;
    m[1][0] = txy+twz;
    m[1][1] = float(1)-(txx+tzz);
    m[1][2] = tyz-twx;
    m[2][0] = txz-twy;
    m[2][1] = tyz+twx;
    m[2][2] = float(1)-(txx+tyy);

	assert( finite() );
}

Matrix3x3 Matrix3x3::operator*( const Matrix3x3& other ) const
{
	Matrix3x3 r;

	for ( int j = 0 ; j < ROWS ; ++j )
		for ( int i = 0 ; i < COLUMNS ; ++i )
			r.m[j][i] = m[j][0]*other.m[0][i] +	
						m[j][1]*other.m[1][i] + 
						m[j][2]*other.m[2][i];

	return r;
}

float Matrix3x3::determinant() const
{
	assert( finite() );

	return det3( m[0], m[1], m[2], 0, 1, 2 );
}

Matrix3x3 Matrix3x3::transpose() const
{
	Matrix3x3 mt;

	for ( int j = 0 ; j < ROWS ; ++j )
		for ( int i = 0 ; i < COLUMNS ; ++i )
			mt.m[i][j] = m[j][i];

	return mt;
}

Matrix3x3 Matrix3x3::inverse() const
{
	float det = determinant();
	assert( det > Matrix3x3::valueMin() || det < -Matrix3x3::valueMin() ); 
	float invdet = float(1) / det;

	Matrix3x3 inv;
	inv.m[0][0] = invdet * (m[1][1]*m[2][2] - m[1][2]*m[2][1]);
	inv.m[0][1] = invdet * (m[2][1]*m[0][2] - m[2][2]*m[0][1]);
	inv.m[0][2] = invdet * (m[0][1]*m[1][2] - m[0][2]*m[1][1]);
	inv.m[1][0] = invdet * (m[1][2]*m[2][0] - m[1][0]*m[2][2]);
	inv.m[1][1] = invdet * (m[2][2]*m[0][0] - m[2][0]*m[0][2]);
	inv.m[1][2] = invdet * (m[0][2]*m[1][0] - m[0][0]*m[1][2]);
	inv.m[2][0] = invdet * (m[1][0]*m[2][1] - m[1][1]*m[2][0]);
	inv.m[2][1] = invdet * (m[2][0]*m[0][1] - m[2][1]*m[0][0]);
	inv.m[2][2] = invdet * (m[0][0]*m[1][1] - m[0][1]*m[1][0]);
	return inv;
}

Matrix3x3 Matrix3x3::orthonormalize() const
{
	assert( finite() );
	assert( getColumn(0).cross(getColumn(1)).length() > valueMin() );
	assert( getColumn(1).cross(getColumn(2)).length() > valueMin() );
	assert( getColumn(0).cross(getColumn(2)).length() > valueMin() );

	// Gram-Schmidt orthogonalization
	Vector3 x = getColumn(0).normalize();
	Vector3 y = getColumn(1);
	y = ( y - x*y.dot(x) ).normalize();
	Vector3 z = getColumn(2);
	z = ( z - x*z.dot(x) - y*z.dot(y) ).normalize();

	Matrix3x3 n;
	n.setColumn( 0, x );
	n.setColumn( 1, y );
	n.setColumn( 2, z );
	assert( n.finite() );
	return n;
}

bool Matrix3x3::lefthand() const
{
	Vector3 x = getColumn( 0 );
	Vector3 y = getColumn( 1 );
	Vector3 z = getColumn( 2 );
	return x.cross(y).dot(z) > float(0);
}

Vector3 Matrix3x3::operator*( const Vector3& v ) const
{
	Vector3 v1;
	rotate( v, &v1 );
	return v1;
}

void Matrix3x3::generateOrthonormalBasis( const Vector3& axis )
{
	assert( fabsf(axis.length()-1.f) < 1e-3f ); // axis must be unit vector

	Vector3 u;
    if ( fabsf(axis.x) < fabsf(axis.y) )
    {
        float invlen = 1.f / sqrtf( axis.y*axis.y + axis.z*axis.z );
        u.x = 0.f;
        u.y = axis.z * invlen;
        u.z = -axis.y * invlen;
    }
	else
	{
        float invlen = 1.f / sqrtf( axis.x*axis.x + axis.z*axis.z );
        u.x = -axis.z * invlen;
        u.y = 0.f;
        u.z = axis.x * invlen;
	}

	setColumn( 0, u );
	setColumn( 1, axis.cross(u) );
	setColumn( 2, axis );
}


} // math
