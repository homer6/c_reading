#include "FilterOutputStream.h"
#include "config.h"

//-----------------------------------------------------------------------------

namespace io
{


FilterOutputStream::FilterOutputStream( OutputStream* target ) :
	m_target(target)
{
}

void FilterOutputStream::close()																	
{
	m_target->close();
}

void FilterOutputStream::flush()																	
{
	m_target->flush();
}

void FilterOutputStream::write( const void* data, long size )									
{
	m_target->write(data,size);
}

lang::String FilterOutputStream::toString() const
{
	return m_target->toString();
}


} // io

