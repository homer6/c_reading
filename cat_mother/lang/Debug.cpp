#include "Debug.h"
#include <lang/Format.h>
#include <stdio.h>
#include <string.h>

#ifdef LANG_WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#include "config.h"

//-----------------------------------------------------------------------------

#define MAX_MSG_LEN 1000

//-----------------------------------------------------------------------------

namespace lang
{


static void dprintln( int level, const Format& msg )
{
	// message type tag: {ERROR, WARNING, INFO}
	char str[MAX_MSG_LEN+16];
	if ( level <= 1 )
		strcpy( str, "ERROR: " );
	else if ( level <= 2 )
		strcpy( str, "WARNING: " );
	else
		strcpy( str, "INFO: " );
	int typeLen = strlen(str);

	// format msg to ASCII-7 string and append newline
	int end = typeLen + msg.format().getBytes( str+typeLen, MAX_MSG_LEN, "ASCII-7" );
	if ( end > MAX_MSG_LEN )
		end = MAX_MSG_LEN;
	str[end+0] = '\n';
	str[end+1] = 0;

	// output to stdout / debug window
	#ifdef LANG_WIN32
	OutputDebugString( str );
	#else
	fputs( str, stdout );
	#endif
}

//-----------------------------------------------------------------------------

void Debug::printlnError( const String& str )
{
	dprintln( 1, Format(str) );
}

void Debug::printlnError( const String& str, const Formattable& arg0 )
{
	dprintln( 1, Format(str,arg0) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1 )
{
	dprintln( 1, Format(str,arg0,arg1) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2,arg3) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2,arg3,arg4) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2,arg3,arg4,arg5) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) );
}

void Debug::printlnError( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8, const Formattable& arg9 )
{
	dprintln( 1, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) );
}

void Debug::printlnWarning( const String& str )
{
	dprintln( 2, Format(str) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0 )
{
	dprintln( 2, Format(str,arg0) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1 )
{
	dprintln( 2, Format(str,arg0,arg1) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2,arg3) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2,arg3,arg4) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2,arg3,arg4,arg5) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) );
}

void Debug::printlnWarning( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8, const Formattable& arg9 )
{
	dprintln( 2, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) );
}

void Debug::println( const String& str )
{
	dprintln( 3, Format(str) );
}

void Debug::println( const String& str, const Formattable& arg0 )
{
	dprintln( 3, Format(str,arg0) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1 )
{
	dprintln( 3, Format(str,arg0,arg1) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2,arg3) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2,arg3,arg4) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2,arg3,arg4,arg5) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) );
}

void Debug::println( const String& str, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8, const Formattable& arg9 )
{
	dprintln( 3, Format(str,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) );
}


} // dev
