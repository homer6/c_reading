#include "MemoryInputStream.h"
#include <memory.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace util
{


MemoryInputStream::MemoryInputStream( Object* dataOwner, const void* data, long size, const String& name )
{
	m_dataOwner = dataOwner;
	m_data		= data;
	m_size		= size;
	m_ptr		= 0;
	m_mark		= 0;
	m_name		= name;
}

long MemoryInputStream::read( void* data, long size )
{
	long left = m_size - m_ptr;
	if ( size > left )
		size = left;

	memcpy( data, reinterpret_cast<const uint8_t*>(m_data)+m_ptr, size );
	m_ptr += size;
	return size;
}

void MemoryInputStream::mark( int /*readlimit*/ )
{
	m_mark = m_ptr;
}

void MemoryInputStream::reset()
{
	m_ptr = m_mark;
}

bool MemoryInputStream::markSupported() const
{
	return true;
}

long MemoryInputStream::available() const
{
	return m_size - m_ptr;
}

String MemoryInputStream::toString() const
{
	return m_name;
}


} // util
