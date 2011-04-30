#include "SurfaceFormat.h"
#include <assert.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace pix
{


/** 
 * List of surface formats: 
 * {format, bitcount, red mask, green mask, blue mask, alpha mask}. 
 */
static const uint32_t s_formats[][6] = 
{
	{SurfaceFormat::SURFACE_UNKNOWN   ,  0, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_R8G8B8    , 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000},
	{SurfaceFormat::SURFACE_A8R8G8B8  , 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000},
	{SurfaceFormat::SURFACE_X8R8G8B8  , 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000},
	{SurfaceFormat::SURFACE_R5G6B5    , 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000},
	{SurfaceFormat::SURFACE_R5G5B5    , 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00000000},
	{SurfaceFormat::SURFACE_P4		 ,  4,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_P8		 ,  8,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_A1R5G5B5  , 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000},
	{SurfaceFormat::SURFACE_X4R4G4B4  , 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x00000000},
	{SurfaceFormat::SURFACE_A4R4G4B4  , 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000},
	{SurfaceFormat::SURFACE_R3G3B2    ,  8, 0x000000e0, 0x0000001c, 0x00000003, 0x00000000},
	{SurfaceFormat::SURFACE_R3G2B3    ,  8, 0x000000e0, 0x00000018, 0x00000007, 0x00000000},
	{SurfaceFormat::SURFACE_A8        ,  8, 0x00000000, 0x00000000, 0x00000000, 0x000000ff},
	{SurfaceFormat::SURFACE_A8R3G3B2  , 16, 0x000000e0, 0x0000001c, 0x00000003, 0x0000ff00},
	{SurfaceFormat::SURFACE_A8R3G2B3  , 16, 0x000000e0, 0x00000018, 0x00000007, 0x0000ff00},
	{SurfaceFormat::SURFACE_DXT1	  , 0,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_DXT3	  , 0,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_DXT5	  , 0,  0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_D32		  , 32, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_D16		  , 16, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
	{SurfaceFormat::SURFACE_D24S8	  , 32, 0x00000000, 0x00000000, 0x00000000, 0x00000000},
};

/** Number of supported surface formats. */
const int FORMATS = sizeof(s_formats)/sizeof(s_formats[0]);

//-----------------------------------------------------------------------------

/** 
 * Returns left shift (=zero bit count before first non-zero bit) from a bit mask.
 */
static inline int maskToShift( uint32_t mask )
{
	int count = 0;
	uint32_t testBit = 1;
	if ( 0 == mask )
		return 32;

	while ( 0 == (mask & testBit) )
	{
		++count;
		testBit += testBit;
	}
	return count;
}

/** 
 * Returns number of non-zero bits in the dword.
 */
static inline int countBits( uint32_t mask )
{
	int count = 0;
	uint32_t testBit = 1;
	while ( 0 != testBit )
	{
		if ( 0 != (mask&testBit) )
			++count;
		testBit += testBit;
	}
	return count;
}

//-----------------------------------------------------------------------------

SurfaceFormat::SurfaceFormat() :
	m_type( SurfaceFormat::SURFACE_UNKNOWN ),
	m_compressable( true )
{
	assert( SURFACE_LAST == FORMATS );
}

SurfaceFormat::SurfaceFormat( SurfaceFormatType type ) :
	m_type( type ),
	m_compressable( true )
{
	assert( (SurfaceFormatType)s_formats[type][0] == type );
	assert( SURFACE_LAST == FORMATS );

	if ( type == SurfaceFormat::SURFACE_DXT1 || type == SurfaceFormat::SURFACE_DXT3 
		|| type == SurfaceFormat::SURFACE_DXT5 )
		m_compressable = false;
}

SurfaceFormat::SurfaceFormat( int bitCount, long redMask, long greenMask, long blueMask, long alphaMask ) :
	m_compressable( true )
{
	m_type = SurfaceFormat::SURFACE_UNKNOWN;

	for ( int i = 0 ; i < FORMATS ; ++i )
	{
		if ( s_formats[i][1] == (uint32_t)bitCount &&
			s_formats[i][2] == (uint32_t)redMask &&
			s_formats[i][3] == (uint32_t)greenMask &&
			s_formats[i][4] == (uint32_t)blueMask &&
			s_formats[i][5] == (uint32_t)alphaMask )
		{
			m_type = (SurfaceFormat::SurfaceFormatType)s_formats[i][0];
		}
	}

	if ( m_type == SurfaceFormat::SURFACE_DXT1 || m_type == SurfaceFormat::SURFACE_DXT3 
		|| m_type == SurfaceFormat::SURFACE_DXT5 )
		m_compressable = false;
}

SurfaceFormat::SurfaceFormatType SurfaceFormat::type() const
{
	return m_type;
}

int SurfaceFormat::pixelSize() const
{
	assert( bltSupported() );
	return (int)(s_formats[m_type][1] >> 3);
}

bool SurfaceFormat::hasAlpha() const
{
	if ( !bltSupported() )
		return false;
	else
		return s_formats[m_type][5] > 0;
}

void SurfaceFormat::copyPixels( void* dst, const SurfaceFormat& srcFormat, const void* src, int pixels ) const
{
	assert( bltSupported() );
	assert( srcFormat.m_type != SURFACE_UNKNOWN );

	if ( bltSupported() )
	{
		const uint8_t*		srcBytes		= reinterpret_cast<const uint8_t*>( src );
		uint8_t*			dstBytes		= reinterpret_cast<uint8_t*>( dst );
		const uint32_t*		srcFmt			= &s_formats[srcFormat.m_type][1];
		const uint32_t*		dstFmt			= &s_formats[m_type][1];
		const int			srcPixelSize	= srcFmt[0] >> 3;
		const int			dstPixelSize	= dstFmt[0] >> 3;
		const uint32_t*		srcMasks		= &srcFmt[1];
		const uint32_t*		dstMasks		= &dstFmt[1];
		int					srcBits[4];
		int					dstBits[4];
		int					srcShifts[4];
		int					dstShifts[4];

		// compute aux channel variables
		for ( int i = 0 ; i < 4 ; ++i )
		{
			srcBits[i] = countBits( srcMasks[i] );
			dstBits[i] = countBits( dstMasks[i] );
			srcShifts[i] = maskToShift( srcMasks[i] );
			dstShifts[i] = maskToShift( dstMasks[i] );
		}

		for ( int i = 0 ; i < pixels ; ++i )
		{
			// read pixel
			uint32_t srcPixel = *srcBytes;
			switch ( srcPixelSize )
			{
			case 4:	srcPixel |= (uint32_t)srcBytes[3] << 24;
			case 3:	srcPixel |= (uint32_t)srcBytes[2] << 16;
			case 2:	srcPixel |= (uint32_t)srcBytes[1] << 8;
			}

			// convert pixel
			uint32_t dstPixel = uint32_t(-1);
			for ( int k = 0 ; k < 4 ; ++k )
			{
				uint32_t srcVal = ( (srcPixel & srcMasks[k]) >> srcShifts[k] ) << (8-srcBits[k]);
				uint32_t dstVal = ( srcVal >> (8-dstBits[k]) ) << dstShifts[k];
				dstPixel = (dstPixel & ~dstMasks[k]) | dstVal;
				if ( 0 == srcBits[k] )
					dstPixel |= dstMasks[k];
			}

			// write pixel
			*dstBytes = (uint8_t)dstPixel;
			switch ( dstPixelSize )
			{
			case 4:	dstBytes[3] = (uint8_t)( dstPixel >> 24 );
			case 3:	dstBytes[2] = (uint8_t)( dstPixel >> 16 );
			case 2:	dstBytes[1] = (uint8_t)( dstPixel >> 8 );
			}

			srcBytes += srcPixelSize;
			dstBytes += dstPixelSize;
		}
	}
}

bool SurfaceFormat::operator==( const SurfaceFormat& other ) const
{
	return m_type == other.m_type;
}

bool SurfaceFormat::operator!=( const SurfaceFormat& other ) const
{
	return m_type != other.m_type;
}

long SurfaceFormat::getChannelMask( int i ) const
{
	assert( i >= 0 && i < 4 );
	assert( bltSupported() );

	if ( !bltSupported() )
		return 0;
	else
		return s_formats[m_type][2+i];
}

int SurfaceFormat::getChannelBitCount( int i ) const
{
	assert( i >= 0 && i < 4 );
	assert( bltSupported() );

	if ( !bltSupported() )
		return 0;
	else
		return countBits( s_formats[m_type][2+i] );
}

void SurfaceFormat::setCompressable( bool enabled )
{
	m_compressable = enabled;
}

bool SurfaceFormat::compressable() const
{
	return m_compressable;
}

bool SurfaceFormat::bltSupported() const 
{
	return m_type > SURFACE_UNKNOWN && m_type < SURFACE_DXT1;
}

bool SurfaceFormat::compressed() const
{
	return m_type == SURFACE_DXT1 || m_type == SURFACE_DXT3 || m_type == SURFACE_DXT5;
}


} // pix
