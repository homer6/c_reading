#include <tester/Test.h>
#include <dev/Profile.h>
#include <dev/TimeStamp.h>
#include <algorithm>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//-----------------------------------------------------------------------------

using namespace dev;

//-----------------------------------------------------------------------------

// assumes that array indices start at 1
static void mergesortReference( int* a, int l, int r, int* b, int* c )
{
	if ( r-l > 0 )
	{
		int m = unsigned(r+l)/2U;
		mergesortReference( a, l, m, b, c );
		mergesortReference( a, m+1, r, b, c );

		for ( int i = 1 ; i <= m-l+1 ; ++i )
			b[i] = a[l+i-1];
		for ( int j = m+1 ; j <= r ; ++j )
			c[j-m] = a[j];

		i = 1; 
		j = 1; 
		b[m-l+2] = INT_MAX; 
		c[r-m+1] = INT_MAX;

		for ( int k = l ; k <= r ; ++k )
		{
			if ( b[i] < c[j] )
				a[k] = b[i++];
			else
				a[k] = c[j++];
		}
	}
}

static void cpy( int* d, const int* s, int count )
{
	assert( count >= 0 );
	int* d1 = d + count;
	for ( ; d != d1 ; ++d )
		*d = *s++;
}

static int test()
{
	const int	maxsize		= 1000000;
	const int	tests		= 5;
	int			sizes[tests];
	double		time0[tests];
	double		time1[tests];

	int*		a0			= new int[maxsize];
	int*		a1			= new int[maxsize];
	int*		a2			= new int[maxsize];
	int*		b			= new int[maxsize];
	int*		c			= new int[maxsize];

	int seed = TimeStamp::currentTime().low;
	printf( "srand = %i\n", seed );
	srand( seed );

	int testsize = maxsize;
	int t;
	for ( t = tests-1 ; t >= 0 ; --t )
	{
		sizes[t] = testsize;
		testsize /= 10;
	}

	for ( t = 0 ; t < tests ; ++t )
	{
		const int size = sizes[t];
		assert( size > 0 );
		assert( size <= maxsize );

		int i;
		for ( i = 0 ; i < size ; ++i )
			a0[i] = rand();

		{cpy( a1, a0, size );
		TimeStamp t0 = TimeStamp::currentTime();
		std::sort( a1, a1+size );
		TimeStamp t1 = TimeStamp::currentTime();
		time0[t] = (t1-t0).seconds();}

		{cpy( a2, a0, size );
		TimeStamp t0 = TimeStamp::currentTime();
		mergesortReference( a2-1, 1, size, b-1, c-1 );
		TimeStamp t1 = TimeStamp::currentTime();
		time1[t] = (t1-t0).seconds();}

		for ( int k = 0 ; k < size ; ++k )
		{
			assert( a1[k] == a2[k] );
		}
	}

	for ( t = 0 ; t < tests ; ++t )
	{
		printf( "%i elements:\n", sizes[t] );
		printf( "    t0=%g (std::sort)\n", time0[t] );
		printf( "    t1=%g (mergesort)\n", time1[t] );
		double d = time0[t] - time1[t];
		if ( d < 0 )
			printf( "    std::sort won by %g %%", -d/time1[t]*100 );
		else
			printf( "    mergesort won by %g %%", d/time0[t]*100 );
		printf( "\n" );
	}

	delete[] c;
	delete[] b;
	delete[] a2;
	delete[] a1;
	delete[] a0;
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
