#include "Int.h"
#include <mem/raw.h>
#include <util/Allocator.h>
#include <assert.h>
#include <stdio.h>

#if defined(_DEBUG) && defined(WIN32) && defined(_MSC_VER)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <crtdbg.h>
#endif

#include <mem/internal/config.h>

//-----------------------------------------------------------------------------

using namespace util;

//-----------------------------------------------------------------------------

int main()
{
	// enabled exit-time leak check
	#if defined(_DEBUG) && defined(WIN32) && defined(_MSC_VER)
	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
	#endif

	Allocator<Int> alloc( "Test", 0 );
	Allocator<Int> alloc2( "Test2", 0 );

	Int* p1 = alloc.allocate(1);
	Int* p2 = alloc.allocate(1);
	Int* a1 = alloc.allocate(10);
	Int* a2 = alloc2.allocate(4);
	void* foo1 = mem_allocate(123,"Foo",0);

	int blocks = alloc.blocksInUse();
	assert( blocks == 3 );
	int bytes = alloc.bytesInUse();
	assert( bytes == (int)sizeof(Int)*(1+1+10)+16*3 );

	printf( "Groups:\n" );
	for ( void* group = mem_Group_first() ; group ; group = mem_Group_next(group) )
		printf( "  %s: %i bytes in %i blocks\n", mem_Group_name(group), mem_Group_bytesInUse(group), mem_Group_blocksInUse(group) );

	mem_setFlags( mem_flags() | DEBUGMEM_LISTFREED );
	alloc.deallocate(p1,1);
	alloc.deallocate(p2,1);
	alloc.deallocate(a1,10);
	alloc2.deallocate(a2,4);
	mem_free(foo1);

	void* group = mem_Group_findByFreedBlock( a1 );
	if ( group )
		printf( "a1 was in group %s", mem_Group_name(group) );

	blocks = alloc.blocksInUse();
	assert( blocks == 0 );
	bytes = alloc.bytesInUse();
	assert( bytes == 0 );
	printf( "ok\n" );
	return 0;
}


