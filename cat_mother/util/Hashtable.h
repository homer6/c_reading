#ifndef _UTIL_HASHTABLE_H
#define _UTIL_HASHTABLE_H


#include <lang/Object.h>
#include <util/Hash.h>
#include <util/Equal.h>
#include <util/Allocator.h>
#include <util/HashtableIterator.h>
#include <util/internal/HashtablePair.h>

namespace util
{


/** 
 * Hashtable maps keys to values. 
 * Default hash function uses key hashCode() for hashing.
 * Default compare function uses key operator==.
 * @param K Key type.
 * @param T Data type.
 * @param F Key hash function type.
 * @param E Key equality compare function type.
 * @param A Allocator of HashtablePair<K,T>.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template < class K, class T, class F=Hash<K>, class E=Equal<K>, class A=Allocator< HashtablePair<K,T> > >
class Hashtable :
	public lang::Object
{
public:
	/** Constructs an empty hash table with load factor of 75/100. */
	explicit Hashtable( const A& alloc );

	/** 
	 * Constructs an empty hash table with specified 
	 * initial capacity, load factor, default value and hash function.
	 */
	explicit Hashtable( int initialCapacity, float loadFactor, 
		const T& defaultValue, const F& hashFunc, const E& equalFunc,
		const A& alloc );

	/** Copy by value. */
	Hashtable( const Hashtable<K,T,F,E,A>& other );

	///
	~Hashtable();

	/** Copy by value. */
	Hashtable<K,T,F,E,A>& operator=( const Hashtable<K,T,F,E,A>& other );

	/** Returns the value of specified key. Puts the key to the map if not exist. */
	T&			operator[]( const K& key );

	/** 
	 * Returns the value of specified key. 
	 * Returns default value if the key was not in the table.
	 */
	T&			get( const K& key );

	/** 
	 * Puts the value at specified key to the container. 
	 * If there is already old value with the same key it is overwritten.
	 */
	void		put( const K& key, const T& value );

	/** 
	 * Removes value at specified key. 
	 * Does nothing if the key is not in the table.
	 */
	void		remove( const K& key );

	/** Removes all keys from the container. */
	void		clear();

	/** Returns the value of specified key. Puts the key to the map if not exist. */
	const T&	operator[]( const K& key ) const;

	/** 
	 * Returns the value of specified key. 
	 * Returns default value if the key is not in the table.
	 */
	const T&	get( const K& key ) const;

	/** Returns number of distinct keys. */
	int			size() const;

	/** Returns true if there is no elements in the table. */
	bool		isEmpty() const;

	/** Returns true if the hash table contains specific key. */
	bool		containsKey( const K& key ) const;

	/** 
	 * Returns number of collisions occured in the hash table.
	 * Can be used for debugging hash functions.
	 */
	int			collisions() const;

	/** 
	 * Returns iterator to the first element. 
	 * Behaviour is undefined if the table is changed during the traversal.
	 */
	HashtableIterator<K,T>	begin() const;

	/** 
	 * Returns iterator to one beyond the last element. 
	 * Behaviour is undefined if the table is changed during the traversal.
	 */
	HashtableIterator<K,T>	end() const;

private:
	int						m_cap;
	HashtablePair<K,T>*		m_data;
	float					m_loadFactor;
	mutable int				m_entries;
	int						m_entryLimit;
	T						m_defaultValue;
	mutable int				m_collisions;
	F						m_hashFunc;
	E						m_equalFunc;
	mutable A				m_alloc;

	void				defaults();
	void				destroy();
	void				grow();
	void				rehashTo( HashtablePair<K,T>* data, int cap ) const;
	HashtablePair<K,T>* getPair( HashtablePair<K,T>* data, int cap, const K& key ) const;
	HashtablePair<K,T>*	allocateTable( int cap ) const;
	void				deallocateTable( HashtablePair<K,T>* data, int cap ) const;
};


#include "Hashtable.inl"


} // util


#endif // _UTIL_HASHTABLE_H
