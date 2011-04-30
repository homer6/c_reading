#include "Group.h"
#include "Group_t.h"
#include "GroupItem_t.h"
#include "testAndSet.h"
#include "error.h"
#include <string.h>
#include <malloc.h>
#include <assert.h>

#if defined(_DEBUG) && defined(WIN32) && defined(_MSC_VER)
#define DEBUGMEM_WIN32
#endif

#ifdef DEBUGMEM_WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <crtdbg.h>
#endif

#include "config.h"

//-----------------------------------------------------------------------------

static long			s_spin		= 0;
static Group_t*		s_groups	= 0;
static int			s_flags		= DEBUGMEM_LEAKCHECK;
static int			s_blockID	= 1;
static int			s_breakID	= -1;

//-----------------------------------------------------------------------------

/** Increments group reference count. */
static void ref( Group_t* group )
{
	assert( group );
	assert( group->refs >= 0 );
	group->refs += 1;
}

/** Frees group if no more references. */
static void unref( Group_t* group )
{
	assert( group );
	assert( group->refs > 0 );

	group->refs -= 1;
	if ( 0 == group->refs )
	{
		assert( !group->items );

		if ( group->next )
			group->next->prev = group->prev;
		if ( group->prev )
			group->prev->next = group->next;
		if ( group == s_groups )
			s_groups = (group->next ? group->next : group->prev);

		Vector_destroy( group->freedBlocks );
		group->freedBlocks = 0;

		free( group );
	}
}

/** Computes hash code from the string. */
static int strhash( const char* str )
{
	int code = 0;
	while ( *str )
	{
		code *= 31;
		code += *str++;
	}
	return code;
}

/** Refresh system flags. */
static void refreshSystemFlags( int flags )
{
#ifdef DEBUGMEM_WIN32
	/* alloc-time integrity check */
	if ( flags & DEBUGMEM_CHECKALWAYS )
		_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_CHECK_ALWAYS_DF );
	else
		_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_CHECK_ALWAYS_DF );

	/* exit-time leak check */
	if ( flags & DEBUGMEM_LEAKCHECK )
		_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
	else
		_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_LEAK_CHECK_DF );

#else

	flags = flags;

#endif
}


//-----------------------------------------------------------------------------

MEM_API void* mem_Group_create( const char* groupname, int groupid )
{
	char* tmpstr = NULL;
	Group_t* group = NULL;
	int hash;

	/* remove everything before projects directory if found */
	tmpstr = strstr(groupname,"\\projects\\");
	if ( !tmpstr )
		tmpstr = strstr(groupname,"/projects/");
	if ( tmpstr )
		groupname = tmpstr + strlen("/projects/");

	/* make sure we don't have too long group names */
	if ( strlen(groupname) >= GROUP_MAX_NAME )
		groupname = groupname + strlen(groupname) - GROUP_MAX_NAME + 1;

	assert( strlen(groupname) < GROUP_MAX_NAME );
	while ( testAndSet(&s_spin,1) );

	/* set default flags. */
	if ( !s_groups )
		refreshSystemFlags( s_flags );

	/* find existing group */
	hash = strhash(groupname);
	group = s_groups;
	for ( ; group ; group = group->next )
	{
		if ( group->hash == hash && !strcmp(group->name,groupname) )
			break;
	}

	/* create new group if not found */
	if ( !group )
	{
		group = (Group_t*)malloc( sizeof(Group_t) );
		memset( group, 0, sizeof(Group_t) );

		strncpy( group->name, groupname, GROUP_MAX_NAME );
		group->hash = hash;
		group->id = groupid;
		group->name[GROUP_MAX_NAME] = 0;
		group->next = s_groups;
		s_groups = group;
		if ( group->next )
			group->next->prev = group;

		group->freedBlocks = Vector_create( sizeof(FreedBlock_t) );
	}

	/* reference group */
	ref( group );

	testAndSet(&s_spin,0);
	return group;
}

MEM_API void mem_Group_release( void* group_ )
{
	Group_t* group = (Group_t*)group_;

	if ( !group )
		return;

	while ( testAndSet(&s_spin,1) );
	unref( group );
	testAndSet(&s_spin,0);
}

MEM_API void* mem_Group_copy( void* group_ )
{
	Group_t* group = (Group_t*)group_;

	if ( !group )
		return NULL;

	while ( testAndSet(&s_spin,1) );
	ref( group );
	testAndSet(&s_spin,0);
	return group;
}

MEM_API void* mem_Group_allocate( void* group_, int n )
{
	Group_t*		group = (Group_t*)group_;
	GroupItem_t*	item = NULL;
	char*			mem = NULL;
	char*			block = NULL;
	int				groupNameLen = 0;

	assert( n >= 0 );
	assert( sizeof(GroupItem_t) <= BLOCK_HEADER_SIZE );
	while ( testAndSet(&s_spin,1) );

	/* update group */
	group->bytesInUse += n;
	group->blocksInUse += 1;
	group->bytesTotal += n;
	group->blocksTotal += 1;
	ref( group );

	/* allocate block */
	mem = malloc( n + sizeof(int) + BLOCK_HEADER_SIZE );
	item = (GroupItem_t*)( mem );
	memset( item, 0, BLOCK_HEADER_SIZE );
	block = mem + BLOCK_HEADER_SIZE;
	*(int*)(block + n) = 0x1234ABCD; /* check for damaged block */

	/* copy group name to the beginning (to ease debugging) */
	memset( item->groupName, 0, sizeof(item->groupName) );
	groupNameLen = strlen(group->name);
	if ( groupNameLen >= (int)sizeof(item->groupName) )
		strncpy( item->groupName, group->name + groupNameLen - (sizeof(item->groupName)-1), sizeof(item->groupName)-1 );
	else
		strncpy( item->groupName, group->name, sizeof(item->groupName)-1 );

	/* link block */
	item->next = group->items;
	group->items = item;
	if ( item->next )
		item->next->prev = item;
	item->size = n;
	item->group = group;

	/* assign unique id, warn about id wrap */
	item->id = s_blockID;
	if ( s_blockID < 0 )
		message( "Warning: Memory block unique id counter wrapped" );
	else
		++s_blockID;

	/* break at specified id */
	if ( s_breakID == item->id )
	{
		item->id = item->id; // dummy instruction for break point
#ifdef DEBUGMEM_WIN32
		_CrtDbgBreak();
#endif
	}

	testAndSet(&s_spin,0);
	assert( item->group );
	assert( item->size == n );
	return block;
}

MEM_API void mem_Group_free( void* group_, void* p, int n )
{
	Group_t*		group = (Group_t*)group_;
	char*			mem = (char*)p - BLOCK_HEADER_SIZE;
	GroupItem_t*	item = (GroupItem_t*)( mem );
	FreedBlock_t	freedBlock;

	if ( !p )
		return;

	assert( n == item->size ); n = n;
	while ( testAndSet(&s_spin,1) );

	/* check for damaged block */
	if ( *(int*)((char*)p+item->size) != 0x1234ABCD )
		error( "Damaged memory block at 0x%x, size=%i bytes, group=%s", p, n, group->name );
	assert( *(int*)((char*)p+item->size) == 0x1234ABCD );

	/* unlink block */
	if ( item->next )
		item->next->prev = item->prev;
	if ( item->prev )
		item->prev->next = item->next;
	if ( item == group->items )
		group->items = (item->next ? item->next : item->prev);

	/* add block to group's list of free blocks */
	if ( s_flags & DEBUGMEM_LISTFREED )
	{
		freedBlock.begin = mem;
		freedBlock.size = item->size;
		Vector_add( group->freedBlocks, &freedBlock, sizeof(freedBlock) );
	}

	/* update group */
	group->bytesInUse -= item->size;
	group->blocksInUse -= 1;
	unref( group );

	/* free block */
	free( mem );

	testAndSet(&s_spin,0);
}

MEM_API int	mem_Group_bytesInUse( void* group_ )
{
	Group_t* group = (Group_t*)group_;
	return group->bytesInUse + group->blocksInUse * BLOCK_HEADER_SIZE;
}

MEM_API int	mem_Group_blocksInUse( void* group_ )
{
	Group_t* group = (Group_t*)group_;
	return group->blocksInUse;
}

MEM_API int	mem_Group_bytesTotal( void* group_ )
{
	Group_t* group = (Group_t*)group_;
	return group->bytesTotal + group->blocksTotal * BLOCK_HEADER_SIZE;
}

MEM_API int	mem_Group_blocksTotal( void* group_ )
{
	Group_t* group = (Group_t*)group_;
	return group->blocksTotal;
}

MEM_API void* mem_Group_first()
{
	Group_t* first = NULL;

	while ( testAndSet(&s_spin,1) );
	
	first = s_groups;
	if ( first )
		ref( first );

	testAndSet(&s_spin,0);
	return first;
}

MEM_API void* mem_Group_next( void* group_ )
{
	Group_t* group = (Group_t*)group_;
	Group_t* next = NULL;
	
	while ( testAndSet(&s_spin,1) );

	next = group->next;
	if ( next )
		ref( next );
	unref( group );

	testAndSet(&s_spin,0);
	return next;
}

MEM_API const char*	mem_Group_name( void* group_ )
{
	Group_t* group = (Group_t*)group_;
	return group->name;
}

MEM_API int	mem_bytesInUse()
{
	int			count	= 0;
	Group_t*	group	= NULL;

	while ( testAndSet(&s_spin,1) );

	for ( group = s_groups ; group ; group = group->next )
		count += group->bytesInUse;

	testAndSet(&s_spin,0);
	return count;
}

MEM_API int	mem_blocksInUse()
{
	int			count	= 0;
	Group_t*	group	= NULL;

	while ( testAndSet(&s_spin,1) );

	for ( group = s_groups ; group ; group = group->next )
		count += group->blocksInUse;

	testAndSet(&s_spin,0);
	return count;
}

MEM_API void mem_setFlags( int flags )
{
	Group_t*	group	= NULL;

	while ( testAndSet(&s_spin,1) );

	s_flags = flags;
	refreshSystemFlags( s_flags );

	if ( !(flags & DEBUGMEM_LISTFREED) )
	{
		for ( group = s_groups ; group ; group = group->next )
			Vector_clear( group->freedBlocks );
	}

	testAndSet(&s_spin,0);
}

MEM_API int mem_flags()
{
	int flags = 0;

	while ( testAndSet(&s_spin,1) );

#ifdef DEBUGMEM_WIN32
	/* alloc-time integrity check */
	if ( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_CHECK_ALWAYS_DF )
		s_flags |= DEBUGMEM_CHECKALWAYS;

	/* exit-time leak check */
	if ( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_LEAK_CHECK_DF )
		s_flags |= DEBUGMEM_LEAKCHECK;
#endif

	flags = s_flags;

	testAndSet(&s_spin,0);
	return flags;
}

MEM_API void* mem_Group_findByFreedBlock( void* p )
{
	Group_t*		found = NULL;
	Group_t*		group = NULL;
	int				i = 0;
	int				size = 0;
	FreedBlock_t*	fb = NULL;
	char*			pc = (char*)p;

	while ( testAndSet(&s_spin,1) );

	for ( group = s_groups ; group && !found ; group = group->next )
	{
		size = Vector_size( group->freedBlocks );
		for ( i = 0 ; i < size ; ++i )
		{
			fb = (FreedBlock_t*)Vector_get( group->freedBlocks, i );
			if ( pc >= fb->begin && pc <= fb->begin+fb->size )
			{
				found = group;
				break;
			}
		}
	}

	testAndSet(&s_spin,0);

	return found;
}

MEM_API int mem_printAllocatedBlocks()
{
	int				groups		= 0;
	int				blocks		= 0;
	Group_t*		group		= NULL;
	Group_t**		grouplist	= NULL;
	GroupItem_t*	item		= NULL;
	int				i			= 0;

	while ( testAndSet(&s_spin,1) );

	// count groups and blocks
	for ( group = s_groups ; group ; group = group->next )
	{
		if ( group->blocksInUse > 0 )
		{
			blocks += group->blocksInUse;
			++groups;
		}
	}

	// collect groups to reverse order
	grouplist = (Group_t**)malloc( sizeof(Group_t*) * groups );
	i = groups;
	for ( group = s_groups ; group ; group = group->next )
	{
		if ( group->blocksInUse > 0 )
			grouplist[--i] = group;
	}
	assert( i == 0 );

	// print group info
	message( "" );
	message( "---------------------------------------------------------" );
	message( "Allocated memory groups:" );
	message( "" );
	for ( i = 0 ; i < groups ; ++i )
	{
		group = grouplist[i];
		message( "Group (%5i blocks, %7i bytes): %s", group->blocksInUse, group->bytesInUse, group->name );
	}

	// print block info
	message( "" );
	message( "---------------------------------------------------------" );
	message( "Allocated memory blocks:" );
	for ( i = 0 ; i < groups ; ++i )
	{
		group = grouplist[i];
		message( "" );
		message( "Group (%5i blocks, %7i bytes): %s", group->blocksInUse, group->bytesInUse, group->name );
		for ( item = group->items ; item ; item = item->next )
		{
			message( "Block (%7i bytes): id=%8i", item->size, item->id );
		}
	}

	// free collected group list
	free( grouplist );

	testAndSet(&s_spin,0);
	return blocks;
}
