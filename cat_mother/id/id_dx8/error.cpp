#include "StdAfx.h"
#include "error.h"
#include <stdio.h>
#include <stdarg.h>

#include "config.h"

//-----------------------------------------------------------------------------

enum DebugLevel
{
	LEVEL_MESSAGE,
	LEVEL_WARNING,
	LEVEL_ERROR
};

//-----------------------------------------------------------------------------

static void print( enum DebugLevel level, const char* msg )
{
	char str[2000];
	const char* typeStr = NULL;

	if ( level == LEVEL_MESSAGE )
		typeStr = "INFO";
	else if ( level == LEVEL_WARNING )
		typeStr = "WARNING";
	else
		typeStr = "ERROR";

	sprintf( str, "%s: id_dx8: %s\n", typeStr, msg );
	#ifdef WIN32
	OutputDebugString( str );
	#else
	puts( str );
	#endif
}

void error( const char* fmt, ... )
{
	// format variable arguments
	char msg[1024];
	va_list marker;
	va_start( marker, fmt );
	vsprintf( msg, fmt, marker );
	va_end( marker );

	print( LEVEL_ERROR, msg );
}

void message( const char* fmt, ... )
{
	// format variable arguments
	char msg[1024];
	va_list marker;
	va_start( marker, fmt );
	vsprintf( msg, fmt, marker );
	va_end( marker );

	print( LEVEL_MESSAGE, msg );
}
