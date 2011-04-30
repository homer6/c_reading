#include <tester/Test.h>
#include <io/ByteArrayInputStream.h>
#include <io/ByteArrayOutputStream.h>
#include <assert.h>
#include <string.h>

//-----------------------------------------------------------------------------

using io::ByteArrayInputStream;
using io::ByteArrayOutputStream;

//-----------------------------------------------------------------------------

static void testHelloWorldInput( const char* buff, long size )
{
	ByteArrayInputStream in( buff, size );
	in.skip( 2 );
	assert( 13 == in.available() );
	char ibuff[256];
	int bytes = in.read( ibuff, sizeof(ibuff) );
	ibuff[bytes] = 0;
	assert( !strcmp(ibuff,"Hello, world!") );
}

static int test_ByteArrayStreams1()
{
	// basic ByteArrayInputStream test
	const char buff[] = "  Hello, world!";
	testHelloWorldInput( buff, strlen(buff) );

	// basic ByteArrayOutputStream test
	ByteArrayOutputStream out( 10 );
	out.write( "  ", 2 );
	out.write( "Hello, world!", 13 );
	out.flush();
	char obuff[256];
	out.toByteArray( obuff, sizeof(obuff) );
	testHelloWorldInput( obuff, out.size() );
	
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test_ByteArrayStreams1, __FILE__ );
