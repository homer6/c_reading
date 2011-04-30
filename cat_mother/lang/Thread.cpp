#include "Thread.h"
#include "Thread_impl.h"
#include "Throwable.h"
#include "Exception.h"
#include "Semaphore.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


class ThreadImpl :
	public Object
{
public:
	Thread_t	handle;
	P(Thread)	obj;
	bool		running;

	ThreadImpl()
	{
		handle	= 0;
		obj		= 0;
		running	= false;
	}

private:
	ThreadImpl( const ThreadImpl& );
	ThreadImpl& operator=( const ThreadImpl& );
};

//-----------------------------------------------------------------------------

static void* runThread( void* arg )
{
	P(ThreadImpl) thread = reinterpret_cast<ThreadImpl*>(arg);
	assert( thread->obj );
	assert( thread->running );

	try 
	{
		thread->obj->run();
	}
	catch ( ... )
	{
	}

	thread->running = false;
	thread->obj = 0;
	thread = 0;
	return 0;
}

//-----------------------------------------------------------------------------

Thread::Thread()
{
	m_this = new ThreadImpl;
}

Thread::~Thread()
{
	assert( !m_this->running );

	if ( m_this->handle )
	{
		Thread_detach( m_this->handle );
		m_this->handle = 0;
	}
}

void Thread::start()
{
	assert( !m_this->running );

	if ( m_this->handle )
	{
		Thread_detach( m_this->handle );
		m_this->handle = 0;
	}

	m_this->obj = this;
	m_this->running = true;
	if ( Thread_create( &m_this->handle, runThread, m_this ) )
	{
		m_this->obj = 0;
		m_this->running = false;
		throw Exception( Format("Failed to create a thread.") );
	}
}

void Thread::join()
{
	if ( m_this->handle )
	{
		Thread_join( m_this->handle );
		m_this->handle = 0;
	}
}

void Thread::run()
{
	assert( false );	// Thread is used by subclassing
}

void Thread::sleep( long millis )
{
	Thread_sleep( millis );
}


} // lang
