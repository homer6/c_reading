#ifndef _LANG_THROWABLE_H
#define _LANG_THROWABLE_H


#include <lang/Format.h>


namespace lang
{


/**
 * Base class for all errors and exceptions.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Throwable
{
public:
	/** Creates throwable object with no error description. */
	Throwable();

	/** Creates throwable object with the specified error description. */
	Throwable( const Format& msg );

	/** Sets the error message. */
	void				setMessage( const Format& msg );

	/** Returns the error message string. */
	const Format&		getMessage() const;

	/** Returns formatted stack trace (if available) of the exception source. */
	const String&		getStackTrace() const;

private:
	Format	m_msg;
	String	m_trace;
};


} // lang


#endif // _LANG_THROWABLE_H

