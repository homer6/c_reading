#include "listFiles.h"
#include <io/File.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <config_msvc.h>
#endif

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

void listFiles( const String& path, 
	const String& prefix, const String& suffix, Vector<String>& files )
{
	// get file names in the directory
	Vector<String> v( Allocator<String>(__FILE__,__LINE__) );
	v.setSize( File(path).list(0,0) );
	int count = File(path).list( v.begin(), v.size() );
	if ( count < v.size() )
		v.setSize( count );
	
	// check file names for the pattern
	for ( int i = 0 ; i < v.size() ; ++i )
	{
		String fname = v[i].toLowerCase();
		if ( fname.startsWith(prefix) && fname.endsWith(suffix) )
		{
			File file( path, v[i] );
			if ( file.isFile() )
				files.add( file.getAbsolutePath() );
		}
	}

	// recurse subdirs
	for ( int i = 0 ; i < v.size() ; ++i )
	{
		File file( path, v[i] );
		if ( file.isDirectory() )
			listFiles( file.getAbsolutePath(), prefix, suffix, files );
	}
}

//-----------------------------------------------------------------------------

