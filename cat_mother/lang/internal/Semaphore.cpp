#include "Semaphore.h"

#ifdef LANG_WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <limits.h>
#else
	#include <semaphore.h>
#endif

#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


class Semaphore::SemaphoreImpl
{
public:
	#ifdef LANG_WIN32
	HANDLE		obj;
	#else
	sem_t		obj;
	#endif

	SemaphoreImpl( int value )
	{
		#ifdef LANG_WIN32
		obj = CreateSemaphore( 0, value, INT_MAX, 0 );
		#else
		sem_init( &obj, 0, value );
		#endif
	}

	~SemaphoreImpl()
	{
		#ifdef LANG_WIN32
		CloseHandle( obj ); 
		#else
		sem_destroy( &obj );
		#endif
	}

	void wait()
	{
		#ifdef LANG_WIN32
		WaitForSingleObject( obj, INFINITE );
		#else
		sem_wait( &obj );
		#endif
	}

	void signal()
	{
		#ifdef LANG_WIN32
		ReleaseSemaphore( obj, 1, 0 );
		#else
		sem_post( &obj );
		#endif
	}
};

//-----------------------------------------------------------------------------

Semaphore::Semaphore( int value )
{
	m_this = new SemaphoreImpl( value );
}

Semaphore::~Semaphore()
{
	delete m_this;
}

void Semaphore::wait()
{
	m_this->wait();
}

void Semaphore::signal()
{
	m_this->signal();
}


} // lang
