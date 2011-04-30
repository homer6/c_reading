#include "ByteArrayInputStream.h"
#include <stdint.h>
#include <memory.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace io
{


ByteArrayInputStream::ByteArrayInputStream( const void* data, long size )
{
	m_data = 0;
	m_size = size;
	m_index = 0;

	if ( m_size < sizeof(m_defaultBuffer) )
		m_data = m_defaultBuffer;
	else
		m_data = new char[ size ];

	memcpy( m_data, data, size );
}

ByteArrayInputStream::~ByteArrayInputStream()
{
	if ( m_size > sizeof(m_defaultBuffer) )
		delete[] m_data;
	m_data = 0;
}

long ByteArrayInputStream::read( void* data, long size )
{
	long left = available();
	long count = size;
	if ( left < count )
		count = left;

	const uint8_t*	src		= reinterpret_cast<const uint8_t*>( m_data ) + m_index;
	uint8_t*		dest	= reinterpret_cast<uint8_t*>( data );

	for ( long i = 0 ; i < count ; ++i )
		dest[i] = src[i];

	m_index += count;
	return count;
}

long ByteArrayInputStream::available() const
{
	return m_size - m_index;
}

lang::String ByteArrayInputStream::toString() const
{
	return "ByteArrayInputStream";
}


} // io
