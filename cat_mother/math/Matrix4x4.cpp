#include "Matrix4x4.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Matrix3x3.h"
#include <dev/Profile.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace math
{


Matrix4x4::Matrix4x4( float diagonal )
{
	for ( int j = 0 ; j < ROWS ; ++j )
		for ( int i = 0 ; i < COLUMNS ; ++i )
			m[j][i] = (i == j ? diagonal : 0.f);
}

Matrix4x4::Matrix4x4( const Matrix3x3& rotation, const Vector3& translation )				
{
	setRotation( rotation ); 
	setTranslation( translation ); 
	m[3][0] = m[3][1] = m[3][2] = 0.f; m[3][3] = 1.f;
}

void Matrix4x4::setPerspectiveProjection( float fovHorz, float front, float back, float aspect )
{
	assert( (back-front) > valueMin() );

	// 'w-friendly' projection matrix
    float w = 1.f / valueTan( fovHorz * .5f );
	if ( w < 0.f )
		w = -w;

	const float INVERSE_ASPECT_RATIO = 1.f / aspect;
	float vw = 2.f*front / w;
	float vh = vw * INVERSE_ASPECT_RATIO;
	float h = 2.f*front / vh;

    float q = back / (back-front);
	MatrixMxN::set( begin(), end(), float(0) );
	m[0][0] = w;
    m[1][1] = h;
    m[2][2] = q;
    m[3][2] = 1.f;
    m[2][3] = -q*front;
}

void Matrix4x4::setPointPlaneProjection( const Vector3& l,
	const Vector3& n, const Vector3& p )
{
	float d		= -p.dot(n);
	float nl	= n.dot(l);

	m[0][0]		= nl + d - l.x*n.x;
	m[0][1]		= -l.x*n.y;
	m[0][2]		= -l.x*n.z;
	m[0][3]		= -l.x*d;

	m[1][0]		= -l.y*n.x;
	m[1][1]		= nl + d - l.y*n.y;
	m[1][2]		= -l.y*n.z;
	m[1][3]		= -l.y*d;

	m[2][0]		= -l.z*n.x;
	m[2][1]		= -l.z*n.y;
	m[2][2]		= nl + d - l.z*n.z;
	m[2][3]		= -l.z*d;

	m[3][0]		= -n.x;
	m[3][1]		= -n.y;
	m[3][2]		= -n.z;
	m[3][3]		= nl;
}

void Matrix4x4::setDirectPlaneProjection( const Vector3& l,
	const Vector3& n, const Vector3& p )
{
	// v' = l dot ( (v-p) dot n / (-l dot n) ) + v

	float np = n.dot(p);
	float nl = n.dot(l);

	m[0][0] = l.x*n.x -nl;
	m[0][1] = l.x*n.y;
	m[0][2] = l.x*n.z;
	m[0][3] = -l.x*np;

	m[1][0] = l.y*n.x;
	m[1][1] = l.y*n.y -nl;
	m[1][2] = l.y*n.z;
	m[1][3] = -l.y*np;

	m[2][0] = l.z*n.x;
	m[2][1] = l.z*n.y;
	m[2][2] = l.z*n.z -nl;
	m[2][3] = -l.z*np;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = -nl;
}

Matrix4x4 Matrix4x4::operator*( const Matrix4x4& other ) const
{
	//dev::Profile pr( "Matrix4x4*" );
	Matrix4x4 r;

	for ( int j = 0 ; j < ROWS ; ++j )
		for ( int i = 0 ; i < COLUMNS ; ++i )
			r.m[j][i] = m[j][0]*other.m[0][i] +	
						m[j][1]*other.m[1][i] + 
						m[j][2]*other.m[2][i] +	
						m[j][3]*other.m[3][i];

	return r;
}

Matrix4x4 Matrix4x4::transpose() const
{
	Matrix4x4 mt;

	for ( int j = 0 ; j < ROWS ; ++j )
		for ( int i = 0 ; i < COLUMNS ; ++i )
			mt.m[i][j] = m[j][i];

	return mt;
}

void Matrix4x4::setTranslation( const Vector3& t )
{
	for ( int i = 0 ; i < 3 ; ++i )
		m[i][3] = t[i];
}

void Matrix4x4::setRotation( const Matrix3x3& rot )
{
	for ( int j = 0 ; j < 3 ; ++j )
		for ( int i = 0 ; i < 3 ; ++i )
			m[j][i] = rot.get(j,i);
}

Matrix3x3 Matrix4x4::rotation() const
{
	Matrix3x3 d;
	for ( int j = 0 ; j < 3 ; ++j )
		for ( int i = 0 ; i < 3 ; ++i )
			d.set( j, i, m[j][i] );
	return d;
}

Vector3 Matrix4x4::translation() const
{
	return Vector3( m[0][3], m[1][3], m[2][3] );
}

float Matrix4x4::determinant() const
{
	assert( finite() );
	
	return 	m[0][0] * det3(m[1],m[2],m[3],1,2,3) 
		-	m[0][1] * det3(m[1],m[2],m[3],0,2,3)
		+	m[0][2] * det3(m[1],m[2],m[3],0,1,3)
		-	m[0][3] * det3(m[1],m[2],m[3],0,1,2);
}

Matrix4x4 Matrix4x4::inverse() const
{
	float det = determinant();
	assert( det > Matrix4x4::valueMin() || det < -Matrix4x4::valueMin() ); 
	float invdet = 1.f / det;

	float invdet0 = invdet;
	Matrix4x4 invm;
	for ( int j = 0 ; j < 4 ; ++j )
	{
		int j0 = (j+3)&3;
		int j1 = (j+1)&3;
		int j2 = (j+2)&3;
		invdet = invdet0;
		invdet0 = -invdet0;
		for ( int i = 0 ; i < 4 ; ++i )
		{
			int i0 = (i+3)&3;
			int i1 = (i+1)&3;
			int i2 = (i+2)&3;
			float v = invdet * det3(m[j0],m[j1],m[j2],i0,i1,i2);
			invm.m[i][j] = v;
			invdet = -invdet;
		}
	}

	return invm;
}

Vector4 Matrix4x4::operator*( const math::Vector4& v ) const
{
	Vector4 v1;
	transform( v, &v1 );
	return v1;
}

Vector3 Matrix4x4::transform( const Vector3& v ) const
{
	Vector3 v1;
	transform( v, &v1 );
	return v1;
}

Vector3 Matrix4x4::rotate( const Vector3& v ) const
{
	Vector3 v1;
	rotate( v, &v1 );
	return v1;
}

/* DOES NOT WORK!
void Matrix4x4::setInverseOrthonormalTransform( const Matrix3x3& rot, const Vector3& t )
{
	m[0][0] = rot(0,0);	m[0][1] = rot(1,0);	m[0][2] = rot(2,0);
	m[1][0] = rot(0,1);	m[1][1] = rot(1,1);	m[1][2] = rot(2,1);
	m[2][0] = rot(0,2);	m[2][1] = rot(1,2);	m[2][2] = rot(2,2);

	m[0][3] = -( t[0]*m[0][0] + t[1]*m[0][1] + t[2]*m[0][2] );
	m[1][3] = -( t[0]*m[1][0] + t[1]*m[1][1] + t[2]*m[1][2] );
	m[2][3] = -( t[0]*m[2][0] + t[1]*m[2][1] + t[2]*m[2][2] );

	m[3][2] = m[3][1] = m[3][0] = 0.f; m[3][3] = 1.f;
}*/


} // math
