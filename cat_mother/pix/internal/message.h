#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#endif

static void message( const char* fmt, ... )
{
	// format variable arguments
	char msg[1024];
	va_list marker;
	va_start( marker, fmt );
	vsprintf( msg, fmt, marker );
	va_end( marker );
	
	#ifdef WIN32
	OutputDebugString( msg );
	#endif
}
