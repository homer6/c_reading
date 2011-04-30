#ifndef _LANG_EXCEPTION_H
#define _LANG_EXCEPTION_H


#include <lang/Throwable.h>


namespace lang
{


/**
 * Base class for all exceptions that an application might want to handle.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Exception :
	public Throwable
{
public:
	/** Creates an exception with no error description. */
	Exception() {}

	/** Creates an exception with the specified error description. */
	Exception( const lang::Format& msg )											: Throwable(msg) {}
};


} // lang


#endif // _LANG_EXCEPTION_H

