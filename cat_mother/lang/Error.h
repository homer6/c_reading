#ifndef _LANG_ERROR_H
#define _LANG_ERROR_H


#include <lang/Throwable.h>


namespace lang
{


/**
 * Base class for all errors that indicate serious problem in an application.
 * These should be handled only by informing the user and terminating application.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Error :
	public Throwable
{
public:
	/** Creates an error with no error description. */
	Error() {}

	/** Creates an error with the specified description. */
	Error( const lang::Format& msg )												: Throwable(msg) {}
};


} // lang


#endif // _LANG_ERROR_H

