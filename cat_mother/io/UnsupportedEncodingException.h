#ifndef _IO_UNSUPPORTEDENCODINGEXCEPTION_H
#define _IO_UNSUPPORTEDENCODINGEXCEPTION_H


#include <io/IOException.h>


namespace io
{


/**
 * Thrown if specified character encoding is not supported.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class UnsupportedEncodingException :
	public IOException
{
public:
	///
	UnsupportedEncodingException( const lang::Format& msg  )						: IOException(msg) {}
};


} // io


#endif // _IO_UNSUPPORTEDENCODINGEXCEPTION_H

