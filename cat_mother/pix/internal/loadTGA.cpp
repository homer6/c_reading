#include "loadTGA.h"
#include "SurfaceFormat.h"
#include <io/InputStream.h>
#include <io/OutputStream.h>
#include <string.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;

//-----------------------------------------------------------------------------

namespace pix
{


class TgaException {};

//-----------------------------------------------------------------------------

/**
 * Reads specified number of bytes from the file.
 * @exception TgaException
 */
static void readFully( InputStream* file, void* buffer, int bytes )
{
	int bytesRead = file->read( buffer, bytes );//fread( buffer, 1, bytes, file );
	if ( bytesRead != bytes )
		throw TgaException();
}

/**
 * Writes specified number of bytes to the file.
 * @exception TgaException
 */
static void writeFully( OutputStream* file, const void* buffer, int bytes )
{
	file->write( buffer, bytes );
	/*int bytesWritten = fwrite( buffer, 1, bytes, file );
	if ( bytesWritten != bytes )
		throw TgaException();*/
}

/**
 * Returns 16-bit value from the buffer at specified position.
 */
static uint16_t get2B( const void* data, int offset )
{
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data) + offset;
	return uint16_t( ( unsigned(bytes[1]) << 8 ) + unsigned(bytes[0]) );
}

/**
 * Writes 16-bit value to the buffer at specified position.
 */
static void put2B( void* data, int offset, uint16_t value )
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(data) + offset;
	bytes[0] = (uint8_t)( value );
	bytes[1] = (uint8_t)( value>>8 );
}

/**
 * Reads colormap to specified array.
 * @exception TgaException
 */
static void readColormap( InputStream* file, uint8_t* colormap, int biMapEntrySize, int biClrUsed )
{
	if ( biMapEntrySize != 3 )
		throw TgaException();

	uint8_t colr[4];
	for ( int i = 0 ; i < biClrUsed ; ++i )
	{
		memset( colr, 0, sizeof(colr) );
		readFully( file, colr, biMapEntrySize );
		colormap[i*4+0] = colr[2];
		colormap[i*4+1] = colr[1];
		colormap[i*4+2] = colr[0];
		colormap[i*4+3] = colr[3];
	}
}

/** 
 * Skips specified number of bytes from the file. 
 * @exception TgaException
 */
static void skipBytes( InputStream* file, long count )
{
	if ( count < 0 )
		throw TgaException();

	while ( count > 0 )
	{
		uint8_t padBuff[16];
		long bytes = count;
		if ( bytes > sizeof(padBuff) )
			bytes = sizeof(padBuff);
		readFully( file, padBuff, bytes );
		count -= bytes;
	}
}

/** Inverts alpha channel, i.e. new a = 1-a. */
/*static void invertAlphaChannelA8R8G8B8( uint8_t* pixels, int biWidth )
{
	for ( int i = 0 ; i < biWidth ; ++i )
		pixels[i*4+3] = (uint8_t)(255 - pixels[i*4+3]);
}*/

//-----------------------------------------------------------------------------

uint8_t* loadTGA( InputStream* file, int* width, int* height, SurfaceFormat* format )
{
	uint8_t* image = 0;

	try
	{
		uint8_t targaheader[18];
		readFully( file, targaheader, sizeof(targaheader) );

		int		biIdLen = targaheader[0];
		int		biCmapType = targaheader[1];
		int		biSubType = targaheader[2];
		int		biMapLen = get2B( targaheader, 5 );
		int		biMapEntrySize = targaheader[7]/8;
		long	biWidth = get2B( targaheader, 12 );
		long	biHeight = get2B( targaheader, 14 );
		int		biBitsPerPixel = targaheader[16];
		int		biImageDescr = targaheader[17];
		bool	biBottomUp = (0 == (0x20 & biImageDescr));
		int		biInterlaceType = (int)( unsigned(biImageDescr) >> 6 );
		bool	biRLE = (biSubType > 8);
		long	biPitch = biWidth * ((biBitsPerPixel+7)/8);

		if ( biCmapType > 1 || 0 != (biBitsPerPixel%8) || 0 != biInterlaceType )
			throw TgaException();
		if ( biRLE )
			throw TgaException();

		skipBytes( file, biIdLen );

		// read colormap
		uint8_t colormap[256][4];
		if ( biMapLen > 0 )
		{
			if ( biMapLen > 256 || 0 != get2B(targaheader,3) )
				throw TgaException();

			readColormap( file, &colormap[0][0], biMapEntrySize, biMapLen );
		}
		else if ( 0 != biCmapType )
			throw TgaException();

		// find out target format
		SurfaceFormat dstFormat = SurfaceFormat::SURFACE_R8G8B8;
		if ( 16 == biBitsPerPixel )
			dstFormat = SurfaceFormat::SURFACE_R5G5B5;
		else if ( 32 == biBitsPerPixel )
			dstFormat = SurfaceFormat::SURFACE_A8R8G8B8;
		long dstPitch = long(biWidth)*long(dstFormat.pixelSize());
			
		// allocate image
		long imgSize = long(biHeight)*dstPitch;
		image = new uint8_t[ imgSize + biPitch ];
		uint8_t* scanlinebuffer = image + imgSize;
		
		// read pixels
		for ( int j = 0 ; j < biHeight ; ++j )
		{
			readFully( file, scanlinebuffer, biPitch );

			int j1 = j;
			if ( biBottomUp )
				j1 = biHeight - j - 1;
			uint8_t* dstline = image + dstPitch*j1;

			switch ( biSubType )
			{
			case 1: // colormapped
				if ( 8 == biBitsPerPixel && 1 == biCmapType )
				{
					int k = 0;
					for ( int i = 0 ; i < (int)biWidth ; ++i )
					{
						int ind = scanlinebuffer[i];
						uint8_t red = colormap[ind][0];
						uint8_t green = colormap[ind][1];
						uint8_t blue = colormap[ind][2];
						dstline[k++] = blue;
						dstline[k++] = green;
						dstline[k++] = red;
					}
				}
				else
				{
					throw TgaException();
				}
				break;

			case 2: // RGB
				switch ( biBitsPerPixel )
				{
				case 16:
					dstFormat.copyPixels( dstline, SurfaceFormat::SURFACE_R5G5B5, scanlinebuffer, biWidth );
					break;
				case 24:
					dstFormat.copyPixels( dstline, SurfaceFormat::SURFACE_R8G8B8, scanlinebuffer, biWidth );
					break;
				case 32:
					dstFormat.copyPixels( dstline, SurfaceFormat::SURFACE_A8R8G8B8, scanlinebuffer, biWidth );
					break;
				default:
					throw TgaException();
				}
				break;

			case 3: // grayscale
				if ( 8 == biBitsPerPixel )
				{
					int k = 0;
					for ( int i = 0 ; i < (int)biWidth ; ++i )
					{
						int ind = scanlinebuffer[i];
						uint8_t red	= (uint8_t)ind;
						uint8_t green = (uint8_t)ind;
						uint8_t blue = (uint8_t)ind;
						dstline[k++] = blue;
						dstline[k++] = green;
						dstline[k++] = red;
					}
				}
				else
				{
					throw TgaException();
				}
				break;

			default:
				throw TgaException();
			}
		}

		*width = biWidth;
		*height = biHeight;
		*format = dstFormat;
	}
	catch ( ... )
	{
		if ( image ) {delete[] image; image = 0;}
	}
	return image;
}

bool saveTGA( OutputStream* file, const void* bits, int width, int height, const SurfaceFormat& format )
{
	assert( bits );
	assert( width > 0 );
	assert( height > 0 );
	assert( format != SurfaceFormat::SURFACE_UNKNOWN );

	bool ok = true;
	uint8_t* scanlinebuffer = new uint8_t[ 4*width+4 ];

	try
	{
		long srcPitch = (long)width * (long)format.pixelSize();

		uint8_t targaheader[18];
		memset( targaheader, 0, sizeof(targaheader) );
		
		put2B( targaheader, 12, (uint16_t)width );
		put2B( targaheader, 14, (uint16_t)height );
		targaheader[17] = 0x20; // top-down non-interlaced

		SurfaceFormat dstFormat = SurfaceFormat::SURFACE_R8G8B8;
		if ( format.hasAlpha() )
			dstFormat = SurfaceFormat::SURFACE_A8R8G8B8;
		else if ( 2 == format.pixelSize() )
			dstFormat = SurfaceFormat::SURFACE_R5G5B5;
		long dstPitch =  (long)width * (long)dstFormat.pixelSize();

		targaheader[2] = 2; // uncompressed (A)RGB
		targaheader[16] = (uint8_t)( 8*dstFormat.pixelSize() );

		writeFully( file, targaheader, sizeof(targaheader) );

		// write pixels
		for ( int j = 0 ; j < height ; ++j )
		{
			const uint8_t* srcline = reinterpret_cast<const uint8_t*>(bits) + j*srcPitch;
			dstFormat.copyPixels( scanlinebuffer, format, srcline, width );
			writeFully( file, scanlinebuffer, dstPitch );
		}
	}
	catch ( ... )
	{
		ok = false;
	}

	delete[] scanlinebuffer; scanlinebuffer = 0;
	return ok;
}


} // pix
