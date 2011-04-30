#ifndef _SG_VERTEXANDINDEXLOCK_H
#define _SG_VERTEXANDINDEXLOCK_H


#include <sg/IndexLock.h>
#include <sg/VertexLock.h>


namespace sg
{


/** 
 * Locks object vertex and index data. 
 * The object class must implement lockVertices(), lockIndices(),
 * unlockVertices() and unlockIndices() methods.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class VertexAndIndexLock
{
public:
	/** Locks object vertex and index data with specified lock mode. */
	VertexAndIndexLock( T* obj, T::LockType lock ) :
		m_indexLock(obj,lock), m_vertexLock(obj,lock)
	{
	}

	/** Unlocks object vertex and index data. */
	~VertexAndIndexLock()
	{
	}

private:
	IndexLock<T>	m_indexLock;
	VertexLock<T>	m_vertexLock;

	VertexAndIndexLock();
	VertexAndIndexLock( const VertexAndIndexLock& );
	VertexAndIndexLock& operator=( const VertexAndIndexLock& );
};


} // sg


#endif // _SG_VERTEXANDINDEXLOCK_H
