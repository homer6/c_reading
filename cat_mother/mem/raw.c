#include "raw.h"
#include "Group.h"
#include "Group_t.h"
#include "GroupItem_t.h"
#include "config.h"

//-----------------------------------------------------------------------------

MEM_API void* mem_allocate( int n, const char* file, int line )
{
	void* group = mem_Group_create( file, line );
	void* p = mem_Group_allocate( group, n );
	mem_Group_release( group );
	return p;
}

MEM_API void mem_free( void* p )
{
	if ( p )
	{
		GroupItem_t* item = (GroupItem_t*)( (char*)p - BLOCK_HEADER_SIZE );
		mem_Group_free( item->group, p, item->size );
	}
}
