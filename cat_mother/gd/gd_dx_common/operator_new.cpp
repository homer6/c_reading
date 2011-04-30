#include "StdAfx.h"
#include <mem/raw.h>
#include "config.h"

//-----------------------------------------------------------------------------

#ifdef _DEBUG
#undef new
void* operator new( unsigned n )
{
	return mem_allocate( n, __FILE__, __LINE__ );
}

void operator delete( void* p )
{
	mem_free( p );
}

void* operator new( unsigned n, const char* file, int line )
{
	return mem_allocate( n, file, line );
}

void operator delete( void* p, const char*, int )
{
	mem_free( p );
}
#endif
