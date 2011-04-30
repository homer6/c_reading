#include <tester/Test.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace tester
{


struct TestEntry
{
	Test::TestFunc	func;
	char			name[256];
	char			path[2048];
};

//-----------------------------------------------------------------------------

static TestEntry* testArray()
{
	static TestEntry s_tests[256];
	return s_tests;
}
	
static int& testCount()
{
	static int s_testCount = 0;
	return s_testCount;
}

//-----------------------------------------------------------------------------

Test::Test( TestFunc func, const char* filename )
{
	// func
	TestEntry test;
	test.func = func;

	// name
	const char* name1 = strrchr( filename, '/' );
	const char* name2 = strrchr( filename, '\\' );
	const char* name = name1;
	if ( name2 > name1 )
		name = name2;
	if ( !name )
		name = filename;
	else
		++name;
	strncpy( test.name, name, sizeof(test.name)-1 );
	test.name[ sizeof(test.name)-1 ] = 0;

	// path
	int pathLen = name - filename - 1;
	if ( pathLen >= sizeof(test.path) )
		pathLen = sizeof(test.path)-1;
	memset( test.path, 0, sizeof(test.path) );
	strncpy( test.path, filename, pathLen );

	testArray()[testCount()++] = test;

	printf( "Registered test %i at %s\n", testCount(), filename );
}

int Test::tests()
{
	return testCount();
}

Test::TestFunc Test::getTestFunc( int i )
{
	assert( i >= 0 && i < tests() );
	return testArray()[ i ].func;
}

const char*	Test::getTestName( int i )
{
	assert( i >= 0 && i < tests() );
	return testArray()[ i ].name;
}

const char*	Test::getTestPath( int i )
{
	assert( i >= 0 && i < tests() );
	return testArray()[ i ].path;
}

void Test::swapTests( int i, int j )
{
	TestEntry tmp = testArray()[i];
	testArray()[i] = testArray()[j];
	testArray()[j] = tmp;
}


} // tester
