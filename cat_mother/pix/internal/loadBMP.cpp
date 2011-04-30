#include "loadBMP.h"
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


class BmpException {};

//-----------------------------------------------------------------------------

/**
 * Reads specified number of bytes from the file.
 * @exception BmpException
 */
static void readFully( InputStream* file, void* buffer, int bytes )
{
	int bytesRead = file->read( buffer, bytes );//fread( buffer, 1, bytes, file );
	if ( bytesRead != bytes )
		throw BmpException();
}

/**
 * Writes specified number of bytes to the file.
 * @exception BmpException
 */
static void writeFully( OutputStream* file, const void* buffer, int bytes )
{
	file->write( buffer, bytes );
	/*int bytesWritten = fwrite( buffer, 1, bytes, file );
	if ( bytesWritten != bytes )
		throw BmpException();*/
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
 * Returns 32-bit value from the buffer at specified position.
 */
static int32_t get4B( const void* data, int offset )
{
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data) + offset;
	return ( int32_t(bytes[3]) << 24 ) + ( int32_t(bytes[2]) << 16 ) + ( int32_t(bytes[1]) << 8 ) + int32_t(bytes[0]);
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
 * Writes 32-bit value to the buffer at specified position.
 */
static void put4B( void* data, int offset, int32_t value )
{
	uint8_t* bytes = reinterpret_cast<uint8_t*>(data) + offset;
	bytes[0] = (uint8_t)( value );
	bytes[1] = (uint8_t)( value>>8 );
	bytes[2] = (uint8_t)( value>>16 );
	bytes[3] = (uint8_t)( value>>24 );
}

/**
 * Reads colormap to specified array.
 */
static void readColormap( InputStream* file, uint8_t* colormap, int biMapEntrySize, int biClrUsed )
{
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
 * @exception BmpException
 */
static void skipBytes( InputStream* file, long count )
{
	if ( count < 0 )
		throw BmpException();
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

//-----------------------------------------------------------------------------

uint8_t* loadBMP( InputStream* file, int* width, int* height, SurfaceFormat* format )
{
	uint8_t* image = 0;

	try
	{
		uint8_t bmpfileheader[14];
		uint8_t bmpinfoheader[64];

		readFully( file, bmpfileheader, sizeof(bmpfileheader) );
		if ( get2B(bmpfileheader,0) != 0x4D42 )
			throw BmpException();
		long offBits = get4B( bmpfileheader, 10 );
		
		// The infoheader might be 12 bytes (OS/2 1.x), 40 bytes (Windows),
		// or 64 bytes (OS/2 2.x).  Check the first 4 bytes to find out which.
		readFully( file, bmpinfoheader, 4 );
		long headerSize = get4B( bmpinfoheader, 0 );
		if ( headerSize < 12 || headerSize > 64 )
			throw BmpException();
		readFully( file, bmpinfoheader+4, headerSize-4 );

		long	biWidth = 0;
		long	biHeight = 0;
		int		biPlanes = 0;
		int		biBitsPerPixel = 0;
		int		biMapEntrySize = 0;
		int		biClrUsed = 0;
		int		biCompression = 0;

		switch ( headerSize )
		{
		case 12:
			// Decode OS/2 1.x header (in Win32 BITMAPCOREHEADER)
			biWidth = get2B( bmpinfoheader, 4 );
			biHeight = get2B( bmpinfoheader, 6 );
			biPlanes = get2B( bmpinfoheader, 8 );
			biBitsPerPixel = get2B( bmpinfoheader, 10 );
			switch ( biBitsPerPixel )
			{
			case 8:
				biMapEntrySize = 3;
				break;
			case 24:
				break;
			default:
				throw BmpException();
			}
			break;

		case 40:
		case 64:
			// Decode Windows 3.x header (in Win32 BITMAPINFOHEADER)
			// or OS/2 2.x header, which has additional fields that we ignore
			biWidth = get2B( bmpinfoheader, 4 );
			biHeight = get2B( bmpinfoheader, 8 );
			biPlanes = get2B( bmpinfoheader, 12 );
			biBitsPerPixel = get2B( bmpinfoheader, 14 );
			biCompression = get4B( bmpinfoheader, 16 );
			biClrUsed = get4B( bmpinfoheader, 32 );

			switch ( biBitsPerPixel )
			{
			case 4:
			case 8:
				biMapEntrySize = 4;
				break;
			case 24:
			case 32:
				break;
			case 16:
			default:
				throw BmpException();
			}

			if ( 1 != biPlanes )
				throw BmpException();
			if ( 0 != biCompression )
				throw BmpException();
			break;

		default:
			throw BmpException();
		}

		// offset to bitmap data
		long bPad = offBits - (headerSize+14);
		
		// read colormap if any
		uint8_t colormap[256][4];
		if ( biMapEntrySize > 0 )
		{
			if ( biClrUsed <= 0 )
				biClrUsed = (1 << biBitsPerPixel);
			if ( biClrUsed > 256 )
				throw BmpException();
			readColormap( file, &colormap[0][0], biMapEntrySize, biClrUsed );
			bPad -= biClrUsed * biMapEntrySize;
		}
		
		// pad
		skipBytes( file, bPad );

		// row pitch aligned to 4-byte boundary
		long biPitch = (biWidth * biBitsPerPixel)/8;
		biPitch += 3;
		biPitch &= ~3;

		// find out target format
		SurfaceFormat dstFormat = SurfaceFormat::SURFACE_R8G8B8;
		if ( 16 == biBitsPerPixel )
			dstFormat = SurfaceFormat::SURFACE_R5G5B5;
		long dstPitch = long(biWidth)*long(dstFormat.pixelSize());
		
		// allocate image and a buffer for scanline
		long imgSize = long(biHeight)*dstPitch;
		image = new uint8_t[ imgSize + biPitch ];
		uint8_t* scanlinebuffer = image + imgSize;

		// read pixels
		for ( int j = (int)biHeight - 1 ; j >= 0 ; --j )
		{
			readFully( file, scanlinebuffer, biPitch );
			uint8_t* dstline = image + j*dstPitch;

			switch ( biBitsPerPixel )
			{
			case 4:{
				int k = 0;
				for ( int i = 0 ; i < (int)biWidth ; ++i )
				{
					int ind = scanlinebuffer[i>>1];
					if ( 0 == (i & 1) )
						ind >>= 4;
					ind &= 0xF;
					uint8_t red = colormap[ind][0];
					uint8_t green = colormap[ind][1];
					uint8_t blue = colormap[ind][2];
					dstline[k++] = blue;
					dstline[k++] = green;
					dstline[k++] = red;
				}
				break;}

			case 8:{
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
				break;}
			
			case 16:{
				dstFormat.copyPixels( dstline, SurfaceFormat::SURFACE_R5G5B5, scanlinebuffer, (int)biWidth );
				break;}

			case 24:{
				dstFormat.copyPixels( dstline, SurfaceFormat::SURFACE_R8G8B8, scanlinebuffer, (int)biWidth );
				break;}
			
			case 32:{
				int k = 0;
				int biWidth4 = int(biWidth)*4;
				for ( int i = 0 ; i < biWidth4 ; i += 4 )
				{
					uint8_t red = scanlinebuffer[i+2];
					uint8_t green = scanlinebuffer[i+1];
					uint8_t blue = scanlinebuffer[i+0];
					dstline[k++] = blue;
					dstline[k++] = green;
					dstline[k++] = red;
				}
				break;}
			}
		}

		// save results
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

bool saveBMP( OutputStream* file, const void* bits, int width, int height, const SurfaceFormat& format )
{
	assert( bits );
	assert( width > 0 );
	assert( height > 0 );
	assert( format != SurfaceFormat::SURFACE_UNKNOWN );

	bool ok = true;
	uint8_t* scanlinebuffer = new uint8_t[ 4*(width+1) ];

	try
	{
		long srcPitch = long(width) * long(format.pixelSize());

		long dstPitch = width*3;
		dstPitch += 3L;
		dstPitch &= ~3L;
			
		SurfaceFormat dstFormat = SurfaceFormat::SURFACE_R8G8B8;
		int biClrUsed = 0;
		int biBitsPerPixel = dstFormat.pixelSize()*8;
		long headerSize = 14 + 40 + biClrUsed*4;
		long fileSize = headerSize + dstPitch*long(height);

		uint8_t bmpfileheader[14];
		uint8_t bmpinfoheader[40];
		memset( bmpfileheader, 0, sizeof(bmpfileheader) );
		memset( bmpinfoheader, 0, sizeof(bmpinfoheader) );

		bmpfileheader[0] = 0x42;
		bmpfileheader[1] = 0x4D;
		put4B( bmpfileheader, 2, fileSize );
		put4B( bmpfileheader, 10, headerSize );

		put2B( bmpinfoheader, 0, 40 );
		put4B( bmpinfoheader, 4, width );
		put4B( bmpinfoheader, 8, height );
		put2B( bmpinfoheader, 12, 1 );
		put2B( bmpinfoheader, 14, (uint16_t)biBitsPerPixel );
		put2B( bmpinfoheader, 32, (uint16_t)biClrUsed );

		writeFully( file, bmpfileheader, sizeof(bmpfileheader) );
		writeFully( file, bmpinfoheader, sizeof(bmpinfoheader) );

		//if ( biClrUsed > 0 )
		//	writeColormap( ... );

		for ( int j = height-1 ; j >= 0 ; --j )
		{
			const uint8_t* srcline = reinterpret_cast<const uint8_t*>(bits) + srcPitch*j;
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
