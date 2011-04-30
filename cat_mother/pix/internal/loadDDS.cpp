#include "loadDDS.h"
#include "Surface.h"
#include "SurfaceFormat.h"
#include <io/InputStream.h>
#include <io/OutputStream.h>
#include <pix/Image.h>
#include <pix/SurfaceFormat.h>
#include <lang/Debug.h>
#include <lang/Array.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

namespace pix
{
	/** 
	 * Computes pitch by DDS pixel format. 
	 * @return -1 if error. 
	 */
	static int getPitch( int width, SurfaceFormat format )
	{
		int pitch = -1;
		if ( format == SurfaceFormat::SURFACE_DXT1 )
			pitch = width * 2;
		else if ( format == SurfaceFormat::SURFACE_DXT3 || format == SurfaceFormat::SURFACE_DXT5 )
			pitch = width * 4;
		else if ( format.bltSupported() )
			pitch = width * format.pixelSize();
		else
			return -1;
		
		pitch = (pitch+3) & ~3;
		return pitch;
	}

	bool readSurfaceAndMipMaps( io::InputStream* file, int width, int height, int datasize, int cellsize, int mipmapcount, SurfaceFormat format, lang::Array<Surface>& surfaces )
	{
		int pitch = getPitch( width, format );
		if ( -1 == pitch )
			return false;
		surfaces.add( Surface( width, height, pitch, datasize, format ) );
		file->read( surfaces[surfaces.size()-1].data(), datasize );				

		int w = width;
		int h = height;
		for ( int i = 0; i < mipmapcount - 1; ++i )
		{
			w = w / 2 < 1 ? 1 : w / 2;
			h = h / 2 < 1 ? 1 : h / 2;

			int mmdatasize = -1;
			if ( format.compressed() )
			{
				mmdatasize = __max(1, w / 4) * __max(1, h / 4);
				mmdatasize *= cellsize;
			}
			else
			{
				mmdatasize = w*h*format.pixelSize();
			}

			int pitch = getPitch( w, format );
			if ( -1 == pitch )
				return false;

			surfaces.add( Surface( w, h, pitch, mmdatasize, format ) );
			file->read( surfaces[surfaces.size()-1].data(), mmdatasize );
		}

		return true;
	}


	bool loadDDS( io::InputStream* file, int* width, int* height, int* pitch, int* mipmaplevels, SurfaceFormat* format, Image::ImageType* type, lang::Array<Surface>& surfaces )
	{
		const char ddsMagic[] = "DDS ";		
		char magic[4];

		file->read( &magic, 4 );
		
		if ( memcmp( &magic, &ddsMagic, 4 ) != 0 )
		{
			Debug::printlnError( "loadDDS: magic incorrect" );
			return false;
		}
		char header[124];

		file->read( &header, 124 );

		// shortcut to parameters

		uint32_t* dwSize		= (uint32_t*)&header[0];
		uint32_t* dwFlags		= (uint32_t*)&header[4];
		uint32_t* dwHeight		= (uint32_t*)&header[8];
		uint32_t* dwWidth		= (uint32_t*)&header[12];
		uint32_t* dwPitchOrSize	= (uint32_t*)&header[16];
	//	uint32_t* dwDepth		= (uint32_t*)&header[20];  // Volume depth
		uint32_t* dwMipMapCount	= (uint32_t*)&header[24];
		
		char* ddPixelFormat		= &header[28 + 4*11];		
		uint32_t* pfSize		= (uint32_t*)ddPixelFormat;
		uint32_t* pfFlags		= (uint32_t*)&ddPixelFormat[4];
		char* pfFourCC			= &ddPixelFormat[8];
		uint32_t* pfRGBBitCount	= (uint32_t*)&ddPixelFormat[12];
		uint32_t* pfRBitMask	= (uint32_t*)&ddPixelFormat[16];
		uint32_t* pfGBitMask	= (uint32_t*)&ddPixelFormat[20];
		uint32_t* pfBBitMask	= (uint32_t*)&ddPixelFormat[24];
		uint32_t* pfAlphaBitMask= (uint32_t*)&ddPixelFormat[28];

		char* ddsCaps			= &header[28 + 4*11 + 32];
		uint32_t* dwCaps1		= (uint32_t*)ddsCaps;
		uint32_t* dwCaps2		= (uint32_t*)&ddsCaps[4];

		// translate image dimensions

		if ( *dwSize != 124 )
		{
			Debug::printlnError( "loadDDS: Invalid file" );
			return false;
		}

		// translate pixelformat

		if ( *pfSize != 32 )
		{
			Debug::printlnError( "loadDDS: Invalid file" );
			return false;
		}

		int datasize = 0;
		int mindatasize = 1;

		if ( *dwFlags & DDSD_WIDTH )
			*width = (int)*dwWidth;		
		if ( *dwFlags & DDSD_HEIGHT )
			*height = (int)*dwHeight;
		if ( *dwFlags & DDSD_PITCH )
		{
			*pitch = (int)*dwPitchOrSize;
			datasize = *pitch * *height;
		}
		else if ( *dwFlags & DDSD_LINEARSIZE )
			datasize = (int)*dwPitchOrSize;
		else
		{
			if ( *pfFlags & DDPF_FOURCC )
			{
				datasize = __max(1, *width / 4) * __max(1, *height / 4) * 8;
				if ( memcmp(pfFourCC, "DXT1", 4) != 0 ) 
					datasize *= 2; 
			}
			else 
			if ( ( *dwFlags & DDSD_WIDTH ) && ( *dwFlags & DDSD_HEIGHT ) && ( *dwFlags & DDSD_PIXELFORMAT ) )
				datasize = (int)*dwHeight * (int)*dwWidth * ( ((int)*pfRGBBitCount) >> 3 );
		}
		if ( *dwFlags & DDSD_MIPMAPCOUNT )
			*mipmaplevels = (int)*dwMipMapCount;


		if ( *pfFlags & DDPF_RGB )
		{
			if ( !(*pfFlags & DDPF_ALPHAPIXELS) )
				*pfAlphaBitMask = 0;
			
			SurfaceFormat fmt( *pfRGBBitCount, *pfRBitMask, *pfGBitMask, *pfBBitMask, *pfAlphaBitMask );
			if ( fmt.type() != SurfaceFormat::SURFACE_UNKNOWN )
				*format = fmt;
			else
			{
				Debug::printlnError( "loadDDS: Unsupported Pixelformat" );
				return false;
			}

			mindatasize = ((int)*pfRGBBitCount) >> 3;
		}
		else if ( *pfFlags & DDPF_FOURCC )
		{
			if (memcmp(pfFourCC, "DXT1", 4) == 0 )
			{
				*format = SurfaceFormat( SurfaceFormat::SURFACE_DXT1 );
				*pitch = *width * 2;
				mindatasize = 8;
			}
			else if (memcmp(pfFourCC, "DXT3", 4) == 0 )
			{
				*format = SurfaceFormat( SurfaceFormat::SURFACE_DXT3 );
				*pitch = *width * 4;
				mindatasize = 16;
			}
			else if (memcmp(pfFourCC, "DXT5", 4) == 0 )
			{
				*format = SurfaceFormat( SurfaceFormat::SURFACE_DXT5 );
				*pitch = *width * 4;
				mindatasize = 16;
			}
			else
			{
				Debug::printlnError( "loadDDS: Unsupported Pixelformat" );
				return false;
			}
		}	

		if ( datasize == 0 )
		{
			Debug::printlnError( "loadDDS: Invalid file" );
			return false;
		}

		bool ok = false;
		if ( *dwCaps2 & DDSCAPS2_CUBEMAP )
		{
			if ( *dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEX )
				ok = readSurfaceAndMipMaps( file, *width, *height, datasize, mindatasize, (int)*dwMipMapCount, *format, surfaces );

			if ( *dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEX )
				ok = readSurfaceAndMipMaps( file, *width, *height, datasize, mindatasize, (int)*dwMipMapCount, *format, surfaces );

			if ( *dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEY )
				ok = readSurfaceAndMipMaps( file, *width, *height, datasize, mindatasize, (int)*dwMipMapCount, *format, surfaces );

			if ( *dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEY )
				ok = readSurfaceAndMipMaps( file, *width, *height, datasize, mindatasize, (int)*dwMipMapCount, *format, surfaces );

			if ( *dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEZ )
				ok = readSurfaceAndMipMaps( file, *width, *height, datasize, mindatasize, (int)*dwMipMapCount, *format, surfaces );

			if ( *dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEZ )
				ok = readSurfaceAndMipMaps( file, *width, *height, datasize, mindatasize, (int)*dwMipMapCount, *format, surfaces );
		}
		else
		{
			ok = readSurfaceAndMipMaps( file, *width, *height, datasize, mindatasize, (int)*dwMipMapCount, *format, surfaces );
		}
		// find image type and return

		if ( ok )
		{
			if (*dwCaps2 & DDSCAPS2_CUBEMAP)
				*type = Image::TYPE_CUBEMAP;
			else if (*dwCaps1 & DDSCAPS_TEXTURE)
				*type = Image::TYPE_BITMAP;
			else
				*type = Image::TYPE_CUSTOM;
		}

		return ok;
	}


} // pix

