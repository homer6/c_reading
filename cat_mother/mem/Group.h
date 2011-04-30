#ifndef _MEM_GROUP_H
#define _MEM_GROUP_H


#ifdef MEM_EXPORTS
	#ifdef __cplusplus
	#define MEM_API extern "C" __declspec(dllexport)
	#else
	#define MEM_API __declspec(dllexport)
	#endif
#else
	#ifdef __cplusplus
	#define MEM_API extern "C" __declspec(dllimport)
	#else
	#define MEM_API __declspec(dllimport)
	#endif
#endif // MEM_EXPORTS


/** Debug memory flags. */
enum DebugMemFlags
{
	/** Enable exit-time leak check. Default is enabled. */
	DEBUGMEM_LEAKCHECK			= 1,
	/** Enable alloc-time integrity check. Warning: Might slow down debug build a lot. */
	DEBUGMEM_CHECKALWAYS		= 2,
	/** Keeps list of freed memory blocks of each group. */
	DEBUGMEM_LISTFREED			= 4,
};


/** 
 * Creates a new group or references existing group. 
 * Groups are identified by unique group name.
 * Group id is optional user-defined integer which is not used by the library.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void*	mem_Group_create( const char* groupname, int groupid );

/** 
 * Releases group if no more references left. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void	mem_Group_release( void* group );

/** 
 * Copies group by reference. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void*	mem_Group_copy( void* group );

/** 
 * Allocates n byte memory block to the group. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void*	mem_Group_allocate( void* group, int n );

/** 
 * Frees n byte memory block from the group. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void	mem_Group_free( void* group, void* p, int n );

/** 
 * Returns the first group. Group needs to be released after use
 * or passed to mem_Group_next, which in turn releases group passed in.
 * This provides iteration safety (iterated groups are also referenced).
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void*	mem_Group_first();

/** 
 * Returns the next group if any. Releases group passed in. 
 * Returned group needs to be released (or passed in mem_Group_next call).
 * This provides iteration safety (iterated groups are also referenced).
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void*	mem_Group_next( void* group );

/** 
 * Returns number of bytes allocated to the group. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_Group_bytesInUse( void* group );

/** 
 * Returns number of memory blocks allocated to the group. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_Group_blocksInUse( void* group );

/** 
 * Returns all-time total number of bytes allocated to the group. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_Group_bytesTotal( void* group );

/** 
 * Returns all-time total number of memory blocks allocated to the group. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_Group_blocksTotal( void* group );

/** 
 * Returns the name of the group. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API const char*	mem_Group_name( void* group );

/** 
 * Finds group by the address of freed memory block. 
 * Does not add reference to the group.
 * @return 0 If group not found.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void*	mem_Group_findByFreedBlock( void* p );

/** 
 * Returns total number of bytes used by all groups. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_bytesInUse();

/** 
 * Returns total number of blocks used by all groups. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_blocksInUse();

/** 
 * Sets debug memory allocation flags. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void	mem_setFlags( int flags );

/** 
 * Returns debug memory allocation flags. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_flags();

/** 
 * Prints memory leaks if any. 
 * @return Number of blocks allocated.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API int		mem_printAllocatedBlocks();


#if defined(NDEBUG) && !defined(MEM_EXPORTS)
#include "Group.inl"
#endif
//#include "Group.inl"


#endif // _MEM_GROUP_H
