#ifndef _MEM_RAW_H
#define _MEM_RAW_H


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


/** 
 * Allocates n byte memory block. 
 * Use mem_alloc macro to get automatic filename and line number.
 * @see mem_free
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void*		mem_allocate( int n, const char* file, int line );

/** 
 * Frees memory block allocated with mem_allocate. 
 * @see mem_allocate
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
MEM_API void		mem_free( void* p );


#if defined(NDEBUG) && !defined(MEM_EXPORTS)
	#include "raw.inl"
#else
	#define MEM_ALLOCATE( BYTES ) mem_allocate(BYTES,__FILE__,__LINE__)
	#define mem_alloc MEM_ALLOCATE
#endif


#endif // _MEM_RAW_H
