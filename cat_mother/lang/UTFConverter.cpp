#include "UTFConverter.h"
#include <io/UnsupportedEncodingException.h>
#include <string.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


enum EncodingType
{
	ENCODING_UNKNOWN,
	ENCODING_ASCII7,
	ENCODING_UTF8,
	ENCODING_UTF16BE,
	ENCODING_UTF16LE,
	ENCODING_UTF32BE,
	ENCODING_UTF32LE
};

//-----------------------------------------------------------------------------

static bool decode_ASCII7( const uint8_t* src, int srcSize, int* srcBytes, Char32* buffer )
{
	const uint8_t*	src0	= src;
	int				err		= 0;
	Char32			cp		= Char32(-1);

	if ( srcSize >= 1 )
	{
		cp = *src++;
		//if ( cp >= (uint8_t)128 )
		//	err = 1;				// ERROR: Out-of-range ASCII-7 code
	}
	else
	{
		// ERROR: Not enough encoded bytes available
		err = 4;
	}

	if ( !err )
		*buffer = cp;
	*srcBytes = src-src0;
	return !err;
}

static bool decode_UTF8( const uint8_t* src, int srcSize, int* srcBytes, Char32* buffer )
{
	const uint8_t*	src0	= src;
	int				err		= 0;
	Char32			cp		= Char32(-1);

	if ( srcSize >= 1 )
	{
		uint8_t first = *src++;
		if ( 0 == (first & 0x80) )
		{
			// ok, single byte ASCII-7 (US-ASCII) code
			cp = first;
		}
		else
		{
			// multibyte character code

			// read remaining byte count and 
			// most signifigant bits of the character code
			int bytes = 1;
			uint8_t ByteCountMask = 0x40;
			Char32 codeMask = 0x3F;
			while ( first & ByteCountMask )
			{
				++bytes;
				ByteCountMask >>= 1;
				codeMask >>= 1;
			}
			if ( bytes < 2 || bytes > 4 )
			{
				// ERROR: Invalid number of following bytes
				err = 1;
			}
			else
			{
				if ( srcSize < bytes )
				{
					// ERROR: Not enough encoded bytes available
					err = 4;
				}
				else
				{
					// read remaining bytes of the character code
					cp = first & codeMask;
					for ( int i = 1 ; i < bytes ; ++i )
					{
						cp <<= 6;
						cp |= ( 0x3F & (Char32)*src++ );
					}
				}
			}
		}
	}
	else
	{
		// ERROR: Not enough encoded bytes available
		err = 4;
	}

	if ( !err )
		*buffer = cp;
	*srcBytes = src-src0;
	return !err;
}

static bool decode_UTF16( const uint8_t* src, int srcSize, int* srcBytes, Char32* buffer, bool bigEndian )
{
	const uint8_t*	src0	= src;
	int				err		= 0;
	Char32			cp		= Char32(-1);

	if ( srcSize >= 2 )
	{
		if ( bigEndian )
			cp = (Char32(src[0])<<8) + Char32(src[1]);
		else
			cp = Char32(src[0]) + (Char32(src[1])<<8);
		src += 2;

		if ( 0xD800 == (cp&0xFFFFFC00) )
		{
			if ( srcSize >= 4 )
			{
				Char32 ch2;
				if ( bigEndian )
					ch2 = (Char32(src[0])<<8) + Char32(src[1]);
				else
					ch2 = Char32(src[0]) + (Char32(src[1])<<8);
				src += 2;

				cp = (cp<<10) + ch2 - ((0xd800<<10UL)+0xdc00-0x10000);
			}
			else
			{
				// ERROR: Not enough encoded bytes available
				err = 4;
			}
		}
	}
	else
	{
		// ERROR: Not enough encoded bytes available
		err = 4;
	}

	if ( !err )
		*buffer = cp;
	*srcBytes = src-src0;
	return !err;
}

static bool decode_UTF32( const uint8_t* src, int srcSize, int* srcBytes, Char32* buffer, bool bigEndian )
{
	const uint8_t*	src0	= src;
	int				err		= 0;
	Char32			cp		= Char32(-1);

	if ( srcSize >= 4 )
	{
		if ( bigEndian )
		{
			cp = 0;
			for ( int i = 0 ; i < 4 ; ++i )
			{
				cp <<= 8;
				cp += src[i];
			}
		}
		else // little endian
		{
			cp = 0;
			for ( int i = 4 ; i > 0 ; )
			{
				--i;
				cp <<= 8;
				cp += src[i];
			}
		}
		src += 4;
	}
	else
	{
		// ERROR: Not enough encoded bytes available
		err = 4;
	}

	if ( !err )
		*buffer = cp;
	*srcBytes = src-src0;
	return !err;
}

static bool encode_ASCII7( uint8_t* dst, int dstSize, int* dstBytes, Char32 cp )
{
	const uint8_t*	dst0	= dst;
	int				err		= 0;

	if ( dstSize >= 1 )
	{
		if ( cp >= 128 )
		{
			// ERROR: Out-of-range ASCII-7 code
			err = 1;
		}
		else
		{
			*dst++ = (uint8_t)cp;
		}
	}
	else
	{
		// ERROR: Not enough buffer space
		err = 5;
		cp = Char32(-1);
	}

	*dstBytes = dst-dst0;
	return !err;
}

static bool encode_UTF8( uint8_t* dst, int dstSize, int* dstBytes, Char32 cp )
{
	const uint8_t*	dst0	= dst;
	int				err		= 0;

	if (cp < 0x80) 
	{
		if ( dstSize < 1 )
		{
			// ERROR: Not enough buffer space.
			err = 5;
		}
		else
		{
			*dst++ = (uint8_t)cp;
		}
	}
	else if (cp < 0x800) 
	{
		if ( dstSize < 2 )
		{
			// ERROR: Not enough buffer space.
			err = 5;
		}
		else
		{
			*dst++ = (uint8_t)( 0xC0 | (cp>>6) );
			*dst++ = (uint8_t)( 0x80 | (cp&0x3F) );
		}
	}
	else if (cp < 0x10000) 
	{
		if ( dstSize < 3 )
		{
			// ERROR: Not enough buffer space.
			err = 5;
		}
		else
		{
			*dst++ = (uint8_t)( 0xE0 | (cp>>12) );
			*dst++ = (uint8_t)( 0x80 | ( (cp>>6) &0x3F) );
			*dst++ = (uint8_t)( 0x80 | (cp&0x3F) );
		}
	}
	else if (cp < 0x200000) 
	{
		if ( dstSize < 4 )
		{
			// ERROR: Not enough buffer space.
			err = 5;
		}
		else
		{
			*dst++ = (uint8_t)( 0xF0 | (cp>>18) );
			*dst++ = (uint8_t)( 0x80 | ( (cp>>12) &0x3F) );
			*dst++ = (uint8_t)( 0x80 | ( (cp>>6) &0x3F) );
			*dst++ = (uint8_t)( 0x80 | (cp&0x3F) );
		}
	}
	else
	{
		// ERROR: Invalid Unicode scalar value
		err = 2;
	}

	*dstBytes = dst-dst0;
	return !err;
}

static bool encode_UTF16( uint8_t* dst, int dstSize, int* dstBytes, Char32 cp, bool bigEndian )
{
	const uint8_t*	dst0	= dst;
	int				err		= 0;

	// encode
	Char codes[2];
	int codeCount = 0;
	if ( cp >= 0x10000 )
	{
		codes[codeCount++] = Char( ((cp-0x10000)>>10) + 0xD800 );
		codes[codeCount++] = Char( ((cp-0x10000)&1023) + 0xDC00 );
	}
	else
	{
		codes[codeCount++] = Char( cp );
	}

	// write
	int codeSize = unsigned(codeCount) * 2U;
	if ( dstSize < codeSize )
	{
		// Error: Not enough buffer space
		err = 5;
	}
	else
	{
		for ( int i = 0 ; i < codeCount ; ++i )
		{
			Char code = codes[i];
			if ( bigEndian )
			{
				*dst++ = uint8_t(code >> 8);
				*dst++ = uint8_t(code);
			}
			else
			{
				*dst++ = uint8_t(code);
				*dst++ = uint8_t(code >> 8);
			}
		}
	}
	
	*dstBytes = dst-dst0;
	return !err;
}

static bool encode_UTF32( uint8_t* dst, int dstSize, int* dstBytes, Char32 cp, bool bigEndian )
{
	const uint8_t*	dst0	= dst;
	int				err		= 0;

	// write
	int codeCount = 1;
	int codeSize = unsigned(codeCount) * 4U;
	if ( dstSize < codeSize )
	{
		// Error: Not enough buffer space
		err = 5;
	}
	else
	{
		Char32 code = cp;
		if ( bigEndian )
		{
			*dst++ = uint8_t(code >> 24);
			*dst++ = uint8_t(code >> 16);
			*dst++ = uint8_t(code >> 8);
			*dst++ = uint8_t(code);
		}
		else
		{
			*dst++ = uint8_t(code);
			*dst++ = uint8_t(code >> 8);
			*dst++ = uint8_t(code >> 16);
			*dst++ = uint8_t(code >> 24);
		}
	}
	
	*dstBytes = dst-dst0;
	return !err;
}

static EncodingType getEncodingType( const char* encoding )
{
	EncodingType type = ENCODING_UNKNOWN;

	if ( !strcmp("ASCII-7",encoding) )		
	{
		type = ENCODING_ASCII7;
	}
	else if ( !strcmp("UTF-8",encoding) )	
	{
		type = ENCODING_UTF8;
	}
	else if ( !strcmp("UTF-16BE",encoding) )
	{
		type = ENCODING_UTF16BE;
	}
	else if ( !strcmp("UTF-16LE",encoding) )
	{
		type = ENCODING_UTF16LE;
	}

	return type;
}

//-----------------------------------------------------------------------------

UTFConverter::UTFConverter( const char* encoding )
{
	m_type = getEncodingType( encoding );
	
	if ( ENCODING_UNKNOWN == m_type )
		throw io::UnsupportedEncodingException( Format("Unsupported encoding: {0}",encoding) );
}

bool UTFConverter::decode( const void* src, const void* srcEnd, int* srcBytes, Char32* dst ) const
{
	const uint8_t*	bsrc		= reinterpret_cast<const uint8_t*>( src );
	const uint8_t*	bsrcEnd		= reinterpret_cast<const uint8_t*>( srcEnd );
	const int		srcSize		= bsrcEnd - bsrc;

	switch ( EncodingType(m_type) )
	{
	case ENCODING_ASCII7:	return decode_ASCII7( bsrc, srcSize, srcBytes, dst );
	case ENCODING_UTF8:		return decode_UTF8	( bsrc, srcSize, srcBytes, dst );
	case ENCODING_UTF16BE:	return decode_UTF16	( bsrc, srcSize, srcBytes, dst, true );
	case ENCODING_UTF16LE:	return decode_UTF16	( bsrc, srcSize, srcBytes, dst, false );
	case ENCODING_UTF32BE:	return decode_UTF32	( bsrc, srcSize, srcBytes, dst, true );
	case ENCODING_UTF32LE:	return decode_UTF32	( bsrc, srcSize, srcBytes, dst, false );
	}
	return false;
}

bool UTFConverter::encode( void* dst, void* dstEnd, int* dstBytes, Char32 src )
{
	uint8_t*	bdst	= reinterpret_cast<uint8_t*>( dst );
	uint8_t*	bdstEnd	= reinterpret_cast<uint8_t*>( dstEnd );
	int			dstSize	= bdstEnd - bdst;

	switch ( EncodingType(m_type) )
	{
	case ENCODING_ASCII7:	return encode_ASCII7( bdst, dstSize, dstBytes, src );
	case ENCODING_UTF8:		return encode_UTF8	( bdst, dstSize, dstBytes, src );
	case ENCODING_UTF16BE:	return encode_UTF16	( bdst, dstSize, dstBytes, src, true );
	case ENCODING_UTF16LE:	return encode_UTF16	( bdst, dstSize, dstBytes, src, false );
	case ENCODING_UTF32BE:	return encode_UTF32	( bdst, dstSize, dstBytes, src, true );
	case ENCODING_UTF32LE:	return encode_UTF32	( bdst, dstSize, dstBytes, src, false );
	}
	return false;
}


} // lang

