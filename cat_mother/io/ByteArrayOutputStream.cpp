#include "ByteArrayOutputStream.h"
#include "IOException.h"
#include <assert.h>
#include <stdint.h>
#include <memory.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace io
{


/**
 * Reserves memory for the buffer. Uses pre-allocated buffer if possible.
 */
static void reserveBuffer( 
	long	newCapacity,
	char*&	data,
	long&	size,
	long&	capacity,
	char*	defaultBuffer )
{
	if ( newCapacity > capacity )
	{
		long actualCapacity = (newCapacity+255) & ~255;
		if ( actualCapacity < 256 )
			actualCapacity = 256;

		char* buffer = new char[ actualCapacity ];
		if ( size > 0 )
			memcpy( buffer, data, size );
		if ( actualCapacity > size ) 
			memset( buffer+size, 0, actualCapacity-size );
		if ( defaultBuffer != data )
			delete[] data;

		data = buffer;
		capacity = actualCapacity;
	}
}

/**
 * Frees any dynamically allocated buffer and activates the default buffer.
 */
static void freeBuffer( char*& data, long& size, long& capacity, char* defaultBuffer, long defaultBufferCapacity )
{
	if ( data != defaultBuffer )
		delete[] data;

	data = defaultBuffer;
	size = 0;
	capacity = defaultBufferCapacity;
}

//-----------------------------------------------------------------------------

ByteArrayOutputStream::ByteArrayOutputStream( long size )
{
	m_data		= m_defaultBuffer;
	m_size		= 0;
	m_capacity	= sizeof(m_defaultBuffer);

	reserveBuffer( size, m_data, m_size, m_capacity, m_defaultBuffer );
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
	freeBuffer( m_data, m_size, m_capacity, m_defaultBuffer, sizeof(m_defaultBuffer) );
}

void ByteArrayOutputStream::reset()
{
	m_size = 0;
}

void ByteArrayOutputStream::write( const void* data, long size )
{
	const long		newSize	= m_size+size;

	if ( newSize > m_capacity )
		reserveBuffer( newSize, m_data, m_size, m_capacity, m_defaultBuffer );

	const uint8_t*	src		= reinterpret_cast<const uint8_t*>( data );
	uint8_t*		dest	= reinterpret_cast<uint8_t*>( m_data ) + m_size;

	for ( long i = 0 ; i < size ; ++i )
		dest[i] = src[i];

	m_size = newSize;
}

void ByteArrayOutputStream::toByteArray( void* data, long size ) const
{
	assert( size >= this->size() );

	const uint8_t*	src		= reinterpret_cast<const uint8_t*>( m_data );
	uint8_t*		dest	= reinterpret_cast<uint8_t*>( data );
	long			count	= this->size();

	if ( count > size )
		count = size;

	for ( long i = 0 ; i < count ; ++i )
		dest[i] = src[i];
}

long ByteArrayOutputStream::size() const
{
	return m_size;
}

lang::String ByteArrayOutputStream::toString() const
{
	return "ByteArrayInputStream";
}


} // io
