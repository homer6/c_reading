#include "StdioEx.h"
#include <lang/String.h>
#include <assert.h>
#include "config.h"

//----------------------------------------------------------------------------

using namespace lang;

//----------------------------------------------------------------------------

namespace io
{


// Truncates string to char string, ignores non-convertable characters
static void truncate( const String& str, char* buffer, int bufferSize )
{
	assert( str.length() < bufferSize );

	if ( bufferSize > 0 )
	{
		int iend = str.length();
		char* bufferEnd = buffer+bufferSize-1;
		for ( int i = 0 ; i != iend && buffer != bufferEnd ; ++i )
		{
			Char ch = str.charAt(i);
			if ( ch < Char(0x80) )
				*buffer++ = (char)ch;
		}
		*buffer++ = 0;
	}
}

// Truncates string to char string, ignores non-convertable characters
static void truncate_w( const String& str, wchar_t* buffer, int bufferSize )
{
	assert( str.length() < bufferSize );

	if ( bufferSize > 0 )
	{
		int iend = str.length();
		wchar_t* bufferEnd = buffer+bufferSize-1;
		for ( int i = 0 ; i != iend && buffer != bufferEnd ; ++i )
		{
			Char ch = str.charAt(i);
			*buffer++ = (wchar_t)ch;
		}
		*buffer++ = 0;
	}
}

/*
 * Truncates Unicode file name to ASCII-7 range and tries to open a file.
 * Ignores non-convertable characters.
 */
static FILE* fopen_ascii7( const String& filename, const char* access )
{
	FILE* file = 0;
	if ( filename.length()+1 < 2048 )
	{
		char filenameBuffer[2048];
		truncate( filename, filenameBuffer, 2048 );
		file = ::fopen( filenameBuffer, access );
	}
	else
	{
		char* filenameBuffer = new char[filename.length()+1];
		truncate( filename, filenameBuffer, filename.length()+1 );
		file = ::fopen( filenameBuffer, access );
		delete[] filenameBuffer;
	}
	return file;
}

//-----------------------------------------------------------------------------

FILE* fopen( const String& filename, const char* access )
{
#ifdef WIN32

	wchar_t accessW[16];
	int i;
	for ( i = 0 ; access[i] && i+1 < 16 ; ++i )
		accessW[i] = (unsigned char)access[i];
	accessW[i] = 0;

	FILE* file = 0;
	if ( filename.length()+1 < 2048 )
	{
		wchar_t filenameBuffer[2048];
		truncate_w( filename, filenameBuffer, 2048 );
		file = _wfopen( filenameBuffer, accessW );
	}
	else
	{
		wchar_t* filenameBuffer = new wchar_t[filename.length()+1];
		truncate_w( filename, filenameBuffer, filename.length()+1 );
		file = _wfopen( filenameBuffer, accessW );
		delete[] filenameBuffer;
	}

	// if we failed then we still try to open with ASCII-7 filename
	// (in case the Windows platform doesn't support _wfopen)
	if ( !file )
		return fopen_ascii7( filename, access );

	return file;

#else

	return fopen_ascii7( filename, access );

#endif
}


} // io
