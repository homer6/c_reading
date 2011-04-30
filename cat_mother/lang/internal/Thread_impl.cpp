#include "Thread_impl.h"
#include <lang/Debug.h>

#ifdef LANG_WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <process.h>
	#include <stdio.h>
#else
	#include <unistd.h>
	#include <pthread.h>
#endif

#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


typedef void* (*start)(void*);

struct ThreadParam
{
	start	func;
	void*	arg;
};

//-----------------------------------------------------------------------------

#ifdef LANG_WIN32
static unsigned __stdcall threadProc( void* a )
{
	ThreadParam*	param	= (ThreadParam*)a;
	start			func	= param->func;
	void*			arg		= param->arg;

	delete param;
	param = 0;

	unsigned rv = (unsigned)func( arg );
	_endthreadex( rv );
	return rv;
}
#endif

//-----------------------------------------------------------------------------

int Thread_create( Thread_t* t, void* (*start)(void*), void* arg )
{
#ifdef LANG_WIN32

	ThreadParam* param = new ThreadParam;
	param->func = start;
	param->arg = arg;
	unsigned id;
	HANDLE h = (HANDLE)_beginthreadex( 0, 0, threadProc, param, 0, &id );
	if ( 0 == h )
	{
		*t = 0;
		delete param;
		return 1;
	}
	else
	{
		*t = (Thread_t)h;
		Debug::println( "The thread 0x{0,X} (handle=0x{0,X}) started.", (int)id, (int)h );
		return 0;
	}

#else

	// pthread_t is not a ptr in every platform
	pthread_t* h = new pthread_t;
	int err = pthread_create( h, 0, start, arg );
	if ( err ) 
	{
		delete h;
		*t = 0;
	}
	else
	{
		*t = (Thread_t)h;
	}
	return err;

#endif
}

int Thread_detach( Thread_t t )
{
#ifdef LANG_WIN32

	HANDLE h = (HANDLE)t;
	if ( h )
	{
		Debug::println( "Detaching thread (handle=0x{0,X}).", (int)h );
		CloseHandle( h );
		return 0;
	}
	return 1;

#else

	pthread_t* h = (pthread_t*)t;
	int rv = pthread_detach( *h );
	delete h;
	return rv;

#endif
}

int Thread_join( Thread_t t )
{
#ifdef LANG_WIN32

	HANDLE h = (HANDLE)t;
	if ( h )
	{
		Debug::println( "Joining thread (handle=0x{0,X})...", (int)h );
		DWORD rv = WaitForSingleObject( h, INFINITE );
		if ( WAIT_OBJECT_0 == rv )
		{
			CloseHandle( h );
			return 0;
		}
	}
	return 1;

#else

	pthread_t* h = (pthread_t*)t;
	int rv = pthread_join( h );
	delete h;
	return rv;

#endif
}

int Thread_sleep( int millis )
{
#ifdef LANG_WIN32

	Sleep( millis );
	return 0;

#else

	usleep( millis*1000 );
	return 0;

#endif
}


} // lang
