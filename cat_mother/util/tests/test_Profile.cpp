#include <tester/Test.h>
#include <dev/Profile.h>
#include <dev/TimeStamp.h>
#include <lang/Thread.h>
#include <lang/String.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

using namespace dev;
using namespace lang;

//-----------------------------------------------------------------------------

class Th1 :
	public Thread
{
public:
	void run()
	{
		TimeStamp c0 = TimeStamp::currentTime();
		{
			Profile pr("Th1");
			clock_t clock0 = clock();
			while ( double(clock()-clock0)/double(CLOCKS_PER_SEC) < 1.0 );
		}
		TimeStamp c1 = TimeStamp::currentTime();
		printf( "Th1 run %f seconds\n", (c1-c0).seconds() );
		fflush( stdout );
	}
};

class Th2 :
	public Thread
{
public:
	void run()
	{
		TimeStamp c0 = TimeStamp::currentTime();
		{
			Profile pr("Th2");
			clock_t clock0 = clock();
			while ( double(clock()-clock0)/double(CLOCKS_PER_SEC) < 2.0 );
		}
		TimeStamp c1 = TimeStamp::currentTime();
		printf( "Th2 run %f seconds\n", (c1-c0).seconds() );
		fflush( stdout );
	}
};

//-----------------------------------------------------------------------------

static void check( double x, double y )
{
	// reference clock() accuracy isn't too great so allow some error
	const float margin = 0.5f;
	assert( (x-y) < margin && (x-y) > -margin );
}

//-----------------------------------------------------------------------------

static int test()
{
	{Profile pr("init");}		// force timer initialization
	
	P(Th1) th1 = new Th1;
	P(Th2) th2 = new Th2;
	th1->start();
	th2->start();
	th1->join();
	th2->join();
	th1->start();
	th2->start();
	th2->join();
	th1->join();

	for ( int i = 0 ; i < Profile::count() ; ++i )
	{
		Profile::BlockInfo* bl = Profile::get(i);

		if ( String(bl->name()) == "Th1" )
		{
			check( bl->time(), 2 );
		}
		else if ( String(bl->name()) == "Th2" )
		{
			check( bl->time(), 4 );
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
