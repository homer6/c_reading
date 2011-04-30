/* Author: Johnson M. Hart */
/* Emulate the Pthreads model for the Win32 platform */
/* The emulation is not complete, but it does provide a subset */
/* required for a first project */

#ifndef _THREAD_EMULATION
#define _THREAD_EMULATION
#ifdef _WINDOWS

/* Win32 */
#include <process.h>
#define pthread_t HANDLE
#define pthread_attr_t DWORD
#define pthread_create(thhandle,attr,thfunc,tharg) \ ((thhandle=(HANDLE)_beginthreadex(NULL,0,thfunc,tharg,0, NULL))==NULL)
#define pthread_join(thread) ((WaitForSingleObject((thread),INFINITE)!=WAIT_OBJECT_0) || !CloseHandle(thread))
#define pthread_detach(thread) if(thread!=NULL)CloseHandle(thread)
#define ts_key_create(ts_key, destructor) {ts_key = TlsAlloc();}
#define pthread_getspecific(ts_key) TlsGetValue(ts_key)
#define pthread_setspecific(ts_key, value) TlsSetValue(ts_key, (void *)value)
#define pthread_self() GetCurrentThreadId()
#define THREAD_FUNCTION void *
#define THREAD_FUNCTION_RETURN void *
#define THREAD_SPECIFIC_INDEX pthread_key_t
#define ts_key_create(ts_key, destructor) pthread_key_create (&(ts_key), destructor);

/* Syncrhronization macros - Assume NT 4.0 or 2000 */
#define _WIN32_WINNT 0x400 // WINBASE.H
#define pthread_mutex_t HANDLE
#define pthread_condvar_t HANDLE
#define pthread_mutex_lock(object) WaitForSingleObject(object, INFINITE)
#define pthread_mutex_unlock(object) ReleaseMutex(object)
#define CV_TIMEOUT 100 /* Arbitrary value */

/* USE THE FOLLOWING FOR WINDOWS 9X */
//#define cond_var_wait(cv,mutex) {ReleaseMutex(mutex);WaitForSingleObject(cv,CV_TIMEOUT);WaitForSingleObject(mutex,INFINITE);};*/
#define pthread_cond_wait(cv,mutex) {SignalObjectAndWait(mutex,cv,INFINITE,FALSE);WaitForSingleObject(mutex,INFINITE);};
#define pthread_cond_broadcast(cv) PulseEvent(cv)

#endif // _WINDOWS
#endif // _THREAD_EMULATION
