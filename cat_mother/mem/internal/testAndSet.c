#include "testAndSet.h"

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <pthread.h>
	static pthread_mutex_t s_refCountMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#include "config.h"

//-----------------------------------------------------------------------------

long testAndSet( long* value, long newValue )
{
	#ifdef WIN32

		return InterlockedExchange( value, newValue );

	#else

		pthread_mutex_lock( &s_refCountMutex );
		long v = *value;
		*value = newValue;
		pthread_mutex_unlock( &s_refCountMutex );
		return v;

	#endif
}
