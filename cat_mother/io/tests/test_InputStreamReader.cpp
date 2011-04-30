#include <tester/Test.h>
#include <io/ByteArrayInputStream.h>
#include <io/InputStreamReader.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

static int test()
{
	char sz[] = "Hello, world!";
	P(ByteArrayInputStream) in = new ByteArrayInputStream( sz, strlen(sz) );
	P(InputStreamReader) reader = new InputStreamReader( in );
	
	Char ch[8];
	reader->read( ch, 1 );
	assert( ch[0] == 'H' );
	reader->read( ch, 4 );
	assert( ch[3] == 'o' );
	reader->mark( 3 );
	reader->read( ch, 2 );
	assert( ch[1] == ' ' );
	reader->reset();
	reader->read( ch, 4 );
	assert( ch[3] == 'o' );
	int count = reader->read( ch, 8 );
	assert( 4 == count );
	count = 0;
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
