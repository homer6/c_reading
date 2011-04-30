#include <malloc.h>
#define mem_Group_create( PARAM, PARAM2 ) 0; (PARAM); (PARAM2)
#define mem_Group_release( PARAM ) (PARAM)
#define mem_Group_copy( PARAM ) (PARAM)
#define mem_Group_allocate( PARAM, BYTES ) malloc(BYTES); (PARAM)
#define mem_Group_free( PARAM, PTR, PARAM2 ) free(PTR); (PARAM); (PARAM2)
#define mem_Group_first() 0
#define mem_Group_next( PARAM ) 0
#define mem_Group_bytesInUse( PARAM ) -1
#define mem_Group_blocksInUse( PARAM ) -1
#define mem_Group_bytesTotal( PARAM ) -1
#define mem_Group_blocksTotal( PARAM ) -1
#define mem_Group_findByName( PARAM ) 0; (PARAM)
#define mem_Group_name( PARAM ) ((PARAM) ? "" : "")
#define mem_Group_findByFreedBlock( PARAM ) (void*)((PARAM) ? 0 : 0)
#define mem_bytesInUse() 0
#define mem_blocksInUse() 0
#define mem_setFlags( PARAM ) (PARAM)
#define mem_flags() (0)
#define mem_printAllocatedBlocks() (0)
