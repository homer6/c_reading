#include "Noise.h"
#include "FloatUtil.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define B 0x100
#define BM 0xff
#define N 0x1000

#define SETUP(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.f;

#define S_CURVE(t)		( t * t * (3.f - 2.f * t) )
#define LERP(t, a, b)	( a + t * (b - a) )
#define AT2(rx,ry)		( rx * q[0] + ry * q[1] )
#define AT3(rx,ry,rz)	( rx * q[0] + ry * q[1] + rz * q[2] )

//-----------------------------------------------------------------------------

namespace math
{


static int		s_p[B + B + 2];
static float	s_g3[B + B + 2][3];
static float	s_g2[B + B + 2][2];
static float	s_g1[B + B + 2];
static int		s_start = 1;

//-----------------------------------------------------------------------------

static void normalize2( float v[2] )
{
	float s;

	s = sqrtf(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

static void normalize3( float v[3] )
{
	float s;

	s = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

static void init()
{
	int i, j, k;

	for (i = 0 ; i < B ; i++) {
		s_p[i] = i;

		s_g1[i] = (float)((rand() % (B + B)) - B) / B;

		for (j = 0 ; j < 2 ; j++)
			s_g2[i][j] = (float)((rand() % (B + B)) - B) / B;
		normalize2(s_g2[i]);

		for (j = 0 ; j < 3 ; j++)
			s_g3[i][j] = (float)((rand() % (B + B)) - B) / B;
		normalize3(s_g3[i]);
	}

	while (--i) {
		k = s_p[i];
		s_p[i] = s_p[j = rand() % B];
		s_p[j] = k;
	}

	for (i = 0 ; i < B + 2 ; i++) {
		s_p[B + i] = s_p[i];
		s_g1[B + i] = s_g1[i];
		for (j = 0 ; j < 2 ; j++)
			s_g2[B + i][j] = s_g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			s_g3[B + i][j] = s_g3[i][j];
	}
}

//-----------------------------------------------------------------------------

float Noise::noise1( float x )
{
	float vec[1] = {x};

	int bx0, bx1;
	float rx0, rx1, sx, t, u, v;

	if (s_start) {
		s_start = 0;
		init();
	}

	SETUP( 0, bx0,bx1, rx0,rx1 );

	sx = S_CURVE(rx0);

	u = rx0 * s_g1[ s_p[ bx0 ] ];
	v = rx1 * s_g1[ s_p[ bx1 ] ];

	return LERP(sx, u, v);
}

float Noise::noise2( float x, float y )
{
	float vec[2] = {x,y};

	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register i, j;

	if (s_start) {
		s_start = 0;
		init();
	}

	SETUP(0, bx0,bx1, rx0,rx1);
	SETUP(1, by0,by1, ry0,ry1);

	i = s_p[ bx0 ];
	j = s_p[ bx1 ];

	b00 = s_p[ i + by0 ];
	b10 = s_p[ j + by0 ];
	b01 = s_p[ i + by1 ];
	b11 = s_p[ j + by1 ];

	sx = S_CURVE(rx0);
	sy = S_CURVE(ry0);

	q = s_g2[ b00 ] ; u = AT2(rx0,ry0);
	q = s_g2[ b10 ] ; v = AT2(rx1,ry0);
	a = LERP(sx, u, v);

	q = s_g2[ b01 ] ; u = AT2(rx0,ry1);
	q = s_g2[ b11 ] ; v = AT2(rx1,ry1);
	b = LERP(sx, u, v);

	return LERP(sy, a, b);
}

float Noise::noise3( float x, float y, float z )
{
	float vec[3] = { x, y, z };

	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	register i, j;

	if (s_start) {
		s_start = 0;
		init();
	}

	SETUP(0, bx0,bx1, rx0,rx1);
	SETUP(1, by0,by1, ry0,ry1);
	SETUP(2, bz0,bz1, rz0,rz1);

	i = s_p[ bx0 ];
	j = s_p[ bx1 ];

	b00 = s_p[ i + by0 ];
	b10 = s_p[ j + by0 ];
	b01 = s_p[ i + by1 ];
	b11 = s_p[ j + by1 ];

	t  = S_CURVE(rx0);
	sy = S_CURVE(ry0);
	sz = S_CURVE(rz0);

	q = s_g3[ b00 + bz0 ] ; u = AT3(rx0,ry0,rz0);
	q = s_g3[ b10 + bz0 ] ; v = AT3(rx1,ry0,rz0);
	a = LERP(t, u, v);

	q = s_g3[ b01 + bz0 ] ; u = AT3(rx0,ry1,rz0);
	q = s_g3[ b11 + bz0 ] ; v = AT3(rx1,ry1,rz0);
	b = LERP(t, u, v);

	c = LERP(sy, a, b);

	q = s_g3[ b00 + bz1 ] ; u = AT3(rx0,ry0,rz1);
	q = s_g3[ b10 + bz1 ] ; v = AT3(rx1,ry0,rz1);
	a = LERP(t, u, v);

	q = s_g3[ b01 + bz1 ] ; u = AT3(rx0,ry1,rz1);
	q = s_g3[ b11 + bz1 ] ; v = AT3(rx1,ry1,rz1);
	b = LERP(t, u, v);

	d = LERP(sy, a, b);

	return LERP(sz, c, d);
}


} // math
