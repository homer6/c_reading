#ifndef _MEM_ALLOCATOR_H
#define _MEM_ALLOCATOR_H


#include <mem/Group.h>
#include <new>


namespace util
{


/** 
 * Manager for storage allocation of objects of type T. 
 * Supports named memory block groups (for memory statistics).
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class Allocator
{
public:
	/** Creates allocator. */
	Allocator();

	/** Creates allocator of specified user. */
	explicit Allocator( const char* file, int line=-1 );

	/** Releases allocator. */
	~Allocator();

	/** Copy by reference. */
	Allocator( const Allocator<T>& other );

	/** Copy by reference. */
	Allocator<T>&	operator=( const Allocator<T>& other );

	/** Allocates and constructs n objects. */
	T*				allocate( int n, void* hint=0 );

	/** Destructs and deallocates n objects. */
	void			deallocate( T* p, int n );

	/** Constructs object to uninitialized memory. */
	T*				construct( void* p, const T& v );
	
	/** Destructs object. */
	void			destroy( T* p );

	/** Returns number of bytes memory allocated in the group. */
	int 			bytesInUse() const;

	/** Returns number of memory blocks allocated in the group. */
	int 			blocksInUse() const;

	/** Returns name of the allocator group. */
	const char* 	name() const;

private:
	void* m_group;
};


#include "Allocator.inl"


} // util


#endif // _MEM_ALLOCATOR_H
