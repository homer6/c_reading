#include <anim/VectorInterpolator.h>
#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/SurfaceFormat.h>
#include <lang/Math.h>
#include <tester/Test.h>
#include <assert.h>
#include <anim/internal/config.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace lang;
using namespace anim;

//-----------------------------------------------------------------------------

static int test()
{
	FileOutputStream out( "/tmp/out/interp.bmp" );
	Image img( 500, 500, SurfaceFormat::SURFACE_R8G8B8 );

	// lerp test
	{VectorInterpolator vi( 1 );
	vi.setInterpolation( VectorInterpolator::INTERPOLATE_LINEAR );
	vi.setKeys( 2 );
	vi.setKeyTime( 0, 3 );
	vi.setKeyTime( 1, 4 );
	float v0[] = {3};
	float v1[] = {5};
	vi.setKeyValue( 0, v0, 1 );
	vi.setKeyValue( 1, v1, 1 );
	float v = -1.f;
	vi.getValue( 3.5f, &v, 1, 0 );
	assert( Math::abs(v-4.f) < 0.01f );}

	// draw 'accelerating' Catmull-Rom spline
	{VectorInterpolator vi( 2 );
	vi.setInterpolation( VectorInterpolator::INTERPOLATE_CATMULLROM );
	float times[] = {0,2,5,9};
	float values[] = {450,450, 50,450, 50,50, 450,50};
	const int keys = sizeof(times) / sizeof(times[0]);
	const int chn = sizeof(values) / sizeof(values[0]) / keys;
	vi.setKeys( keys );
	for ( int i = 0 ; i < keys ; ++i )
	{
		vi.setKeyTime( i, times[i] );
		vi.setKeyValue( i, values+i*2, chn );
	}
	int hint = 0;
	int points = 100;
	for ( int i = 0 ; i < points ; ++i )
	{
		float u = (float)i / points;
		float v[2];
		hint = vi.getValue( u*(times[keys-1]-times[0])+times[0], v, 2, hint );
		if ( v[0] > 0 && v[0] < img.width() && v[1] > 0 && v[1] < img.height() )
		{
			int x = (int)v[0];
			int y = img.height() - 1 - (int)v[1];
			img.setPixel( x, y, 0xFF );
			if ( x+1 < img.width() )
				img.setPixel( x+1, y, 0xFF );
			if ( y+1 < img.height() )
				img.setPixel( x, y+1, 0xFF );
			if ( x+1 < img.width() && y+1 < img.height() )
				img.setPixel( x+1, y+1, 0xFF );
		}
	}
	}

	img.save( &out, out.toString() );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
