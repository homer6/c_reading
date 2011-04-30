#include <tester/Test.h>
#include <lang/Format.h>
#include <assert.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

static int test()
{
	{
		Char buff[512];
		
		Format fmt( "Hello, world." );
		int len = fmt.format( buff, 512 );
		assert( String(buff) == "Hello, world." );
		
		fmt = Format( "1={1}, 1.23={0,#.00}, 0023={2,0000}, xyz={3}", 1.23, 1, 23, "xyz" );
		len = fmt.format( buff, 512 );
		assert( String(buff) == "1=1, 1.23=1.23, 0023=0023, xyz=xyz" );
	}

	{
		Char buff[13];

		Format fmt( "Hello, world." );
		int len = fmt.format( buff, 13 );
		assert( String(buff) == "Hello, world" );
		
		fmt = Format( "1={1}, 1.23={0,#.00}, 0023={2,0000}", 1.23, 1, 23 );
		len = fmt.format( buff, 13 );
		assert( String(buff) == "1=1, 1.23=1." );
	}
	
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
