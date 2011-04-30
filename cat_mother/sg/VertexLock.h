#ifndef _SG_VERTEXLOCK_H
#define _SG_VERTEXLOCK_H


#include <lang/Object.h>


namespace sg
{


/** 
 * Locks object vertex data. 
 * The object class must implement lockVertices() and 
 * unlockVertices() methods.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class VertexLock
{
public:
	/** Locks object vertex data with specified lock mode. */
	VertexLock( T* obj, T::LockType lock ) :
		m_obj(obj)
	{
		obj->lockVertices( lock );
	}

	/** Unlocks object vertex data. */
	~VertexLock()
	{
		m_obj->unlockVertices();
	}

private:
	T* m_obj;	// note! this must be weak pointer because otherwise all models must be allocated to heap

	VertexLock();
	VertexLock( const VertexLock& );
	VertexLock& operator=( const VertexLock& );
};


} // sg


#endif // _SG_VERTEXLOCK_H
