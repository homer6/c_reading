#include "SurfaceUtil.h"
#include "SurfaceFormat.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace pix
{


/** Tempory dynamic buffer. */
template <class T> class TempArray
{
public:
	explicit TempArray( int size )
	{
		m_data = new T[size];
		m_size = size;
	}

	~TempArray()
	{
		delete[] m_data;
	}

	T* data()
	{
		return m_data;
	}

	int size() const
	{
		return m_size;
	}

private:
	T*	m_data;
	int	m_size;

	TempArray( const TempArray<T>& );
	TempArray<T>& operator=( const TempArray<T>& );
};

struct DXTColBlock
{
	uint16_t	col0;
	uint16_t	col1;
	uint8_t		rows[4];
};

struct DXTAlphaBlockExplicit
{
	uint16_t	rows[4];
};

struct DXTAlphaBlock3BitLinear
{
	uint8_t		alpha0;
	uint8_t		alpha1;
	uint8_t		values[6];
};

//-----------------------------------------------------------------------------

/** 
 * Returns left shift (=zero bit count before first non-zero bit) from a bit mask.
 */
static inline int getShift( uint32_t mask )
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

/**
 * Computes 8-bit error mask from source and destination bit counts.
 */
static inline uint32_t makeErrorMask( int srcBits, int dstBits )
{
	if ( srcBits <= dstBits )
	{
		return 0;
	}
	else
	{
		int diffBits = srcBits - dstBits;
		uint32_t mask = (1 << diffBits) - 1;
		return mask;
	}
}

/** 
 * Clears an array with zero bytes.
 */
template <class T> static void zero( T* d, int size )
{
	memset( d, 0, sizeof(T)*size );
}

//-----------------------------------------------------------------------------

/** Returns red component of R5G6B5 in range [0,255]. */
static inline uint32_t getRedR5G6B5( uint32_t c )
{
	return (c>>11) * 255 / 31;
}

/** Returns green component of R5G6B5 in range [0,255]. */
static inline uint32_t getGreenR5G6B5( uint32_t c )
{
	return ((c>>5)&0x3F) * 255 / 63;
}

/** Returns blue component of R5G6B5 in range [0,255]. */
static inline uint32_t getBlueR5G6B5( uint32_t c )
{
	return (c&0x1F) * 255 / 31;
}

/** Returns ARGB8888 from values in range [0,255]. */
static inline uint32_t makeA8R8G8B8( uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha )
{
	return (alpha<<24) + (red<<16) + (green<<8) + blue;
}

/** Returns R5G6B5 as ARGB8888. */
static inline uint32_t convertR5G6B5toARGB8888( uint32_t c )
{
	return makeA8R8G8B8( getRedR5G6B5(c), getGreenR5G6B5(c), getBlueR5G6B5(c), 0xFF );
}

/** Gets DXT1-3 block colors. */
static inline void getBlockColorsA8R8G8B8( const DXTColBlock* colorBlock, uint32_t c[4] )
{
	if ( colorBlock->col0 > colorBlock->col1 ) 
	{
		c[0] = convertR5G6B5toARGB8888( colorBlock->col0 );
		c[1] = convertR5G6B5toARGB8888( colorBlock->col1 );

		// Four-color block: derive the other two colors.
		// 00 = c[0], 01 = c[1], 10 = c[2], 11 = c[3]
		// These 2-bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block.
		c[2] = makeA8R8G8B8( 
			(2*getRedR5G6B5(colorBlock->col0)+getRedR5G6B5(colorBlock->col1)+1)/3, 
			(2*getGreenR5G6B5(colorBlock->col0)+getGreenR5G6B5(colorBlock->col1)+1)/3,
			(2*getBlueR5G6B5(colorBlock->col0)+getBlueR5G6B5(colorBlock->col1)+1)/3,
			255 );

		c[3] = makeA8R8G8B8( 
			(getRedR5G6B5(colorBlock->col0)+2*getRedR5G6B5(colorBlock->col1)+1)/3, 
			(getGreenR5G6B5(colorBlock->col0)+2*getGreenR5G6B5(colorBlock->col1)+1)/3,
			(getBlueR5G6B5(colorBlock->col0)+2*getBlueR5G6B5(colorBlock->col1)+1)/3,
			255 );
	}	 
	else
	{ 
		c[0] = convertR5G6B5toARGB8888( colorBlock->col0 );
		c[1] = convertR5G6B5toARGB8888( colorBlock->col1 );

		// Three-color block: derive the other color.
		// 00 = c[0],  01 = c[1],  10 = c[2],  
		// 11 = transparent.
		// These 2-bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block. 
		c[2] = makeA8R8G8B8( 
			(getRedR5G6B5(colorBlock->col0)+getRedR5G6B5(colorBlock->col1))/2,
			(getGreenR5G6B5(colorBlock->col0)+getGreenR5G6B5(colorBlock->col1))/2,
			(getBlueR5G6B5(colorBlock->col0)+getBlueR5G6B5(colorBlock->col1))/2,
			255 );

		c[3] = makeA8R8G8B8(0,0,0,0);
	}
}

/** Gets DXT4/5 block alpha values. */
static inline void getBlockAlphas( const DXTAlphaBlock3BitLinear* alphaBlock, uint32_t a[8] )
{
	// 8-alpha or 6-alpha block?   
	a[0] = alphaBlock->alpha0;
	a[1] = alphaBlock->alpha1;

	if ( alphaBlock->alpha0 > alphaBlock->alpha1 )
	{    
		// 8-alpha block:  derive the other six alphas.
		// Bit code 000 = a[0], 001 = a[1], others are interpolated.
		a[2] = (6 * alphaBlock->alpha0 + 1 * alphaBlock->alpha1 + 3) / 7;		// Bit code 010
		a[3] = (5 * alphaBlock->alpha0 + 2 * alphaBlock->alpha1 + 3) / 7;		// Bit code 011
		a[4] = (4 * alphaBlock->alpha0 + 3 * alphaBlock->alpha1 + 3) / 7;		// Bit code 100
		a[5] = (3 * alphaBlock->alpha0 + 4 * alphaBlock->alpha1 + 3) / 7;		// Bit code 101
		a[6] = (2 * alphaBlock->alpha0 + 5 * alphaBlock->alpha1 + 3) / 7;		// Bit code 110
		a[7] = (1 * alphaBlock->alpha0 + 6 * alphaBlock->alpha1 + 3) / 7;		// Bit code 111  
	}    
	else
	{  
		// 6-alpha block.
		// Bit code 000 = a[0], 001 = a[1], others are interpolated.
		a[2] = (4 * alphaBlock->alpha0 + 1 * alphaBlock->alpha1 + 2) / 5;		// Bit code 010
		a[3] = (3 * alphaBlock->alpha0 + 2 * alphaBlock->alpha1 + 2) / 5;		// Bit code 011
		a[4] = (2 * alphaBlock->alpha0 + 3 * alphaBlock->alpha1 + 2) / 5;		// Bit code 100
		a[5] = (1 * alphaBlock->alpha0 + 4 * alphaBlock->alpha1 + 2) / 5;		// Bit code 101
		a[6] = 0;																// Bit code 110
		a[7] = 255;																// Bit code 111
	}
}

/** Reads ARGB8888 pixel value from DXT1-compressed texture. */
static inline long getPixelDXT1( int x, int y, 
	const void* data, int pitch )
{
	int xblock = x >> 2;
	int yblock = y >> 2;

	const uint8_t* blockData = reinterpret_cast<const uint8_t*>(data) + xblock*8 + yblock*pitch;
	const DXTColBlock* colorBlock = reinterpret_cast<const DXTColBlock*>( blockData );

	uint32_t c[4];
	getBlockColorsA8R8G8B8( colorBlock, c );
	
	uint8_t b = colorBlock->rows[ y & 3 ];
	return c[ ((b>>(x&3)*2)&3) ];
}

/** Reads ARGB8888 pixel value from DXT2/3-compressed texture. */
static inline long getPixelDXT3( int x, int y, 
	const void* data, int pitch )
{
	int xblock = x >> 2;
	int yblock = y >> 2;

	const uint8_t* blockData = reinterpret_cast<const uint8_t*>(data) + xblock*16 + yblock*pitch;
	const DXTAlphaBlockExplicit* alphaBlock = reinterpret_cast<const DXTAlphaBlockExplicit*>( blockData );
	const DXTColBlock* colorBlock = reinterpret_cast<const DXTColBlock*>( blockData+8 );

	uint32_t c[4];
	getBlockColorsA8R8G8B8( colorBlock, c );
	uint8_t b = colorBlock->rows[ y & 3 ];
	uint32_t color = c[ ((b>>(x&3)*2)&3) ];

	uint32_t alpha = ( alphaBlock->rows[y&3] >> (4*(x&3)) ) & 0xF;
	alpha = (alpha * 255) / 15;

	return (color&0xFFFFFF) + (alpha<<24);
}

/** Reads ARGB8888 pixel value from DXT4/5-compressed texture. */
static inline long getPixelDXT5( int x, int y, 
	const void* data, int pitch )
{
	int xblock = x >> 2;
	int yblock = y >> 2;

	const uint8_t* blockData = reinterpret_cast<const uint8_t*>(data) + xblock*16 + yblock*pitch;
	const DXTAlphaBlock3BitLinear* alphaBlock = reinterpret_cast<const DXTAlphaBlock3BitLinear*>( blockData );
	const DXTColBlock* colorBlock = reinterpret_cast<const DXTColBlock*>( blockData+8 );

	uint32_t c[4];
	getBlockColorsA8R8G8B8( colorBlock, c );
	uint8_t b = colorBlock->rows[ y & 3 ];
	uint32_t color = c[ ((b>>(x&3)*2)&3) ];

	uint32_t a[8];
	getBlockAlphas( alphaBlock, a );
	
	// first two rows of 4 pixels each:
	const uint32_t mask = 0x00000007;		// bits = 00 00 01 11
	uint32_t bits = *reinterpret_cast<const uint32_t*>( alphaBlock->values );
	uint8_t alphaBits[4][4];
	alphaBits[0][0] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[0][1] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[0][2] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[0][3] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[1][0] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[1][1] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[1][2] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[1][3] = (uint8_t)( bits & mask );

	// now for last two rows:
	bits = *reinterpret_cast<const uint32_t*>( alphaBlock->values+3 );
	alphaBits[2][0] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[2][1] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[2][2] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[2][3] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[3][0] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[3][1] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[3][2] = (uint8_t)( bits & mask );
	bits >>= 3;
	alphaBits[3][3] = (uint8_t)( bits & mask );

	// get alpha value
	uint32_t alpha = a[ alphaBits[y&3][x&3] & 7 ];
	
	return (color&0xFFFFFF) + (alpha<<24);
}

//-----------------------------------------------------------------------------

void SurfaceUtil::blt( 
	const SurfaceFormat& dstFormat, int dstWidth, int dstHeight, void* dstData, int dstPitch,
	const SurfaceFormat& srcFormat, const void* srcData, int srcPitch )
{
	const int		width			= dstWidth;
	const int		height			= dstHeight;
	const int		srcPixelSize	= srcFormat.pixelSize();
	const int		dstPixelSize	= dstFormat.pixelSize();

	// error diffusion buffers (4 channels, 2 rows, 2 pixels extra)
	const int CHANNELS = 4;
	const int BUFFER_ROWS = 2;
	const int BUFFER_EXTRA = BUFFER_ROWS;
	const int BUFFER_PITCH = width + BUFFER_EXTRA;
	TempArray<uint32_t> buffer( BUFFER_PITCH*CHANNELS*BUFFER_ROWS );
	zero( buffer.data(), buffer.size() );
	uint32_t* buffers[CHANNELS][BUFFER_ROWS];
	for ( int i = 0 ; i < CHANNELS ; ++i )
	{
		for ( int j = 0 ; j < BUFFER_ROWS ; ++j )
		{
			buffers[i][j] = buffer.data() + 1 + BUFFER_PITCH*(i*BUFFER_ROWS+j);
			assert( buffers[i][j]-1 >= buffer.data() && buffers[i][j]+width+1 <= buffer.data()+buffer.size() );
		}
	}

	// is dithering needed?
	bool dither = false;
	for ( int i = 0 ; i < CHANNELS ; ++i )
		if ( countBits(srcFormat.getChannelMask(i)) >
			countBits(dstFormat.getChannelMask(i)) )
		{
			dither = true;
			break;
		}

	// copy pixel block
	if ( dither )
	{
		// for each row
		for ( int j = 0 ; j < height ; ++j )
		{
			// for each channel
			for ( int k = 0 ; k < CHANNELS ; ++k )
			{
				const uint8_t*	src		= reinterpret_cast<const uint8_t*>( srcData ) + srcPitch * j;
				uint8_t*		dst		= reinterpret_cast<uint8_t*>( dstData ) + dstPitch * j;
				uint32_t**		buf		= buffers[k];

				// channel auxiliary variables
				uint32_t	srcMask		= srcFormat.getChannelMask(k);
				uint32_t	dstMask		= dstFormat.getChannelMask(k);
				uint32_t	srcShift	= getShift( srcMask );
				uint32_t	dstShift	= getShift( dstMask );
				int			srcMax		= srcMask >> srcShift;
				int			dstMax		= dstMask >> dstShift;
				int			srcBits		= countBits( srcMax );
				int			dstBits		= countBits( dstMax );
				uint32_t	errMask		= makeErrorMask( srcBits, dstBits );

				if ( srcBits > 0 && dstBits > 0 )
				{
					// read even rows from left to right
					// and odd rows from right to left (with mirrored filter)
					int i0, i1, idelta;
					if ( j & 1 )
					{
						i0 = width-1;
						i1 = -1;
						idelta = -1;
						src += (width-1)*srcPixelSize;
						dst += (width-1)*dstPixelSize;
					}
					else
					{
						i0 = 0;
						i1 = width;
						idelta = 1;
					}
					for ( int i = i0 ; i != i1 ; i += idelta )
					{
						// read channel value
						uint32_t srcVal = *src;
						switch ( srcPixelSize )
						{
						case 4:	srcVal |= (uint32_t)src[3] << 24;
						case 3:	srcVal |= (uint32_t)src[2] << 16;
						case 2:	srcVal |= (uint32_t)src[1] << 8;
						}

						srcVal &= srcMask;
						srcVal >>= srcShift;
						srcVal <<= 8-srcBits;

						// compute error
						uint32_t dstVal = (srcVal<<4) + buf[0][i];
						dstVal &= (~errMask)<<4;
						uint32_t err = (srcVal<<4) - dstVal;
						if ( (int)err < 0 )
							err = 0;
						err >>= 4;

						// distribute error
						uint32_t err3 = err + err * 2U;
						uint32_t err4 = err * 4U;
						buf[0][i] += (srcVal<<4);
						buf[0][i+idelta] += err3 + err4;
						buf[1][i-idelta] += err3;
						buf[1][i] += err4 + err;
						buf[1][i+idelta] += err;

						src += srcPixelSize*idelta;
					}

					// write current row
					for ( int i = i0 ; i != i1 ; i += idelta )
					{
						// read and convert channel value
						uint32_t srcVal = (buf[0][i]+8) >> 4;
						if ( srcVal > 255 )
							srcVal = 255;
						srcVal >>= 8-dstBits;
						srcVal <<= dstShift;

						// read dst channel value
						uint32_t dstVal = *dst;
						switch ( dstPixelSize )
						{
						case 4:	dstVal |= (uint32_t)dst[3] << 24;
						case 3:	dstVal |= (uint32_t)dst[2] << 16;
						case 2:	dstVal |= (uint32_t)dst[1] << 8;
						}

						// replace dst channel value
						dstVal = (dstVal & ~dstMask) | srcVal;

						// write dst value
						dst[0] = (uint8_t)dstVal;
						switch ( dstPixelSize )
						{
						case 4:	dst[3] = (uint8_t)(dstVal >> 24);
						case 3:	dst[2] = (uint8_t)(dstVal >> 16);
						case 2:	dst[1] = (uint8_t)(dstVal >> 8);
						}

						dst += dstPixelSize*idelta;
					}
				}
				else if ( dstBits > 0 )
				{
					// write current row with constant (default) channel value
					uint32_t srcVal = uint32_t(-1) & dstMask;
					for ( int i = 0 ; i < width ; ++i )
					{
						// read dst channel value
						uint32_t dstVal = *dst;
						switch ( dstPixelSize )
						{
						case 4:	dstVal |= (uint32_t)dst[3] << 24;
						case 3:	dstVal |= (uint32_t)dst[2] << 16;
						case 2:	dstVal |= (uint32_t)dst[1] << 8;
						}

						// replace dst channel value
						dstVal = (dstVal & ~dstMask) | srcVal;

						// write dst value
						dst[0] = (uint8_t)dstVal;
						switch ( dstPixelSize )
						{
						case 4:	dst[3] = (uint8_t)(dstVal >> 24);
						case 3:	dst[2] = (uint8_t)(dstVal >> 16);
						case 2:	dst[1] = (uint8_t)(dstVal >> 8);
						}

						dst += dstPixelSize;
					}
				}

				// cycle current/next row buffers
				uint32_t* tmp = buf[0];
				for ( int i = 0 ; i < BUFFER_ROWS-1 ; ++i )
					buf[i] = buf[i+1];
				buf[BUFFER_ROWS-1] = tmp;
				zero( tmp-BUFFER_EXTRA/2, BUFFER_PITCH );
			} // for each channel
		}
	} 
	else // !dither
	{
		// for each row
		for ( int j = 0 ; j < height ; ++j )
		{
			const uint8_t*	src		= reinterpret_cast<const uint8_t*>( srcData ) + srcPitch * j;
			uint8_t*		dst		= reinterpret_cast<uint8_t*>( dstData ) + dstPitch * j;

			dstFormat.copyPixels( dst, srcFormat, src, width );
		}
	}
}

void SurfaceUtil::blt( 
	const SurfaceFormat& dstFormat, int dstX, int dstY, int dstWidth, int dstHeight, void* dstData, int dstPitch,
	const SurfaceFormat& srcFormat, int srcX, int srcY, int srcWidth, int srcHeight, const void* srcData, int srcPitch )
{
	assert( dstX >= 0 );
	assert( dstY >= 0 );
	assert( dstWidth > 0 );
	assert( srcWidth > 0 );
	assert( dstHeight > 0 );
	assert( srcHeight > 0 );
	assert( dstPitch >= dstWidth*dstFormat.pixelSize() );
	assert( srcPitch >= srcWidth*srcFormat.pixelSize() );

	if ( dstWidth == srcWidth && dstHeight == srcHeight )
	{
		const int		dstPixelSize	= dstFormat.pixelSize();
		const int		srcPixelSize	= srcFormat.pixelSize();
		uint8_t*		dst				= reinterpret_cast<uint8_t*>(dstData) + dstX * dstPixelSize + dstY * dstPitch;
		const uint8_t*	src				= reinterpret_cast<const uint8_t*>(srcData) + srcX * srcPixelSize + srcY * srcPitch;

		blt( dstFormat, dstWidth, dstHeight, dst, dstPitch, srcFormat, src, srcPitch );
	}
	else
	{
		const int			srcPixelSize	= srcFormat.pixelSize();
		const int			bufPitch		= dstWidth * srcPixelSize;
		TempArray<uint8_t>	buf				( dstHeight * bufPitch );
		const int			srcDX16			= (int)( (float)srcWidth * 65536.f / (float)dstWidth );
		const int			srcDY16			= (int)( (float)srcHeight * 65536.f / (float)dstHeight );
		int					srcY16			= srcY << 16;

		for ( int j = 0 ; j < dstHeight ; ++j )
		{
			uint8_t*		dst				= reinterpret_cast<uint8_t*>( buf.data() ) + j * bufPitch;
			const uint8_t*	src				= reinterpret_cast<const uint8_t*>( srcData ) + (srcY16>>16) * srcPitch;
			int				srcX16			= srcX << 16;
			
			for ( int i = 0 ; i < dstWidth ; ++i )
			{
				int srcX = (srcX16>>16) * srcPixelSize;
				for ( int k = 0 ; k < srcPixelSize ; ++k )
					*dst++ = src[srcX++];
				srcX16 += srcDX16;
			}

			srcY16 += srcDY16;
		}

		const int	dstPixelSize	= dstFormat.pixelSize();
		uint8_t*	dst				= reinterpret_cast<uint8_t*>(dstData) + dstX * dstPixelSize + dstY * dstPitch;

		blt( dstFormat, dstWidth, dstHeight, dst, dstPitch, srcFormat, buf.data(), bufPitch );
	}
}

long SurfaceUtil::getPixel( int x, int y, 
	int width, int height, 
	const void* data, int pitch, 
	const SurfaceFormat& format )
{
	assert( x >= 0 && x < width ); width;
	assert( y >= 0 && y < height ); height;
	assert( format.bltSupported() || format.compressed() );

	long argb = 0;
	if ( format.compressed() )
	{
		switch ( format.type() )
		{
		case SurfaceFormat::SURFACE_DXT1:	argb = getPixelDXT1( x, y, data, pitch ); break;
		case SurfaceFormat::SURFACE_DXT3:	argb = getPixelDXT3( x, y, data, pitch ); break;
		case SurfaceFormat::SURFACE_DXT5:	argb = getPixelDXT5( x, y, data, pitch ); break;
		default:							assert( false );
		}
	}
	else
	{
		const uint8_t* pixeldata = reinterpret_cast<const uint8_t*>( data );
		long i = x * format.pixelSize() + y * pitch;

		SurfaceFormat fmtARGB8888 = SurfaceFormat::SURFACE_A8R8G8B8;
		uint8_t buff[4];
		fmtARGB8888.copyPixels( buff, format, pixeldata+i, 1 );
		
		argb = buff[0];
		argb += uint32_t(buff[1])<<8;
		argb += uint32_t(buff[2])<<16;
		argb += uint32_t(buff[3])<<24;
	}
	return argb;
}


} // pix
