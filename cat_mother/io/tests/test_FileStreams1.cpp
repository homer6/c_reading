#include <tester/Test.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <assert.h>
#include <string.h>

//-----------------------------------------------------------------------------

using io::FileInputStream;
using io::FileOutputStream;

//-----------------------------------------------------------------------------

static void testHelloWorldInput( const char* filename )
{
	FileInputStream in( filename );
	in.skip( 2 );
	assert( 13 == in.available() );
	char buff[256];
	int bytes = in.read( buff, 255 );
	buff[bytes] = 0;
	assert( !strcmp(buff,"Hello, world!") );
}

static int test_FileStreams1()
{
	// basic FileInputStream test
	testHelloWorldInput( "data/hello.txt" );

	// basic FileOutputStream test
	FileOutputStream out( "/tmp/out/hello_out.txt" );
	out.write( "  ", 2 );
	out.write( "Hello, world!", 13 );
	out.flush();
	out.close();
	testHelloWorldInput( "/tmp/out/hello_out.txt" );
	
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test_FileStreams1, __FILE__ );
