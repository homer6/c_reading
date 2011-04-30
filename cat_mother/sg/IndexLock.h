#ifndef _SG_INDEXLOCK_H
#define _SG_INDEXLOCK_H


#include <lang/Object.h>


namespace sg
{


/** 
 * Locks object index data. 
 * The object class must implement lockIndices() and 
 * unlockIndices() methods.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class IndexLock
{
public:
	/** Locks object index data with specified lock mode. */
	IndexLock( T* obj, T::LockType lock ) :
		m_obj(obj)
	{
		obj->lockIndices( lock );
	}

	/** Unlocks object index data. */
	~IndexLock()
	{
		m_obj->unlockIndices();
	}

private:
	T* m_obj;	// note! this must be weak pointer because otherwise all models must be allocated to heap

	IndexLock();
	IndexLock( const IndexLock& );
	IndexLock& operator=( const IndexLock& );
};


} // sg


#endif // _SG_INDEXLOCK_H
