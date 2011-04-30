#include "StdAfx.h"
#include "LogFile.h"
#include <time.h>

//-----------------------------------------------------------------------------

LogFile::LogFile( const char* name )
{
	m_file = fopen( name, "at" );
	
	if ( m_file )
	{
		time_t t;
		time( &t );
		fputs( "\n\n", m_file );
		fputs( asctime(localtime(&t)), m_file );
		fputs( "----------------------------------------------------------\n", m_file );
	}
}

LogFile::~LogFile()
{
	fclose( m_file );
}

void LogFile::printf( const char* fmt, ... )
{
	if ( m_file )
	{
		// format variable arguments
		char msg[2000];
		va_list marker;
		va_start( marker, fmt );
		vsprintf( msg, fmt, marker );
		va_end( marker );

		strcat( msg, "\n" );
		fputs( msg, m_file );
	}
}
