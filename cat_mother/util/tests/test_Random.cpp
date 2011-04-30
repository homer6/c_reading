#include <tester/Test.h>
#include <util/Random.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <config_msvc.h>
#endif

//-----------------------------------------------------------------------------

using namespace util;

//-----------------------------------------------------------------------------

static int test()
{
	Random rng;

	// number of 1s per bit
	int bits[32];
	for ( int i = 0 ; i < 32 ; ++i )
		bits[i] = 0;

	// get n random numbers,
	// count number of 1s per bit
	const int count = 100000;
	for ( int i = 0 ; i < count ; ++i )
	{
		int x = rng.nextInt();
		int b = 1;
		for ( int k = 0 ; k < 32 ; ++k )
		{
			if ( x & b )
				bits[k] += 1;
			b += b;
		}
	}

	// print non-random bits
	bool firstNonRandom = true;
	for ( int i = 0 ; i < 32 ; ++i )
	{
		double prc = double(bits[i])/double(count)*100.;
		if ( prc < 45 || prc > 55 )
		{
			if ( firstNonRandom )
			{
				printf( "WARNING: rng non-random bits:\n" );
				firstNonRandom = false;
			}
			printf( "  bit %i set %g%%\n", i, prc );
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
