#include "StdAfx.h"
#include "Sort.h"
#include <math/FloatUtil.h>
#include <assert.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

class SortPair
{
public:
	float	value;
	int		index;

	bool	operator<( const SortPair& other ) const {return value < other.value;}
};

//-----------------------------------------------------------------------------

void Sort::bucketSort( const float* begin, const float* end, 
	int* buffer, int* order )
{
	const int BUCKETS = 256;	// must be power of 2

	// reset buckets
	const int BUCKETS2 = BUCKETS * 2;
	int buckets[BUCKETS2];
	for ( int i = 0 ; i < BUCKETS2 ; ++i )
		buckets[i] = -1;

	// find domain
	float rangeMin = FLT_MAX;
	float rangeMax = -FLT_MAX;
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
	if ( invRangeDelta < -FLT_MIN || invRangeDelta > FLT_MIN )
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

void Sort::stdSort( const float* begin, const float* end, 
	int* buffer, int* order )
{
	int count = end - begin;
	SortPair* buf = reinterpret_cast<SortPair*>(buffer);
	for ( int i = 0 ; i < count ; ++i )
	{
		buf[i].value = begin[i];
		buf[i].index = i;
	}

	std::sort( buf, buf+count );

	for ( int i = 0 ; i < count ; ++i )
		order[i] = buf[i].index;
}
