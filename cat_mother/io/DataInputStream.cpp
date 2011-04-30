#include "DataInputStream.h"
#include "EOFException.h"
#include <lang/UTFConverter.h>
#include <lang/UTF16.h>
#include <assert.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

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

static void resizeBuffer( int newSize, void*& buffer, int& bufferSize )
{
	if ( bufferSize < newSize || !buffer )
	{
		if ( buffer )
		{
			delete[] reinterpret_cast<uint8_t*>(buffer);
			buffer = 0;
		}
		
		bufferSize = newSize*2;
		buffer = new uint8_t[ bufferSize ];
	}
}

static void freeBuffer( void*& buffer )
{
	if ( buffer )
	{
		delete[] reinterpret_cast<uint8_t*>(buffer);
		buffer = 0;
	}
}

//-----------------------------------------------------------------------------

DataInputStream::DataInputStream( InputStream* in ) :
	FilterInputStream(in)
{
	m_inBuffer		= 0;
	m_inBufferSize	= 0;
	m_size			= 0;
}

DataInputStream::~DataInputStream()
{
	if ( m_inBuffer )
		freeBuffer( m_inBuffer );
}

long DataInputStream::skip( long n )
{
	long skipped = FilterInputStream::skip( n );
	m_size += skipped;
	return skipped;
}

long DataInputStream::read( void* data, long size )
{
	long bytesRead = FilterInputStream::read( data, size );
	m_size += bytesRead;
	return bytesRead;
}

void DataInputStream::readFully( void* data, long size )
{
	long bytesRead = FilterInputStream::read( data, size );
	m_size += bytesRead;

	if ( bytesRead != size )
		throw EOFException( Format("Unexpected end of file in {0}.",toString()) );
}

bool DataInputStream::readBoolean()
{
	uint8_t v;
	readFully( &v, sizeof(v) );
	return v != 0;
}

uint8_t DataInputStream::readByte()
{
	int8_t v;
	readFully( &v, sizeof(v) );
	return v;
}

Char DataInputStream::readChar()
{
	uint8_t bytes[2];
	readFully( bytes, sizeof(bytes) );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(bytes) );

	uint16_t v = *reinterpret_cast<uint16_t*>(bytes);
	return (Char)v;
}

String DataInputStream::readChars( int n )
{
	const int		bufferSize = 256;
	Char			buffer[bufferSize];
	Char*			buff = buffer;
	String			str;

	for ( int i = 0 ; i < n ; ++i )
	{
		*buff++ = readChar();
		
		if ( i+1 >= n || buff-buffer > bufferSize-8 )
		{
			*buff = Char(0);
			str = str + buffer;
		}
	}

	return str;
}

double DataInputStream::readDouble()
{
	uint8_t bytes[ sizeof(double) ];
	readFully( bytes, sizeof(bytes) );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(bytes) );

	double v = *reinterpret_cast<double*>(bytes);
	return v;
}

float DataInputStream::readFloat()
{
	uint8_t bytes[ sizeof(float) ];
	readFully( bytes, sizeof(bytes) );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(bytes) );

	float v = *reinterpret_cast<float*>(bytes);
	return v;
}

int DataInputStream::readInt()
{
	uint8_t bytes[ sizeof(int32_t) ];
	readFully( bytes, sizeof(bytes) );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(bytes) );

	int32_t v = *reinterpret_cast<int32_t*>(bytes);
	return (int)v;
}

long DataInputStream::readLong()
{
	uint8_t bytes[ sizeof(int64_t) ];
	readFully( bytes, sizeof(bytes) );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(bytes) );

	int64_t v = *reinterpret_cast<int64_t*>(bytes);
	return (long)v;
}

int DataInputStream::readShort()
{
	uint8_t bytes[ sizeof(int16_t) ];
	readFully( bytes, sizeof(bytes) );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(bytes) );

	int16_t v = *reinterpret_cast<int16_t*>(bytes);
	return (int)v;
}

String DataInputStream::readUTF()
{
	UTFConverter	cnv( "UTF-8" );
	const int		outBufferSize = 256;
	Char			outBuffer[outBufferSize];
	Char*			outBuff = outBuffer;
	String			str;

	// UTF-8 data size (bytes)
	int encodedBytes = readShort();
	if ( encodedBytes <= 0 )
		throw IOException( Format("Invalid UTF-8 data in {0}.",toString()) );

	// data
	resizeBuffer( encodedBytes, m_inBuffer, m_inBufferSize );
	uint8_t* inBuffer = reinterpret_cast<uint8_t*>(m_inBuffer);
	readFully( inBuffer, encodedBytes );

	// decode
	int i = 0;
	while ( i < encodedBytes )
	{
		int bytes = 0;
		Char32 cp;
		if ( cnv.decode( inBuffer+i, inBuffer+encodedBytes, &bytes, &cp ) )
		{
			outBuff = UTF16::append( outBuff, cp );
		}
		i += bytes;

		if ( i >= encodedBytes || 
			outBuff-outBuffer > outBufferSize-8 )
		{
			*outBuff = Char(0);
			str = str + outBuffer;
		}
	}

	assert( 0 == encodedBytes-i );
	return str;
}

long DataInputStream::size() const
{
	return m_size;
}


} // io
