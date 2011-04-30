/*
 * Packs sequence of images to single image. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <pix/Image.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <lang/Math.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <util/Vector.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

//-----------------------------------------------------------------------------

static const char* IMGPAGER_VERSION = "1.0";

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace util;
using namespace lang;

//-----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
	try
	{
		printf( "imgpager %s -- Packs sequence of images to single image page\nCopyright (C) 2003 Cat Mother, Ltd.\n\n", IMGPAGER_VERSION );

		printf( "Image sequence name format examples:\nimg%%i.jpg -> img1.jpg, ...\nimg%%03i.jpg -> img001.jpg, ...\n\n" );

		// read image format string
		char fmt[2048];
		printf( "Image sequence name format (see above for examples): " );
		int n = scanf( "%s", fmt );
		if ( n != 1 )
		{
			printf( "Invalid image name format string\n" );
			return 1;
		}

		// read first frame number
		int firstFrame = 0;
		printf( "First frame number, inclusive: " );
		n = scanf( "%i", &firstFrame );
		if ( n != 1 || firstFrame < 1 || firstFrame > 1000000 )
		{
			printf( "Invalid first frame number, must be in range [1,1000000]\n" );
			return 1;
		}

		// read last frame number
		int lastFrame = 0;
		printf( "Last frame number, inclusive: " );
		n = scanf( "%i", &lastFrame );
		if ( n != 1 || lastFrame < firstFrame || lastFrame > 1000000 )
		{
			printf( "Invalid last frame number, must be in range [firstFrame,1000000]\n" );
			return 1;
		}

		// read output file name
		char outfname[2048];
		printf( "Output file name (tga, jpg, bmp): " );
		n = scanf( "%s", outfname );
		if ( n != 1 )
		{
			printf( "Invalid output file name\n" );
			return 1;
		}

		// load images
		int width = 0;
		int height = 0;
		Vector<P(Image)> images( Allocator<P(Image)>(__FILE__) );
		for ( int i = firstFrame ; i <= lastFrame ; ++i )
		{
			char fname[2048];
			sprintf( fname, fmt, i );

			P(FileInputStream) in = new FileInputStream( fname );
			P(Image) img = new Image( in );
			images.add( img );

			if ( !img->format().bltSupported() )
				throw Exception( Format("Blit not supported for image type {0}", fname) );

			if ( i == firstFrame )
				height = img->height();
			else if ( height != img->height() )
				throw Exception( Format("Source image {0} is not same height ({1}) as previous frame ({2})", fname, img->height(), height) );

			if ( i == firstFrame )
				width = img->width();
			else if ( width != img->width() )
				throw Exception( Format("Source image {0} is not same width ({1}) as previous frame ({2})", fname, img->width(), width) );
		}

		// power of 2 dimensions
		printf( "Writing %s...\n", outfname );
		int pixels = images.size() * width * height;
		int minDim = (int)Math::sqrt( (float)pixels );
		int dim = 4;
		while ( dim < minDim )
			dim += dim;

		// create black image
		long color = 0;
		if ( !images[0]->format().hasAlpha() )
			color = 0xFF000000;
		P(Image) img = new Image( dim, dim, SurfaceFormat::SURFACE_A8R8G8B8 );
		for ( int y = 0 ; y < img->height() ; ++y )
			for ( int x = 0 ; x < img->width() ; ++x )
				img->surface().setPixel( x, y, color );

		// blit images to new one
		int index = 0;
		int rows = 0;
		int cols = 0;
		for ( int y = 0 ; y < img->height() && index < images.size() ; y += height )
		{
			cols = 0;
			for ( int x = 0 ; x < img->width() && index < images.size() ; x += width, ++index )
			{
				Surface& surface = img->surface(0);
				surface.blt( x, y, width, height, &images[index]->surface(), 0, 0, width, height );
				++cols;
			}
			++rows;
		}

		// save output
		P(FileOutputStream) out = new FileOutputStream( outfname );
		img->save( out );

		// print ImageAnim command
		printf( "\nImageAnim command (for particle systems):\n" );
		printf( "ImageAnim %s %i %i %i <fps> <behaviour = RANDOM, LIFE, LOOP, MIRROR>\n", outfname, rows, cols, lastFrame-firstFrame+1 );
		printf( "syntax: ImageAnim <file> <rows> <columns> <total frames> <fps> <behaviour>\n" );
		
	}
	catch ( Throwable& e )
	{
		char msg[512];
		e.getMessage().format().getBytes( msg, sizeof(msg), "ASCII-7" );
		puts( msg );
		return 1;
	}
	return 0;
}
