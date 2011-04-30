#include "Object.h"
#include "Mutex.h"
#include <mem/raw.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


Object::MutexLock::MutexLock( const Object& o )
{
	assert( o.m_mutex );	// initMutex has not been called

	m = o.m_mutex;
	m->lock();
}

Object::MutexLock::MutexLock( const Object* o )
{
	assert( o->m_mutex );	// initMutex has not been called

	m = o->m_mutex;
	m->lock();
}

Object::MutexLock::~MutexLock()
{
	m->unlock();
}

//-----------------------------------------------------------------------------

Object::Object( int flags ) :
	m_refs(0), m_mutex(0)
{
	if ( flags & OBJECT_INITMUTEX )
		initMutex();
}

Object::Object( const Object& ) :
	m_refs(0), m_mutex(0)
{
}

Object::~Object()																		
{
	assert( 0 == m_refs );

	if ( m_mutex ) 
	{
		delete m_mutex;
		m_mutex = 0;
	}
}

Object& Object::operator=( const Object& )
{
	return *this;
}

void Object::initMutex()
{
	if ( !m_mutex )
		m_mutex = new Mutex;
}

void Object::addReference()
{
	assert( m_refs >= 0 );
	
	Mutex::incrementRC( &m_refs );
}

void Object::release()
{
	assert( m_refs > 0 );
	
	if ( 0 == Mutex::decrementRC(&m_refs) )
		delete this;
}

#ifdef _DEBUG
#undef new
#endif

void* Object::operator new( unsigned n )
{
	return mem_allocate( n, __FILE__, __LINE__ );
}

void* Object::operator new( unsigned n, const char* file, int line )
{
	return mem_allocate( n, file, line );
}

void Object::operator delete( void* p )
{
	mem_free( p );
}

void Object::operator delete( void* p, const char*, int )
{
	mem_free( p );
}


} // lang


// Global memory allocation (_DEBUG only)
#ifdef _DEBUG
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
