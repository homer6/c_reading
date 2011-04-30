#include <tester/Test.h>
#include <io/InputStream.h>
#include <io/DirectoryInputStreamArchive.h>
#include <util/Vector.h>
#include <stdio.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

static int test()
{
	DirectoryInputStreamArchive arc;
	arc.addPath( "./" );
	arc.addPaths( "../../tester/all" );

	P(InputStream) testInputStream = arc.getInputStream( "all.dsp" );
	assert( testInputStream );

	char msg[1024];
	arc.toString().getBytes( msg, sizeof(msg), "ASCII-7" );
	printf( "Files (%s):\n", msg );
	for ( int i = 0 ; i < arc.size() ; ++i )
	{
		arc.getEntry(i).getBytes( msg, sizeof(msg), "ASCII-7" );
		printf( "%s, ", msg );
	}
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
