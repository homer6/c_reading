#ifndef _LANG_DEBUG_H
#define _LANG_DEBUG_H


#include <lang/Formattable.h>


namespace lang
{


/** 
 * Debug output. Thread-safe.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Debug
{
public:
	/** Outputs an informative message. */
	static void		println( const lang::String& str );
	static void		println( const lang::String& str, const lang::Formattable& arg0 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7, const lang::Formattable& arg8 );
	static void		println( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7, const lang::Formattable& arg8, const lang::Formattable& arg9 );

	/** Outputs a warning message. */
	static void		printlnWarning( const lang::String& str );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7, const lang::Formattable& arg8 );
	static void		printlnWarning( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7, const lang::Formattable& arg8, const lang::Formattable& arg9 );

	/** Outputs an error message. */
	static void		printlnError( const lang::String& str );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7, const lang::Formattable& arg8 );
	static void		printlnError( const lang::String& str, const lang::Formattable& arg0, const lang::Formattable& arg1, const lang::Formattable& arg2, const lang::Formattable& arg3, const lang::Formattable& arg4, const lang::Formattable& arg5, const lang::Formattable& arg6, const lang::Formattable& arg7, const lang::Formattable& arg8, const lang::Formattable& arg9 );
};


} // lang


#endif // _LANG_DEBUG_H
