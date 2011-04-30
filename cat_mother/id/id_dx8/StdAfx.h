#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dinput.h>

// Debug global memory allocation
#ifdef _DEBUG
void*	operator new( unsigned n );
void*	operator new( unsigned n, const char* file, int line );
void	operator delete( void* p );
void	operator delete( void* p, const char* file, int line );
#define LANG_DEBUG_NEW new(__FILE__,__LINE__)
#endif
