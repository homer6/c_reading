#ifndef _IO_IOEXCEPTION_H
#define _IO_IOEXCEPTION_H


#include <lang/Exception.h>


namespace io
{


/**
 * Thrown if input/output operation fails.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class IOException :
	public lang::Exception
{
public:
	///
	IOException( const lang::Format& msg )											: Exception(msg) {}
};


} // io


#endif // _IO_IOEXCEPTION_H

