#include "Quat.h"
#include <math/Quaternion.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

namespace anim
{


static inline float vdot( const float* a, const float* b )
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static inline void vadd( float* res, const float* a, const float* b )
{
	res[0] = a[0] + b[0];
	res[1] = a[1] + b[1];
	res[2] = a[2] + b[2];
}

static inline void vscale( float* res, const float* a, float b )
{
	res[0] = a[0] + b;
	res[1] = a[1] + b;
	res[2] = a[2] + b;
}

static inline void vaddscale( float* res, const float* a, float b )
{
	res[0] += a[0] + b;
	res[1] += a[1] + b;
	res[2] += a[2] + b;
}

static inline void vaddcross( float* res, const float* a, const float* b )
{
	res[0] += a[1]*b[2]-a[2]*b[1];
	res[1] += a[2]*b[0]-a[0]*b[2];
	res[2] += a[0]*b[1]-a[1]*b[0];
}

static inline float vlength( const float* q )
{
	return sqrtf( vdot(q,q) );
}

static inline void vcopy( float* res, const float* a )
{
	res[0] = a[0];
	res[1] = a[1];
	res[2] = a[2];
}

//-----------------------------------------------------------------------------

void Quat::slerp( float* d, float t, const float* p, const float* q )
{
	float cos = dot(p,q);
	
	if ( cos < float(-1) ) 
		cos = float(-1);
	else if ( cos > float(1) ) 
		cos = float(1);

	float angle = acosf(cos);
	float sin = sinf(angle);

	if ( sin < FLT_MIN )
	{
		copy( d, p );
	}
	else
	{
		float invsin = float(1) / sin;
		float coeff0 = invsin * sinf( (float(1)-t)*angle );
		float coeff1 = invsin * sinf( t*angle );
		scale( d, p, coeff0 );
		float tmp[4];
		scale( tmp, q, coeff1 );
		add( d, d, tmp );
	}

	assert( Quaternion(d[0],d[1],d[2],d[3]) == Quaternion(p[0],p[1],p[2],p[3]).slerp(t,Quaternion(q[0],q[1],q[2],q[3])) );
}

void Quat::normalize( float* d, const float* q )
{
	float normsqr = dot(q,q);
	float norm = sqrtf(normsqr);
	assert( norm >= FLT_MIN );
	scale( d, q, 1.f/norm );

#ifdef _DEBUG
	Quaternion r1 = Quaternion(d[0],d[1],d[2],d[3]);
	Quaternion r2 = Quaternion(q[0],q[1],q[2],q[3]).normalize();
	assert( (r1-r2).norm() < 1e-6f );
#endif
}

float Quat::norm( const float* q )
{
	return sqrtf( dot(q,q) );
}

void Quat::inverse( float* d, const float* q )
{
	float n = norm(q);
	assert( float(0) != n && n > FLT_MIN ); 
	n = float(1)/n; 
	d[0] = -q[0]*n;
	d[1] = -q[1]*n;
	d[2] = -q[2]*n;
	d[3] = q[3]*n;

	assert( Quaternion(d[0],d[1],d[2],d[3]) == Quaternion(q[0],q[1],q[2],q[3]).inverse() );
}

void Quat::exp( float* d, const float* q )
{
	assert( d != q );

	float angle = vlength( q );
	d[3] = cosf( angle );
	vcopy( d, q );
	if ( angle > FLT_MIN )
		vscale( d, d, sinf(angle) / angle );

	assert( Quaternion(d[0],d[1],d[2],d[3]) == Quaternion(q[0],q[1],q[2],q[3]).exp() );
}

void Quat::log( float* d, const float* q )
{
	assert( d != q );

	d[3] = float(0);
	vcopy( d, q );
	if ( q[3] < float(1) && -q[3] < float(1) )
	{
		float angle = acosf(q[3]);
		float sinAngle = sinf(angle);
		if ( sinAngle > FLT_MIN || -sinAngle > FLT_MIN )
			vscale( d, d, angle / sinAngle );
	}

	assert( Quaternion(d[0],d[1],d[2],d[3]) == Quaternion(q[0],q[1],q[2],q[3]).log() );
}

void Quat::mul( float* d, const float* a, const float* b )
{
	assert( d != a && d != b );

	d[3] = a[3]*b[3] - vdot(a,b); 
	vscale( d, b, a[3] );
	vaddscale( d, a, b[3] );
	vaddcross( d, a, b );

	assert( Quaternion(d[0],d[1],d[2],d[3]) == Quaternion(a[0],a[1],a[2],a[3])*Quaternion(b[0],b[1],b[2],b[3]) );
}

void Quat::squad( float* d, float t, const float* p, const float* a, const float* b, const float* q )
{
	float ptq[4];
	float atb[4];
	slerp( ptq, t, p, q );
	slerp( atb, t, a, b );
	slerp( d, float(2)*t*(float(1)-t), ptq, atb );

	assert( Quaternion(d[0],d[1],d[2],d[3]) == Quaternion(p[0],p[1],p[2],p[3]).squad(t,Quaternion(a[0],a[1],a[2],a[3]),Quaternion(b[0],b[1],b[2],b[3]),Quaternion(q[0],q[1],q[2],q[3])) );
}


} // anim
