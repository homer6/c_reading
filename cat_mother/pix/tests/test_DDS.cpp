#include <tester/Test.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <lang/String.h>
#include <lang/Format.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#ifdef _MSC_VER
#include <config_msvc.h>
#endif

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace lang;

//-----------------------------------------------------------------------------

static String OUTPUT_DIR = "../../../tmp/out";

//-----------------------------------------------------------------------------

static P(Image) loadImage( const String& name )
{
	P(FileInputStream) in = new FileInputStream( name );
	P(Image) img = new Image( in, name );
	in->close();

	// save single-surface output
	if ( img->format() == SurfaceFormat::SURFACE_DXT1 ||
		img->format() == SurfaceFormat::SURFACE_DXT3 ||
		img->format() == SurfaceFormat::SURFACE_DXT5 )
	{
		int totalWidth = 0;
		for ( int i = 0 ; i < img->surfaces() ; ++i )
			totalWidth += img->surface(i).width();

		Image img2( totalWidth, img->height(), SurfaceFormat::SURFACE_A8R8G8B8 );
		Surface& s2 = img2.surface();
		int surfaceIndex = 0;
		int basex = 0;
		for ( int i = 0 ; i < img->surfaces() ; ++i )
		{
			const Surface& s = img->surface( i );
			for ( int j = 0 ; j < s.height() ; ++j )
				for ( int i = 0 ; i < s.width() ; ++i )
					s2.setPixel( basex+i, j, s.getPixel(i,j) );
			basex += s.width();
		}

		String basename = File(name).getName();
		if ( basename.indexOf(".") > 0 )
			basename = basename.substring( 0, basename.indexOf(".") );

		String fname = OUTPUT_DIR + "/DXT_" + basename + Format("_SF{0}",(int)img->format().type()).format() + ".tga";
		P(FileOutputStream) fout = new FileOutputStream( fname );
		img2.save( fout );
	}
	return img;
}

static printImageInfo( Image* image )
{
	printf( "Image format = %i\n", (int)image->format().type() );
	printf( "Mipmap levels : %d\n", image->mipMapLevels() );
	printf( "Total Surfaces : %d\n", image->surfaces() );
	for ( int i = 0; i < image->surfaces(); ++i )
		printf( "   Surface #%d : dimensions = %d * %d, datasize = %d\n", i, image->surface(i).width(), image->surface(i).height(), image->surface(i).dataSize() );
}

static int test()
{
	//P(Image) lmap = loadImage( "data/CELL1LightingMap.dds" );
	//printf( "Compressed DDS texture \"data/CELL1LightingMap.dds\" loaded.\n" );
	//printImageInfo( lmap );

	P(Image) rgb = loadImage( "data/DDSTex1.dds" );
	printf( "RGB DDS texture \"data/DDSTex1.dds\" loaded.\n" );
	printImageInfo( rgb );

	P(Image) rgba = loadImage( "data/earth.dds" );
	printf( "RGBA DDS texture \"data/earth.dds\" loaded. \n");
	printImageInfo( rgba );

	P(Image) rgbmips = loadImage( "data/DDSTex2.dds" );
	printf( "RGB DDS texture with mipmaps \"data/DDSTex2.dds\" loaded.\n");
	printImageInfo( rgbmips );

	P(Image) dxt = loadImage( "data/DDSTex3.dds" );
	printf( "Compressed DDS Texture \"data/DDSTex3.dds\" loaded.\n");
	printImageInfo( dxt );

	P(Image) dxtmips = loadImage( "data/DDSTex4.dds" );
	printf( "Compressed DDS Texture with mipmaps \"data/DDSTex4.dds\" loaded.\n");
	printImageInfo( dxtmips );

	P(Image) rgbcube = loadImage( "data/TestCubeTexture1.dds" );
	printf( "RGB Cube DDS Texture \"data/TestCubeTexture1.dds\" loaded. \n");
	printImageInfo( rgbcube );

	P(Image) rgbcubemips = loadImage( "data/TestCubeTexture2.dds" );
	printf( "RGB Cube DDS Texture with mipmaps \"data/TestCubeTexture2.dds\" loaded. \n");
	printImageInfo( rgbcubemips );

	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
