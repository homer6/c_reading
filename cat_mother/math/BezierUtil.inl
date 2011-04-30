template <class T> T BezierUtil<T>::getCubicBezierCurve( const T x[4], float u )
{
	float uinv = 1.f - u;
	float uinvpow2 = uinv * uinv;
	float u3x = u * 3.f;
	float c0 = (uinvpow2*uinv);
	float c1 = (u3x*uinvpow2);
	float c2 = (u3x*u*uinv);
	float c3 = (u*u*u);
	T xu = x[0]*c0;
	xu += x[1]*c1;
	xu += x[2]*c2;
	xu += x[3]*c3;
	return xu;
}

template <class T> T BezierUtil<T>::getCubicBezierCurveDt( const T x[4], float u )
{
	float uinv = 1.f - u;
	float uinvpow2 = uinv * uinv;
	float upow2 = u * u;
	float ua = 3.f * uinvpow2;
	float ub = 6.f * uinv * u;
	float uc = 3.f * upow2;
	float c0 = -ua;
	float c1 = ua - ub;
	float c2 = ub - uc;
	float c3 = uc;
	T xu = x[0]*c0;
	xu += x[1]*c1;
	xu += x[2]*c2;
	xu += x[3]*c3;
	return xu;
}

template <class T> void BezierUtil<T>::splitCubicBezierCurve( const T x[4], T a[4], T b[4] )
{
	a[0] = x[0];
	a[1] = x[0]*.5f + x[1]*.5f;
	a[2] = x[0]*.25f + x[1]*.5f + x[2]*.25f;
	a[3] = x[0]*.125f + x[1]*.375f + x[2]*.375f + x[3]*.125f;
	b[0] = x[3];
	b[1] = x[3]*.5f + x[2]*.5f;
	b[2] = x[3]*.25f + x[2]*.5f + x[1]*.25f;
	b[3] = x[3]*.125f + x[2]*.375f + x[1]*.375f + x[0]*.125f;
}

template <class T> void BezierUtil<T>::splitCubicBezierCurve( const T& x0, const T& x1, const T& x2, const T& x3, T* a0, T* a1, T* a2, T* a3, T* b0, T* b1, T* b2, T* b3 )
{
	*a0 = x0;
	*a1 = x0*.5f + x1*.5f;
	*a2 = x0*.25f + x1*.5f + x2*.25f;
	*a3 = x0*.125f + x1*.375f + x2*.375f + x3*.125f;
	*b0 = x3;
	*b1 = x3*.5f + x2*.5f;
	*b2 = x3*.25f + x2*.5f + x1*.25f;
	*b3 = x3*.125f + x2*.375f + x1*.375f + x0*.125f;
}

template <class T> void BezierUtil<T>::splitCubicBezierCurve( const T x[4], float a, float b, T y[4] )
{
	// (M^-1 * S * M) * Points
	const float im[4][4] = { {1.f, 0.f, 0.f, 0.f}, {1.f, 1.f/3.f, 0.f, 0.f}, {1.f, 2.f/3.f, 1.f/3.f, 0.f}, {1.f, 1.f, 1.f, 1.f} };
	const float m[4][4] = { {1.f, 0.f, 0.f, 0.f}, {-3.f, 3.f, 0.f, 0.f}, {3.f, -6.f, 3.f, 0.f}, {-1.f, 3.f, -3.f, 1} };
	int i, j;
	float s = b - a;
	float s2 = s * s;
	float s3 = s2 * s;
	float a2 = a * a;
	float a3 = a2 * a;
	float sm[4][4] = { {1.f, a, a, a3}, {0.f, s, 2.f*a*s, 3.f*a2*s}, {0.f, 0.f, s2, 3.f*a*s2}, {0.f, 0.f, 0.f, s3}};
	float tmp[4][4];
	for ( j = 0 ; j < 4 ; ++j )
		for ( i = 0 ; i < 4 ; ++i )
			tmp[j][i] = im[j][0]*sm[0][i] +	im[j][1]*sm[1][i] + 
				im[j][2]*sm[2][i] +	im[j][3]*sm[3][i];
	float tm[4][4];
	for ( j = 0 ; j < 4 ; ++j )
		for ( i = 0 ; i < 4 ; ++i )
			tm[j][i] = tmp[j][0]*m[0][i] +	tmp[j][1]*m[1][i] + 
				tmp[j][2]*m[2][i] + tmp[j][3]*m[3][i];
	for ( i = 0 ; i < 4 ; ++i )
		y[i] = x[0]*tm[i][0] + x[1]*tm[i][1] + x[2]*tm[i][2] + x[3]*tm[i][3];
}

template <class T> T BezierUtil<T>::getCubicBezierPatch( const T x[4][4], float u, float v )
{
	float uinv = 1.f - u;
	float uinvpow2 = uinv * uinv;
	float u3x = u * 3.f;
	float uc[4];
	uc[0] = (uinvpow2*uinv);
	uc[1] = (u3x*uinvpow2);
	uc[2] = (u3x*u*uinv);
	uc[3] = (u*u*u);
	float vinv = 1.f - v;
	float vinvpow2 = vinv * vinv;
	float v3x = v * 3.f;
	float vc[4];
	vc[0] = (vinvpow2*vinv);
	vc[1] = (v3x*vinvpow2);
	vc[2] = (v3x*v*vinv);
	vc[3] = (v*v*v);
	T xuv = (x[0][0]*uc[0] + x[0][1]*uc[1] + x[0][2]*uc[2] + x[0][3]*uc[3] ) * vc[0];
	for ( int i = 1 ; i < 4 ; ++i )
		xuv += (x[i][0]*uc[0] + x[i][1]*uc[1] + x[i][2]*uc[2] + x[i][3]*uc[3]) * vc[i];
	return xuv;
}
