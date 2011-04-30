#include "Throwable.h"
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


Throwable::Throwable()
{
	setMessage( Format("") );
}

Throwable::Throwable( const lang::Format& msg )
{
	setMessage( msg );
}

void Throwable::setMessage( const lang::Format& msg )
{
	m_msg = msg;
}

const lang::Format& Throwable::getMessage() const
{
	return m_msg;
}

const String& Throwable::getStackTrace() const
{
	return m_trace;
}


} // lang

