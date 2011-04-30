#ifndef _LANG_OBJECT_H
#define _LANG_OBJECT_H


#include <lang/Ptr.h>


namespace lang
{


class Mutex;


/**
 * Provides functionality for basic synchronization and 
 * thread-safe reference counting.
 * If an Object is created to stack then
 * special care must be taken that the object
 * reference count is not affected anywhere.
 *
 * Usage example:
 * <pre>
	class Hello : public Object { ... };

	void f()
	{
		P(Hello) hello = new Hello;
		hello->printMessage();
		// hello goes out of scope and gets deleted
	}
	</pre>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Object
{
public:
	/* Helper class for synchronizing access to an Object. */
	class MutexLock { Mutex* m; MutexLock(); MutexLock( const MutexLock& ); MutexLock& operator=( const MutexLock& ); public: MutexLock( const Object& o ); MutexLock( const Object* o ); ~MutexLock();};

	/** Object creation flags. */
	enum ObjectFlags
	{
		/** Do not initialize synchronization objects. */
		OBJECT_DEFAULT			= 0,
		/** Initialize synchronization object. */
		OBJECT_INITMUTEX		= 1
	};

	/** 
	 * Initializes reference count to zero. 
	 * @param flags Object creation flags.
	 */
	Object( int flags=OBJECT_DEFAULT );

	/** Initializes reference count to zero. */
	Object( const Object& );

	/** Ensures that the reference count is zero. */
	virtual ~Object();

	/** Allocates an Object. */
	void*			operator new( unsigned n );

	/** Allocates an Object with debug information. */
	void*			operator new( unsigned n, const char* file, int line );

	/** Frees the Object. */
	void			operator delete( void* p );

	/** Frees the Object with debug information. */
	void			operator delete( void* p, const char* file, int line );

	/** Returns this Object. */
	Object&			operator=( const Object& );

	/** 
	 * Initializes object specific synchronization object.
	 * Calling initMutex() is identical to passing OBJECT_INITMUTEX
	 * to Object constructor. initMutex() should be used when it is not feasible
	 * to modify arguments passed to Object ctor, for example when
	 * adding synchronization to a derived class.
	 */
	void			initMutex();

	/** Increments reference count by one. */
	void			addReference();

	/** 
	 * Decrements reference count by one and 
	 * deletes the object if the count reaches zero. 
	 */
	void			release();

private:
	friend class MutexLock;
	long	m_refs;
	Mutex*	m_mutex;
};


} // lang


/**
 * Acquired exclusive access to the Object and releases it at the end of the scope.
 * Object needs to have been either created with flag OBJECT_INITMUTEX set
 * or initMutex() must have been called afterwards, before using synchronized().
 *
 * Usage example:
 * <pre>
	class Hello : public Object { ... };

	void Hello::world() const
	{
		synchronized( this );
	}
	</pre>
 */
#define synchronized( OBJ )		lang::Object::MutexLock mutexLock( OBJ )

/** Smart pointer to an object. */
#define P( CLASS ) lang::Ptr< CLASS >

// Debug global memory allocation
#ifdef _DEBUG
void*	operator new( unsigned n );
void*	operator new( unsigned n, const char* file, int line );
void	operator delete( void* p );
void	operator delete( void* p, const char* file, int line );
#define LANG_DEBUG_NEW new(__FILE__,__LINE__)
#endif


#endif // _LANG_OBJECT_H
