#ifndef _LANG_NUMBERFORMATEXCEPTION_H
#define _LANG_NUMBERFORMATEXCEPTION_H


#include <lang/Exception.h>


namespace lang
{


/**
 * Thrown if the application attempts to 
 * convert a string to one of the numeric types, 
 * but the string does not have the correct format.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class NumberFormatException :
	public Exception
{
public:
	///
	NumberFormatException( const lang::Format& msg )								: Exception(msg) {}
};


} // lang


#endif // _LANG_NUMBERFORMATEXCEPTION_H

