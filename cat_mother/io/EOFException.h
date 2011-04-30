#ifndef _IO_EOFEXCEPTION_H
#define _IO_EOFEXCEPTION_H


#include <io/IOException.h>


namespace io
{


/**
 * Thrown if an end of file or end of stream has been reached 
 * unexpectedly during input.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class EOFException :
	public IOException
{
public:
	///
	EOFException( const lang::Format& msg )											: IOException(msg) {}
};


} // io


#endif // _IO_EOFEXCEPTION_H

