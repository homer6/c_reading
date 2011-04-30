#include <tester/Test.h>
#include <util/Hashtable.h>
#include <lang/String.h>
#include <assert.h>
#include <stdio.h>
#include "Int.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

static String fmt( int x )
{
	char ch[8];
	sprintf( ch, "%i", x );
	String str = ch;
	return str;
}

static int test()
{
	Allocator< HashtablePair<String,Int> > alloc(__FILE__,__LINE__);
	Hashtable<String,Int> a( alloc );

	int i;
	for ( i = 0 ; i < 100 ; ++i )
	{
		String str = fmt(i);
		a[str] = i;
		assert( a[str] == i );
	}
	int coll = a.collisions();

	Hashtable<String,Int> b( a );
	Hashtable<String,Int> c( alloc );
	c = b;
	a = c;

	for ( i = 0 ; i < 100 ; ++i )
	{
		String str = fmt(i);
		assert( a[str] == i );
	}

	Hashtable<Int,String> d( 1, 0.75f, "-1", Hash<Int>(), Equal<Int>(), Allocator< HashtablePair<Int,String> >(__FILE__,__LINE__) );
	for ( i = 0 ; i < 100 ; ++i )
	{
		String str = fmt(i);
		d[i] = str;
		assert( d[i] == str );
	}
	coll = d.collisions();
	assert( coll == 75 );	// Int::hashCode() returns i&~3

	for ( i = 1 ; i < 100 ; i += 2 )
		d.remove( i );

	i = 0;
	for ( HashtableIterator<Int,String> it = d.begin() ; it != d.end() ; ++it )
	{
		String str = fmt(i);
		assert( it.key() == i );
		assert( it.value() == str );
		i += 2;
	}

 	assert( d[101] == "-1" );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
