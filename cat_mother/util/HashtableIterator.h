#ifndef _UTIL_HASHTABLEITERATOR_H
#define _UTIL_HASHTABLEITERATOR_H


#include <util/internal/HashtablePair.h>
#include <assert.h>


namespace util
{


/** 
 * Forward iterator of Hashtable elements. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class K, class T> class HashtableIterator
{
public:
	HashtableIterator()																{m_data=0; m_last=0; m_index=0; m_item=0;}
	HashtableIterator( HashtablePair<K,T>* data, int cap, int index );

	/** Sets the iterator to point to the next element. */
	HashtableIterator<K,T>&	operator++();

	/** Returns true if the iterators point to the same item. */
	bool			operator==( const HashtableIterator<K,T>& other ) const			{return m_item == other.m_item;}

	/** Returns true if the iterators point to different items. */
	bool			operator!=( const HashtableIterator<K,T>& other ) const			{return m_item != other.m_item;}

	/** Return item key. */
	const K&		key() const														{assert( m_item ); return m_item->key;}

	/** Return item value. */
	T&				value() const													{assert( m_item ); return m_item->value;}

private:
	HashtablePair<K,T>*		m_data;
	int						m_last;
	int						m_index;
	HashtablePair<K,T>*		m_item;
};


#include "HashtableIterator.inl"


} // util


#endif // _UTIL_HASHTABLEITERATOR_H
