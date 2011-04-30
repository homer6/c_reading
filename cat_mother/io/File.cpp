#include "File.h"
#include "config.h"
#include "StdioEx.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#endif

//-----------------------------------------------------------------------------

#define PATH_FILE		0x01
#define PATH_DIR		0x02
#define PATH_HIDDEN		0x04
#define PATH_READONLY	0x08

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


#ifdef WIN32

const Char		File::separatorChar			= '\\';
const String	File::separator				= "\\";
const Char		File::pathSeparatorChar		= ';';
const String	File::pathSeparator			= ";";

#else

const Char		File::separatorChar			= '/';
const String	File::separator				= "/";
const Char		File::pathSeparatorChar		= ':';
const String	File::pathSeparator			= ":";

#endif

//-----------------------------------------------------------------------------

/** 
 * Splits path into separate components.
 *
 * @param path Path to split.
 * @param drive [out] Receives drive number if any. 0=none, 1=A, 2=B, 3=C, ...
 * @param directory [out] Receives directory (absolute or relative) if any.
 * @param name [out] Receives filename if any.
 * @param extension [out] Receives file extension (without the dot) if any.
 */
static void splitPath( const String& path, int* drive, String* directory, String* name, String* extension )
{
	// find path terminator if any
	int pathend = path.lastIndexOf('/');
#ifdef WIN32
	{int end2 = path.lastIndexOf('\\');
	if ( end2 > pathend )
		pathend = end2;}
#endif

	// find out drive number, 0=none, 1=A, 2=B, 3=C, ...
	int pathstart = 0;
	int drv = 0;
#ifdef WIN32
	if ( path.length() > 2 && path.charAt(1) == ':' )
	{
		Char ch = path.charAt(1);
		if ( ch >= 'a' && ch <= 'z' )
		{
			drv = ch - 'a' + 1;
			pathstart = 2;
		}
		else if ( ch >= 'A' && ch <= 'Z' )
		{
			drv = ch - 'A' + 1;
			pathstart = 2;
		}
	}
#endif

	// find directory and file name
	String fname;
	String dir;
	if ( pathend >= 0 && pathend+1 < path.length() )
	{
		fname = path.substring( pathend+1 );
		dir = path.substring( pathstart, pathend );
	}
	else
	{
		fname = path.substring( pathstart );
	}

	// unify name separators
#ifdef WIN32
	dir = dir.replace( '\\', '/' );
#endif

	// find file name extension
	String ext;
	if ( extension )
	{
		int extpos = fname.lastIndexOf('.');
		if ( extpos > pathend )
			ext = fname.substring( extpos+1 );
	}

	// store results
	if ( drive )
		*drive = drv;
	if ( directory )
		*directory = dir;
	if ( name )
		*name = fname;
	if ( extension )
		*extension = ext;
}

/** 
 * Gets information about a file or directory.
 * @param path Name of the path to get information.
 * @param flags [out] Receives path status flags. See PATH_DIR, PATH_READONLY, etc.
 * @param size [out] Receives size of the file if not a directory.
 * @return If the file or directory does not exists then the return value is false.
 */
static bool getPathInfo( const lang::String& path, int* flags, long* size )
{
#ifdef WIN32

	char buf[1024];
	path.getBytes( buf, sizeof(buf), "ASCII-7" );
	_finddata_t fileinfo;
	long h = _findfirst( buf, &fileinfo );
	int attrib = 0;
	long filesize = 0;
	if ( -1 != h )
	{
		if ( _A_RDONLY & fileinfo.attrib )
			attrib |= PATH_READONLY;
		if ( _A_HIDDEN & fileinfo.attrib )
			attrib |= PATH_HIDDEN;
		if ( _A_SUBDIR & fileinfo.attrib )
			attrib |= PATH_DIR;
		else
			filesize = fileinfo.size;
		_findclose( h );
	}
	if (flags) *flags = attrib;
	if (size) *size = filesize;
	return -1 != h;

#else

	assert( false ); // unimplemented
	return false;

#endif
}

//-----------------------------------------------------------------------------

File::File()
{
}

File::File( const String& path )
{
	m_path = path.replace( separatorChar, '/' );
	if ( m_path.endsWith("/") )
		m_path = m_path.substring( 0, m_path.length()-1 );
}

File::File( const lang::String& parent, const lang::String& name )
{
	m_path = parent.replace( separatorChar, '/' );
	if ( !m_path.endsWith("/") && m_path.length() > 0 )
		m_path = m_path + "/";
	m_path = m_path + name;
}

bool File::exists() const
{
	return getPathInfo( m_path, 0, 0 );
}

bool File::isAbsolute() const
{
#ifdef WIN32

	return m_path.startsWith("/") || 
		m_path.length() > 2 && m_path.charAt(1) == ':' && m_path.charAt(2) == '/';

#else

	return m_path.startsWith("/");
	
#endif
}

bool File::isDirectory() const
{
	int flags;
	bool found = getPathInfo( m_path, &flags, 0 );
	return found && 0 != (PATH_DIR & flags);
}

bool File::isFile() const
{
	int flags;
	bool found = getPathInfo( m_path, &flags, 0 );
	return found && 0 == (PATH_DIR & flags);
}

long File::length() const
{
	long size;
	getPathInfo( m_path, 0, &size );
	return size;
}

String File::getPath() const
{
	return m_path;
}

String File::getName() const
{
	String name;
	splitPath( m_path, 0, 0, &name, 0 );
	return name;
}

String File::getParent() const
{
	String dir;
	splitPath( m_path, 0, &dir, 0, 0 );
	return dir;
}

String File::getAbsolutePath() const
{
#ifdef WIN32

	char rel[1024];
	m_path.getBytes( rel, sizeof(rel), "ASCII-7" );
	rel[1023] = 0;
	char buf[1024];
	String abspath = m_path;
	if ( _fullpath(buf,rel,sizeof(buf)) )
	{
		buf[0] = (char)toupper( buf[0] );
		abspath = buf;
	}
	return abspath.replace( separatorChar, '/' );

#else

	assert( false ); // unimplemented
	return m_path;

#endif
}

int File::list( String* buffer, int bufferSize ) const
{
#ifdef WIN32

	// path -> path/*
	char fullpath[1024];
	String str = getAbsolutePath().replace( '/', separatorChar );
	str.getBytes( fullpath, sizeof(fullpath), "ASCII-7" );
	fullpath[1020] = 0;
	int len = strlen(fullpath);
	if ( fullpath[len-1] != '\\' )
		fullpath[len++] = '\\';
	fullpath[len++] = '*';
	fullpath[len] = 0;

	int i = 0;
	int count = 0;
	_finddata_t fileinfo;
	long h = _findfirst( fullpath, &fileinfo );
	if ( -1 != h )
	{
		do
		{
			if ( strcmp(fileinfo.name,".") && 
				strcmp(fileinfo.name,"..") )
			{
				if ( i < bufferSize )
					buffer[i++] = fileinfo.name;
				++count;
			}
		} while ( 0 == _findnext(h,&fileinfo) );
		_findclose( h );
	}
	return count;

#else

	assert( false ); // unimplemented
	return 0;

#endif
}


} // io
