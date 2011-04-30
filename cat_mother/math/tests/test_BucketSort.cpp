#include <tester/Test.h>
#include <dev/Profile.h>
#include <lang/Float.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <math/FloatUtil.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include <math/internal/config.h>

//-----------------------------------------------------------------------------

using namespace dev;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

/**
 * Sorts array using buckets. O(n) algorithm. Sort accuracy is 8 bits.
 * @param begin Beginning of the array to sort.
 * @param end End of the array to sort.
 * @param buffer Temporary buffer. Must be at least TWICE the size of input.
 * @param order [out] Receives ascending order. (indices to the source data)
 */
void bucketSort( const float* begin, const float* end, 
	int* buffer, int* order )
{
	const int BUCKETS = 256;	// must be power of 2

	// reset buckets
	const int BUCKETS2 = BUCKETS * 2;
	//Vector<int> buckets( Allocator<int>(__FILE__,__LINE__) ); buckets.setSize( BUCKETS2 );
	int buckets[BUCKETS2];
	for ( int i = 0 ; i < BUCKETS2 ; ++i )
		buckets[i] = -1;

	// find domain
	float rangeMin = Float::MAX_VALUE;
	float rangeMax = -Float::MAX_VALUE;
	for ( const float* it = begin ; it != end ; ++it )
	{
		float v = *it;
		if ( v < rangeMin )
			rangeMin = v;
		if ( v > rangeMax )
			rangeMax = v;
	}

	// scale domain to bucket domain
	float invRangeDelta = rangeMax - rangeMin;
	if ( invRangeDelta < -Float::MIN_VALUE || invRangeDelta > Float::MIN_VALUE )
		invRangeDelta = 1.f / invRangeDelta;
	else
		invRangeDelta = 0.f;

	// put values to buckets
	float scale = invRangeDelta * (float)(BUCKETS-1);
	int bufferItems = 0;
	int index = 0;
	for ( const float* it = begin ; it != end ; ++it )
	{
		int item = FloatUtil::floatToInt( (*it - rangeMin)*scale );
		assert( item >= 0 && item < BUCKETS );
		int* bucket = &buckets[ unsigned(item & (BUCKETS-1)) * 2U ];
		
		if ( -1 == bucket[0] )
		{
			bucket[0] = index;
		}
		else
		{
			buffer[bufferItems] = index;
			buffer[bufferItems+1] = bucket[1];
			bucket[1] = bufferItems;
			bufferItems += 2;
		}

		++index;
	}

	// get indices from buckets
	int* order0 = order;
	for ( int i = 0 ; i < BUCKETS ; ++i )
	{
		int* bucket = &buckets[ (unsigned)i * 2U ];
		
		if ( -1 != bucket[0] )
		{
			*order++ = bucket[0];
		
			while ( -1 != bucket[1] )
			{
				assert( bucket[1] >= 0 && bucket[1] < (end-begin)*2 );
				bucket = buffer + bucket[1];
				*order++ = bucket[0];
			}
		}
	}
	assert( order-order0 == end-begin ); order0 = order0;
}

//-----------------------------------------------------------------------------

static int test()
{
	const int		COUNT = 65536;
	float			a[COUNT];
	float			b[COUNT];
	int				ind[COUNT];

	Vector<int>		buffer( Allocator<int>(__FILE__,__LINE__) ); 
	buffer.setSize( COUNT*2 );

	float rangeMin = -1000.f;
	float rangeMax = 1000.f;
	for ( int i = 0 ; i < COUNT ; ++i )
	{
		float f = (float)rand() / (float)RAND_MAX;
		float v = (rangeMax-rangeMin)*f + rangeMin;
		a[i] = v;
	}

	for ( int i = 0 ; i < COUNT ; ++i )
		b[i] = a[i];
	{Profile pr( "std" );
	std::sort( b, b+COUNT );}

	{Profile pr( "bucket" );
	bucketSort( a, a+COUNT, buffer.begin(), ind );}

	/*printf( "\nunsorted: " );
	for ( int i = 0 ; i < COUNT ; ++i )
		printf( "%g, ", a[i] );
	printf( "\nbucketSort: " );
	for ( int i = 0 ; i < COUNT ; ++i )
		printf( "%g ", a[ind[i]] );
	printf( "\nstd.sort: " );
	for ( int i = 0 ; i < COUNT ; ++i )
		printf( "%g ", b[i] );*/

	// profiling statistics
	for ( int k = 0 ; k < Profile::count() ; ++k )
	{
		Profile::BlockInfo* b = Profile::get( k );
		printf( "%s: %g ms (%g/s)\n", b->name(), b->time()*1e3f, 1.f/b->time() );
	}
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
