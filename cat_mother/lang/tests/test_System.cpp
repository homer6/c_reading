#include <tester/Test.h>
#include <lang/System.h>
#include <lang/Thread.h>
#include <assert.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

using lang::System;
using lang::Thread;

//-----------------------------------------------------------------------------

static int test_System1()
{
	const long margin = 50;		// acceptable error for a second, ms
	long t = System::currentTimeMillis();
	Thread::sleep( 1000 );
	t = System::currentTimeMillis() - t;
	assert( t >= 1000-margin && t <= 1000+margin );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test_System1, __FILE__ );
