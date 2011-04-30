#include <tester/Test.h>
#include <io/FileInputStream.h>
#include <util/Properties.h>
#include <assert.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

static int test()
{
	FileInputStream in( "test.prop" );
	Properties prop;
	prop.load( &in );
	assert( prop["Test123"] == "123" );
	assert( prop["TestHelloWorld"] == "Hello, world!" );
	assert( prop["ThirdTest"] == "Third test" );
	assert( prop["LastTest"] == "eof" );

	// mem group test
	printf( "Memory groups in test_Properties:\n" );
	for ( void* g = mem_Group_first() ; g ; g = mem_Group_next(g) )
		printf( "  %s (%i bytes in %i blocks)\n", mem_Group_name(g), mem_Group_bytesInUse(g), mem_Group_blocksInUse(g) );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
