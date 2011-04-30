#include <tester/Test.h>
#include <lang/Thread.h>
#include <lang/Throwable.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER) && defined(_DEBUG) && defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include <direct.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace tester;

//-----------------------------------------------------------------------------

int runAll()
{
	try
	{
		printf( "Number of tests: %i\n", Test::tests() );

		// sort tests by path name
		for ( int i = 0 ; i < Test::tests() ; ++i )
			for ( int j = i+1 ; j < Test::tests() ; ++j )
				if ( strcmp(Test::getTestPath(i),Test::getTestPath(j)) > 0 )
					Test::swapTests(i,j);

		for ( int i = 0 ; i < Test::tests() ; ++i )
		{
			Test::TestFunc	func = Test::getTestFunc(i);
			const char*		name = Test::getTestName(i);
			const char*		path = Test::getTestPath(i);

			_chdir( path );
			printf( "\nRunning %s...\n", name );
			printf( "-----------------------------------------------------------\n", name );
			fflush( stdout );
			int rval = func();
			if ( rval != 0 )
			{
				fprintf( stderr, "ERROR: %s test returned %i\n", name, rval );
				fgetc( stdin );
				return rval;
			}
		}
	}
	catch ( Throwable& e )
	{
		char msg[1024];
		e.getMessage().format().getBytes( msg, sizeof(msg), "ASCII-7" );
		fprintf( stderr, "ERROR: %s\n", msg );
		fgetc( stdin );
		return 1;
	}
	catch ( ... )
	{
		fprintf( stderr, "ERROR: Unknown exception!\n" );
		fgetc( stdin );
		return 2;
	}
	return 0;
}

int main()
{
	int err = runAll();
	lang::Thread::sleep( 1000 );

	#if defined(_MSC_VER) && defined(_DEBUG) && defined(WIN32)
	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
	#endif
	return err;
}

/*
#pragma init_seg( lib )
class TestObj : public lang::Object 
{public: TestObj() {new int;}
} test;
*/
