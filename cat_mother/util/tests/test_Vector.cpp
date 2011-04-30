#include <tester/Test.h>
#include <util/Vector.h>
#include <assert.h>
#include <stdio.h>
#include "Int.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

static Vector<Int> reverse( Vector<Int> x )
{
	Vector<Int> y(x);
	for ( int i = 0 ; i < x.size()/2 ; ++i )
	{
		Int tmp = x[i];
		y[i] = x[x.size()-1-i];
		y[x.size()-1-i] = tmp;
	}
	return y;
}

static int test()
{
	Vector<Int> a( Allocator<Int>(__FILE__,__LINE__) );
	int i;
	for ( i = 0 ; i < 100 ; ++i )
		a.add( i );
	
	Vector<Int> b = reverse(a);

	assert( b.size() == a.size() );
	for ( i = 0 ; i < b.size() ; ++i )
	{
		assert( i == b[b.size()-i-1] );
		assert( i == a[i] );
	}

	Vector<int> x( Allocator<int>(__FILE__,__LINE__) );
	x.clear();
	x.setSize( 123, -123 );
	for ( i = 0 ; i < x.size() ; ++i )
	{
		assert( x[i] == -123 );
	}
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
