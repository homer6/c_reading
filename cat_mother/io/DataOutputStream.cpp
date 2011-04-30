#include "DataOutputStream.h"
#include <lang/UTFConverter.h>
#include <lang/UTF16.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace io
{


static bool bigEndian()
{
	uint32_t x = 1;
	return 0 == *reinterpret_cast<uint8_t*>(&x);
}

static void reverse( uint8_t* begin, uint8_t* end )
{
	for ( ; begin != end && begin != --end ; ++begin )
	{
		uint8_t b = *begin;
		*begin = *end;
		*end = b;
	}
}

//-----------------------------------------------------------------------------

DataOutputStream::DataOutputStream( OutputStream* out )	: 
	FilterOutputStream(out) 
{
	m_size	= 0;
}

void DataOutputStream::writeBoolean( bool value )
{
	uint8_t v = uint8_t(value ? 1 : 0);
	write( &v, sizeof(v) );
}

void DataOutputStream::writeByte( int value )
{
	int8_t v = (uint8_t)value;
	write( &v, sizeof(v) );
}

void DataOutputStream::writeChar( lang::Char value )
{
	uint16_t v = (uint16_t)value;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );

	write( bytes, sizeof(v) );
}

void DataOutputStream::writeChars( const lang::String& value )
{
	int len = value.length();
	for ( int i = 0 ; i < len ; ++i )
	{
		lang::Char ch = value.charAt(i);
		writeChar( ch );
	}
}

void DataOutputStream::writeDouble( double value )
{
	double v = value;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );

	write( bytes, sizeof(v) );
}

void DataOutputStream::writeFloat( float value )
{
	float v = value;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );

	write( bytes, sizeof(v) );
}

void DataOutputStream::writeInt( int value )
{
	int32_t v = value;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );

	write( bytes, sizeof(v) );
}

void DataOutputStream::writeLong( long value )
{
	int64_t v = value;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );

	write( bytes, sizeof(v) );
}

void DataOutputStream::writeShort( int value )
{
	int16_t v = (int16_t)value;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );

	write( bytes, sizeof(v) );
}

void DataOutputStream::writeUTF( const lang::String& value )
{
	lang::UTFConverter	cnv( "UTF-8" );
	const int			bufferSize = 256;
	uint8_t				buffer[bufferSize];
	uint8_t*			buff = buffer;
	const int			len	= value.length();

	// UTF-8 data size (bytes)
	int encodedBytes = 0;
	int i;
	for ( i = 0 ; i < len ; ++i )
	{
		lang::Char32 cp = lang::Char32(-1);

		lang::Char ch1 = value.charAt(i);
		if ( lang::UTF16::isFirstSurrogate(ch1) )
		{
			if ( i+1 < len )
			{
				lang::Char ch2 = value.charAt(i+1);

				if ( lang::UTF16::isSecondSurrogate(ch2) )
				{
					cp = lang::UTF16::makeCodePoint( ch1, ch2 );
					++i;
				}
			}
		}
		else
		{
			cp = ch1;
		}

		if ( cp != lang::Char32(-1) )
		{
			int bytes;
			if ( cnv.encode( buffer, buffer+sizeof(buffer), &bytes, cp ) )
				encodedBytes += bytes;
		}
	}
	writeShort( encodedBytes );

	// data
	for ( i = 0 ; i < len ; ++i )
	{
		lang::Char32 cp = lang::Char32(-1);

		lang::Char ch1 = value.charAt(i);
		if ( lang::UTF16::isFirstSurrogate(ch1) )
		{
			if ( i+1 < len )
			{
				lang::Char ch2 = value.charAt(i+1);

				if ( lang::UTF16::isSecondSurrogate(ch2) )
				{
					cp = lang::UTF16::makeCodePoint( ch1, ch2 );
					++i;
				}
			}
		}
		else
		{
			cp = ch1;
		}

		if ( cp != lang::Char32(-1) )
		{
			int bytes;
			if ( cnv.encode( buff, buffer+sizeof(buffer), &bytes, cp ) )
				buff += bytes;
		}

		if ( i+1 >= len || buff-buffer > bufferSize-8 )
		{
			int bytes = buff-buffer;
			write( buffer, bytes );
			buff = buffer;
			encodedBytes -= bytes;
		}
	}

	assert( 0 == encodedBytes );
}

long DataOutputStream::size() const
{
	return m_size;
}


} // io

