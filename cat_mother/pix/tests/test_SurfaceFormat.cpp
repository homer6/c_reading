#include <tester/Test.h>
#include <pix/Image.h>
#include <pix/SurfaceFormat.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace pix;

//-----------------------------------------------------------------------------

static int test()
{
	SurfaceFormat	sourceFormat			= SurfaceFormat::SURFACE_A8R8G8B8;
	const int		pixelCount				= 8;
	uint32_t		source[ pixelCount ]	= 
	{ 
		0xF0806040,0xF0806040, 0xF1816141,0xF1816141, 
		0xF2816141,0xF2816141, 0xF3816141,0xF3816141 
	};

	// two 32->16
	
	SurfaceFormat targetFormat = SurfaceFormat::SURFACE_R5G5B5;
	uint16_t target[ pixelCount ];
	targetFormat.copyPixels( target, sourceFormat, source, pixelCount );

	SurfaceFormat targetFormat2 = SurfaceFormat::SURFACE_A4R4G4B4;
	uint16_t target2[ pixelCount ];
	targetFormat2.copyPixels( target2, sourceFormat, source, pixelCount );

	int i;
	for ( i = 0 ; i < pixelCount ; ++i )
	{
		assert( target[i] == 0xC188 );
		assert( target2[i] == 0xF864 );
	}

	// 16->32, alpha channel default

	sourceFormat.copyPixels( source, targetFormat, target, pixelCount );
	for ( i = 0 ; i < pixelCount ; ++i )
	{
		assert( source[i] == 0xFF806040 );
	}

	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
