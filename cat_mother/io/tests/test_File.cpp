#include <tester/Test.h>
#include <io/File.h>
#include <util/Vector.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

static int test()
{
	File f( "../../io/tests/test_File.cpp" );
	assert( f.exists() );
	assert( f.isFile() );
	assert( !f.isDirectory() );
	assert( f.length() > 300 );
	assert( f.getName() == "test_File.cpp" );
	assert( f.getParent() == "../../io/tests" );

	File d( "../../io" );
	assert( d.exists() );
	assert( !d.isFile() );
	assert( d.isDirectory() );
	assert( d.getName() == "io" );
	assert( d.getParent() == "../.." );
	Vector<String> files( Allocator<String>(__FILE__) );
	files.setSize( d.list(0,0) );
	d.list( files.begin(), files.size() );
	assert( files.indexOf("File.cpp") >= 0 );

	File d2( d.getPath(), "tests" );
	assert( d2.isDirectory() );
	File f2( d2.getPath(), "test_File.cpp" );
	assert( f2.isFile() );

	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
