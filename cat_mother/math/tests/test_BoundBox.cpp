#include <tester/Test.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <util/Vector.h>
#include <math/EigenUtil.h>
#include <math/Vector3.h>
#include <math/Matrix3x3.h>
#include <math/BSphereBuilder.h>
#include <math/OBBoxBuilder.h>
#include <assert.h>
#include <math/internal/config.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------
/*
static Vector3 computeMean( const Vector3* begin, const Vector3* end )
{
	int n = end - begin;
	Vector3 v(0,0,0);
	for ( ; begin != end ; ++begin )
		v += *begin;
	v *= 1.f/n;
	return v;
}

static float computeCovariance( int i, int j, 
	const Vector3* begin, const Vector3* end, const Vector3& mean )
{
	int n = end - begin;
	float cov = 0.f;
	for ( ; begin != end ; ++begin )
	{
		Vector3 p = *begin;
		cov += (p[i]-mean[i]) * (p[j]-mean[j]);
	}
	cov /= n;
	return cov;
}
*/
static void drawLine( Image& img, Vector3 a, Vector3 b )
{
	const int n = (int)((a-b).length()*1.5f+2);
	for ( int i = 0 ; i < n ; ++i )
	{
		float v = (float)i / (float)n;
		Vector3 p = a + (b-a)*v;
		int x = (int)p.x;
		int y = img.height() - 1 - (int)p.y;
		if ( x >= 0 && x < img.width() && y >= 0 && y < img.height() )
			img.surface().setPixel( x, y, 0xFF00FF00 );
	}
}

static int test()
{
	FileInputStream in( "points.bmp" );
	Image img( &in, "points.bmp" );
	
	// read points
	Vector<Vector3> points( Allocator<Vector3>(__FILE__,__LINE__) );
	for ( int j = 0 ; j < img.height() ; ++j )
		for ( int i = 0 ; i < img.width() ; ++i )
			if ( img.surface().getPixel(i,j) == 0xFF000000 )
				points.add( Vector3(i,img.height()-j-1,0) );
/*
	// compute covariance matrix
	assert( points.size() > 0 );
	Vector3 mean = computeMean( points.begin(), points.end() );
	Matrix3x3 cov(0);
	for ( int i = 0 ; i < 3 ; ++i )
		for ( int j = 0 ; j < 3 ; ++j )
			cov(i,j) = computeCovariance( i, j, points.begin(), points.end(), mean );

	// solve eigenvectors
	Matrix3x3 evec;
	EigenUtil::computeSymmetric( cov, 0, &evec );
	Matrix3x3 rot = evec.orthonormalize();

	// solve OBB dimensions
	Vector3 min(Float::MAX_VALUE,Float::MAX_VALUE,Float::MAX_VALUE);
	Vector3 max(-Float::MAX_VALUE,-Float::MAX_VALUE,-Float::MAX_VALUE);
	for ( int i = 0 ; i < points.size() ; ++i )
	{
		Vector3 p = points[i];
		
		for ( int k = 0 ; k < 3 ; ++k )
		{
			float v = p.dot( evec.getColumn(k) );
			if ( v < min[k] )
				min[k] = v;
			if ( v > max[k] )
				max[k] = v;
		}
	}
	Vector3 dim = (max-min)*.5f;
	Vector3 center = rot.getColumn(0)*(max.x+min.x)*.5f +
		rot.getColumn(1)*(max.y+min.y)*.5f;
*/
	// compute obb
	OBBoxBuilder obbb;
	while ( obbb.nextPass() )
		obbb.addPoints( points.begin(), points.size() );
	Vector3 center = obbb.box().translation();
	Matrix3x3 rot = obbb.box().rotation();
	Vector3 dim = obbb.box().dimensions();

	// plot box
	drawLine( img, 
		center + rot.getColumn(0)*-dim.x + rot.getColumn(1)*-dim.y,
		center + rot.getColumn(0)*dim.x + rot.getColumn(1)*-dim.y );
	drawLine( img, 
		center + rot.getColumn(0)*dim.x + rot.getColumn(1)*-dim.y,
		center + rot.getColumn(0)*dim.x + rot.getColumn(1)*dim.y );
	drawLine( img, 
		center + rot.getColumn(0)*dim.x + rot.getColumn(1)*dim.y,
		center + rot.getColumn(0)*-dim.x + rot.getColumn(1)*dim.y );
	drawLine( img, 
		center + rot.getColumn(0)*-dim.x + rot.getColumn(1)*dim.y,
		center + rot.getColumn(0)*-dim.x + rot.getColumn(1)*-dim.y );

	// compute & plot bounding sphere
	BSphereBuilder bsb;
	while ( bsb.nextPass() )
		bsb.addPoints( points.begin(), points.size() );
	BSphere bsphere = bsb.sphere();
	const int segs = 50;
	Vector3 p0 = bsphere.translation() + Vector3(1,0,0)*bsphere.radius();
	for ( int i = 1 ; i <= segs ; ++i )
	{
		float ang = (float)i / (float)segs * Math::PI * 2.f;
		
		Vector3 p1 = bsphere.translation();
		p1 += Vector3( Math::cos(ang), Math::sin(ang), 0.f ) * bsphere.radius();
		drawLine( img, p0, p1 );

		p0 = p1;
	}

	// save result
	img = Image( SurfaceFormat::SURFACE_R5G5B5, &img );
	FileOutputStream out( "/temp/points_obb.tga" );
	img.save( &out, "/temp/points_obb.tga" );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
