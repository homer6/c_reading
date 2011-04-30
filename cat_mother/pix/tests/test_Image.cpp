#include <tester/Test.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceUtil.h>
#include <pix/SurfaceFormat.h>
#include <lang/String.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace lang;

//-----------------------------------------------------------------------------

static P(Image) loadImage( const String& name )
{
	P(FileInputStream) in = new FileInputStream( name );
	P(Image) img = new Image( in, name );
	in->close();
	return img;
}

static void saveImage( Image* img, const String& name )
{
	P(FileOutputStream) out = new FileOutputStream( name );
	img->save( out, name );
	out->close();
}

static int test()
{
	String outDir = "../../../tmp/out";

	{
		P(Image) image = loadImage( "data/boxtex.bmp" );
		int w = image->width();
		int h = image->height();
		P(Image) i2 = new Image( w, h, image->format() );
		Surface& i2s = i2->surface(0);
		i2s.blt( 0,0,w/2,h/2, &image->surface(0), 0,0,w,h );
		i2s.blt( w/2,h/2,w/2,h/2, &image->surface(0), 0,0,w,h );
		i2s.blt( w/2-w/6,h/2-h/6,w/3,h/3, &image->surface(0), 0,0,w,h );
		saveImage( image, outDir + "/out_boxtex0.bmp" );
		saveImage( i2, outDir + "/out_boxtex0_4.bmp" );
	}
	{
		// no dithering
		printf( "  no-dithering\n" );
		P(Image) image = loadImage( "data/space.jpg" );
		image = new Image( SurfaceFormat::SURFACE_A8R8G8B8, image );
		P(Image) img = new Image( image->width(), image->height(), SurfaceFormat::SURFACE_R5G5B5 );
		const uint32_t* src = (const uint32_t*)image->surface(0).data();
		const uint32_t* srcEnd = src + image->width() * image->height();
		uint16_t* dst = (uint16_t*)img->surface(0).data();
		for ( ; src != srcEnd ; ++src )
		{
			uint32_t pix = *src;
			uint32_t r = (pix & 0xFF0000)>>16;
			uint32_t g = (pix & 0xFF00)>>8;
			uint32_t b = pix & 0xFF;
			
			uint32_t d = 0;
			d |= (b>>3);
			d |= (g>>3)<<5;
			d |= (r>>3)<<10;
			
			*dst = (uint16_t)d;
			++dst;
		}
		saveImage( img, outDir + "/out_space1.bmp" );

		// Floyd-Steinberg dithering
		printf( "  Floyd-Steinberg dithering\n" );
		img = new Image( SurfaceFormat::SURFACE_R5G6B5, image );
		Surface& imgs = img->surface(0);
		SurfaceUtil::blt( imgs.format(), imgs.width(), imgs.height(), imgs.data(), imgs.pitch(), 
			image->surface(0).format(), image->surface(0).data(), image->surface(0).pitch() );
		saveImage( img, outDir + "/out_space2.bmp" );
	}
	{
		P(Image) image = loadImage( "data/boxtex.bmp" );
		saveImage( image, outDir + "/out_boxtex1.jpg" );
	}
	{
		P(Image) image = loadImage( "data/boxtex.jpg" );
		saveImage( image, outDir + "/out_boxtex2.bmp" );
	}
	{
		P(Image) image = loadImage( "data/boxtex.tga" );
		saveImage( image, outDir + "/out_boxtex3.bmp" );
	}
	{
		P(Image) image = loadImage( "data/boxtex2.tga" );
		saveImage( image, outDir + "/out_boxtex4.bmp" );
	}
	{
		P(Image) image = loadImage( "data/boxtex3.tga" );
		saveImage( image, outDir + "/out_boxtex5.bmp" );
		saveImage( image, outDir + "/out_boxtex5.jpg" );
	}
	{
		P(Image) image = loadImage( "data/boxtex.bmp" );
		saveImage( image, outDir + "/out_boxtex6.tga" );
	}
	{
		P(Image) image = loadImage( "data/boxtex.tga" );
		saveImage( image, outDir + "/out_boxtex7.tga" );
	}
	{
		P(Image) image = loadImage( "data/boxtex2.tga" );
		saveImage( image, outDir + "/out_boxtex8.tga" );
	}
	{
		P(Image) image = loadImage( "data/boxtex3.tga" );
		saveImage( image, outDir + "/out_boxtex9.tga" );
	}

	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
