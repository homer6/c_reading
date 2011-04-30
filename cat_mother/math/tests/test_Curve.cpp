#include <tester/Test.h>
#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <dev/TimeStamp.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <util/Vector.h>
#include <math/EigenUtil.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Matrix3x3.h>
#include <math/Matrix4x4.h>
#include <math/BezierUtilVector3.h>
#include <math/Intersection.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <assert.h>
#include <math/internal/config.h>

//-----------------------------------------------------------------------------

using namespace dev;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

static void setPixel( Image* img, float x, float y, long color )
{
	int x1 = (int)x;
	int y1 = (int)y;

	if ( x1 >= 0 && x1 < img->width() &&
		y1 >= 0 && y1 < img->height() )
	{
		img->surface().setPixel( x1, y1, color );
	}
}

static void setPixel( Image* img, float x, float y, float z, long color )
{
	int x1 = (int)x;
	int y1 = (int)y;

	if ( x1 >= 0 && x1 < img->width() &&
		y1 >= 0 && y1 < img->height() )
	{
		int z1 = (int)(z*3.f);
		int shift = 0;
		if ( z1 < 0 )
			z1 = 0;
		else if ( z1 >= 256 && z1 < 512 )
			{shift = 8; z1 -= 256; color |= 0xFF;}
		else if ( z1 >= 512 && z1 < 768 )
			{shift = 16; z1 -= 512; color |= 0xFFFF;}
		else if ( z1 >= 768 )
			{shift = 16; z1 = 255; color |= 0xFFFF;}
		color &= ~(255 << shift);
		color |= z1 << shift;
		img->surface().setPixel( x1, y1, color );
	}
}

static void drawLine( Image* img, float x0, float y0, float x1, float y1, long color )
{
	int n = 2;
	if ( Math::abs(x1-x0) > n )
		n = (int)Math::round( Math::abs(x1-x0) + .5f );
	if ( Math::abs(y1-y0) > n )
		n = (int)Math::round( Math::abs(y1-y0) + .5f );

	float dx = (x1 - x0)/(n-1);
	float dy = (y1 - y0)/(n-1);
	float x = x0;
	float y = y0;
	for ( int i = 0 ; i < n ; ++i )
	{
		setPixel( img, x, y, color );
		x += dx;
		y += dy;
	}
}

static float drawBezier( const float* x, const float* y, int n,
	Image* img, long color )
{
	assert( 4 == n );

	float xprev = x[0];
	float yprev = y[0];
	float len = 0.f;
	float nextTan = 0;
	for ( int i = 0 ; i < 1000 ; ++i )
	{
		float u = (float)i / 1000.f;
		float x1=0, y1=0;

		if ( n == 4 )
		{
			// get point on curve
			x1 = BezierUtil<float>::getCubicBezierCurve( x, u );
			y1 = BezierUtil<float>::getCubicBezierCurve( y, u );

			// draw tangent
			if ( u >= nextTan )
			{
				float dx1 = BezierUtil<float>::getCubicBezierCurveDt( x, u );
				float dy1 = BezierUtil<float>::getCubicBezierCurveDt( y, u );
				float len = Math::sqrt( dx1*dx1 + dy1*dy1 );
				dx1 /= len;
				dy1 /= len;
				float lineLen = 100.f;
				drawLine( img, x1, y1, x1+dx1*lineLen, y1+dy1*lineLen, ~color );
				nextTan += .1f;
			}

			// update length
			len += Math::sqrt( (x1-xprev)*(x1-xprev) + (y1-yprev)*(y1-yprev) );
			xprev = x1;
			yprev = y1;
		}

		setPixel( img, x1, y1, color );
	}

	return len;
}

static float bsplineGeometric( int i, int j, const float* x, int n, int k, float u )
{
	if ( j > 0 )
	{
		assert( k-j > 0 );
		float r = (u - i) / (float)(k-j);
		float c1 = bsplineGeometric(i-1,j-1,x,n,k,u);
		float c2 = bsplineGeometric(i,j-1,x,n,k,u);
		return (1.f-r) * c1 + r * c2;
	}
	else
	{
		assert( i < n );
		if ( i >= 0 )
			return x[i];
		else
			return x[0];
	}
}

static float bsplineMatrix( int i, const float* x, int n, int order, float u )
{
	if ( 4 == order )
	{
		Matrix4x4 m;
		m.setRow( 0, Vector4( 1, 4, 1, 0) );
		m.setRow( 1, Vector4(-3, 0, 3, 0) );
		m.setRow( 2, Vector4( 3,-6, 3, 0) );
		m.setRow( 3, Vector4(-1, 3,-3, 1) );
		m *= 1.f/6.f;
		
		Vector4 uv(1,u,u*u,u*u*u);
		Vector4 t;
		for ( int k = 0 ; k < 4 ; ++k )
			t[k] = uv[0]*m(0,k) + uv[1]*m(1,k) + uv[2]*m(2,k) + uv[3]*m(3,k);

		float v = 0.f;
		for ( int k = 0 ; k < 4 ; ++k )
		{
			int j = i+k;
			assert( j < n );
			v += t[k] * x[j];
		}
		return v;
	}
	else if ( 3 == order )
	{
		Matrix3x3 m;
		m.setRow( 0, Vector3(1,1,0) );
		m.setRow( 1, Vector3(-2,2,0) );
		m.setRow( 2, Vector3(1,-2,1) );
		m *= 1.f/2.f;

		Vector3 uv(1,u,u*u);
		Vector3 t;
		for ( int k = 0 ; k < 3 ; ++k )
			t[k] = uv[0]*m(0,k) + uv[1]*m(1,k) + uv[2]*m(2,k);

		float v = 0.f;
		for ( int k = 0 ; k < 3 ; ++k )
		{
			int j = i+k;
			assert( j < n );
			v += t[k] * x[j];
		}
		return v;
	}
	return 0.f;
}

static float bspline( const float* x, int n, int k, float u )
{
	int i = (int)u;
	if ( i < 0 )
		return x[0];
	else if ( i >= n )
		return x[n-1];
	else if ( i >= 0 && i+k <= n )	return bsplineMatrix( i, x, n, k, u-i );
	//else if ( i >= k-1 && i < n )	return bsplineGeometric( i, k-1, x, n, k, u );
	else
		return 0.f;
}

static void drawBspline( const float* x, const float* y, int n, int k,
	Image* img, long color, int yoffset=0 )
{
	for ( int i = 0 ; i < 1000 ; ++i )
	{
		float u = (float)i / 1000.f * n;
		int x1=0, y1=0;

		x1 = (int)bspline( x, n, k, u );
		y1 = (int)bspline( y, n, k, u ) + yoffset;

		if ( x1 >= 0 && x1 < img->width() &&
			y1 >= 0 && y1 < img->height() )
		{
			img->surface().setPixel( x1, y1, color );
		}
	}
}

static void drawChaikin( const float* x, const float* y, int n,
	Image* img, long color, int yoffset )
{
	if ( n < 1000 )
	{
		Vector<float> x1( Allocator<float>(__FILE__,__LINE__) );
		Vector<float> y1( Allocator<float>(__FILE__,__LINE__) );
		x1.setSize( (n-1)*2 ); x1.clear();
		y1.setSize( (n-1)*2 ); y1.clear();
		for ( int i = 0 ; i+1 < n ; ++i )
		{
			float xq = .75f * x[i] + .25f * x[i+1];
			float xr = .25f * x[i] + .75f * x[i+1];
			x1.add( xq );
			x1.add( xr );

			float yq = .75f * y[i] + .25f * y[i+1];
			float yr = .25f * y[i] + .75f * y[i+1];
			y1.add( yq );
			y1.add( yr );
		}

		drawChaikin( x1.begin(), y1.begin(), x1.size(), img, color, yoffset );
	}
	else
	{
		for ( int i = 0 ; i < n ; ++i )
		{
			int x1 = (int)x[i];
			int y1 = (int)y[i] + yoffset;
			if ( x1 >= 0 && x1 < img->width() &&
				y1 >= 0 && y1 < img->height() )
				img->surface().setPixel( x1, y1, color );
		}
	}
}

static int test()
{
	Image img( 500, 500, SurfaceFormat::SURFACE_R8G8B8 );

	// draw simple curves
	const int n = 4;
	float x[n] = {450,50,50,450};
	float y[n] = {50,50,450,450};
	//float x[n] = {450,50,50,450};
	//float y[n] = {50,450,50,450};
	float len = drawBezier( x, y, n, &img, 0xFF0000 );
	printf( "  curve distance (brute force) = %g\n", len );

	// profile distance computations
	Vector3 p[n];
	for ( int i = 0 ; i < n ; ++i )
		p[i] = Vector3( x[i], y[i], 0.f );
	float len1, len2;
	TimeStamp t0;
	for ( int i = 0 ; i < 1000 ; ++i )
		len1 = BezierUtilVector3::getCubicBezierCurveLength(p,0,1);
	TimeStamp t1;
	for ( int i = 0 ; i < 1000 ; ++i )
		len2 = BezierUtilVector3::getCubicBezierCurveLength2(p,0,1,1.f);
	TimeStamp t2;
	printf( "  curve length (integration) = %g, time %g\n", len1, (t1-t0).seconds() );
	printf( "  curve length (approx) = %g, time %g, %.0f times slower\n", len2, (t2-t1).seconds(), (t2-t1).seconds()/(t1-t0).seconds() );
	//drawBspline( x, y, 3, 3, &img, 0xFF, 8 );
	//drawChaikin( x, y, n, &img, 0xFFFF, 16 );

	const int n2 = 6;
	float x1[n2] = {450,50,50,450,450,50};
	float y1[n2] = {50,50,250,250,450,450};
	//drawBspline( x1, y1, n2, 4, &img, 0xFF00 );

	// surface test
	Vector3 patch[4][4] =
	{
		{Vector3(50,50,0), Vector3(150,50,0), Vector3(250,50,0), Vector3(350,50,0)},
		{Vector3(50,150,0), Vector3(150,150,100), Vector3(250,150,255), Vector3(350,150,0)},
		{Vector3(50,250,0), Vector3(150,250,50), Vector3(250,250,200), Vector3(350,250,0)},
		{Vector3(50,350,0), Vector3(150,350,0), Vector3(250,350,0), Vector3(350,350,0)},
	};
	int samples = 400;
	#ifdef _DEBUG
	samples = 100;
	#endif
	for ( int i = 0 ; i < samples ; ++i )
		for ( int j = 0 ; j < samples ; ++j )
		{
			float u = (float)i / samples;
			float v = (float)j / samples;

			Vector3 p = BezierUtil<Vector3>::getCubicBezierPatch( patch, u, v );
			setPixel( &img, p.x, p.y, p.z, 0x00 );
		}

	// patch flatness
	float flat = BezierUtilVector3::getCubicBezierPatchFlatness(patch);
	printf( "  flatness = %g\n", flat );

	// save output
	io::FileOutputStream out( "/temp/curve.bmp" );
	img.save( &out, "/temp/curve.bmp" );
	out.close();
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
