#include "FilterInputStream.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace io
{


FilterInputStream::FilterInputStream( InputStream* source ) : 
	m_source(source) 
{
}

void FilterInputStream::close()																	
{
	m_source->close();
}

long FilterInputStream::read( void* data, long size )											
{
	return m_source->read(data,size);
}

long FilterInputStream::skip( long n )															
{
	assert( n >= 0 );
	return m_source->skip(n);
}

long FilterInputStream::available() const														
{
	return m_source->available();
}

lang::String FilterInputStream::toString() const
{
	return m_source->toString();
}


} // io

