#include <tester/Test.h>
#include <lang/String.h>
#include <lang/Format.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

static int test_String1()
{
	const char* xa = "Hello, world!";
	const char* xa2 = " \n Hello, world! \n ";
	String x = xa;
	String xs = x.substring(7,12);
	assert( xs.length() == 12-7 );
	assert( xs == "world" );
	assert( String(xa+7,5) == xs );
	assert( x.endsWith("world!") );
	assert( !x.endsWith("world") );
	assert( x.startsWith("Hello") );
	assert( !x.startsWith("ello") );
	assert( x.hashCode() != 0 );
	assert( 7 == x.indexOf('w') );
	assert( 8 == x.indexOf("orld") );
	assert( 8 == x.lastIndexOf('o') );
	assert( 8 == x.lastIndexOf("o") );
	assert( x.replace('l','m') == "Hemmo, wormd!" );
	assert( String(xa2).trim() == xa );
	assert( String("greenblob #f").indexOf("#f") == 10 );
	assert( String("x.lua").getBytes(0,0,"ASCII-7") == 5 );
	char buf[256]; String("x.lua").getBytes(buf,sizeof(buf),"ASCII-7"); 
	assert( !strcmp(buf,"x.lua") );
	int len = Format("1,2,{0}",3).format().getBytes( buf, sizeof(buf), "ASCII-7" );
	assert( len == 5 );
	assert( !strcmp(buf,"1,2,3") );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test_String1, __FILE__ );
