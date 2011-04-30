#include <malloc.h>
#define mem_allocate( BYTES, PARAM, PARAM2 ) malloc(BYTES); (PARAM); (PARAM2);
#define mem_free( PTR ) free(PTR)
#define mem_alloc( BYTES ) malloc(BYTES);
