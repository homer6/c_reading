#ifndef _UTIL_VECTOR_H
#define _UTIL_VECTOR_H


#include <lang/Object.h>
#include <util/Allocator.h>
#include <assert.h>


namespace util
{


/** 
 * Dynamic array. Vector is not synchronized.
 * Maintains correct constructor/destructor calling semantics,
 * for example setSize( size()-1 ) causes destructor
 * to be called for the last element in the vector.
 * @param T Contained type.
 * @param A Allocator of T.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T, class A=Allocator<T> > class Vector : 
	public lang::Object
{
public:
	/** Constructs an empty vector. */
	explicit Vector( const A& alloc );

	/** Copy by value. */
	Vector( const Vector<T,A>& other );

	/** Releases vector memory if no more references left. */
	~Vector();

	/** Copy by value. */
	Vector<T,A>& operator=( const Vector<T,A>& other );

	/** Return ith element in the vector. */
	T&			operator[]( int index )												{assert( index >= 0 && index < m_len ); return m_data[index];}

	/** Return ith element in the vector. */
	T&			get( int index )													{assert( index >= 0 && index < m_len ); return m_data[index];}

	/** Sets new length of the vector. */
	void		setSize( int length );

	/** Sets new length of the vector and initializes new elements with specified value. */
	void		setSize( int length, const T& value );

	/** Adds an element at the end of the vector. */
	void		add( const T& item );

	/** Inserts an element at specified index in the vector. */
	void		add( int index, const T& item );

	/** Removes a value at specified index. */
	void		remove( int index );

	/** Removes a range between begin (inclusive) and end (exclusive). */
	void		remove( int begin, int end );

	/** Removes all elements from this vector. */
	void		clear();

	/** Trims capacity of this vector to current size. */
	void		trimToSize();

	/** Returns the first element of the vector. */
	T&			firstElement()														{assert( m_len > 0 ); return *m_data;}

	/** Returns the last element of the vector. */
	T&			lastElement()														{assert( m_len > 0 ); return m_data[m_len-1];}

	/** Returns pointer to the first element in the vector. */
	T*			begin()																{return m_data;}

	/** Returns pointer to one beyond the last element in the vector. */
	T*			end()																{return m_data+m_len;}

	/** Return ith element in the vector. */
	const T&	operator[]( int index ) const										{assert( index >= 0 && index < m_len ); return m_data[index];}

	/** 
	 * Returns the first occurence of the item or -1 if not found. 
	 * Starts the search from the beginning of the vector.
	 */
	int			indexOf( const T& item ) const;

	/** 
	 * Returns the first occurence of the item or -1 if not found. 
	 * Starts the search from the specified position.
	 */
	int			indexOf( const T& item, int index ) const;

	/** 
	 * Returns the last occurence of the item or -1 if not found. 
	 * Searches the vector backwards starting at the end.
	 */
	int			lastIndexOf( const T& item ) const;

	/** 
	 * Returns the previous occurence of the item or -1 if not found. 
	 * Searches the vector backwards starting at the specified position.
	 */
	int			lastIndexOf( const T& item, int index ) const;
	
	/** Returns the first element of the vector. */
	const T&	firstElement() const 												{assert( m_len > 0 ); return *m_data;}

	/** Returns the last element of the vector. */
	const T&	lastElement() const 												{assert( m_len > 0 ); return m_data[m_len-1];}

	/** Returns number of elements in the vector. */
	int			size() const														{return m_len;}

	/** Returns true if there is no elements in the vector. */
	bool		isEmpty() const														{return 0 == m_len;}

	/** Return ith element in the vector. */
	const T&	get( int index ) const												{assert( index >= 0 && index < m_len ); return m_data[index];}

	/** Returns pointer to the first element in the vector. */
	const T*	begin() const														{return m_data;}

	/** Returns pointer to one beyond the last element in the vector. */
	const T*	end() const															{return m_data+m_len;}

private:
	T*				m_data;
	int				m_len;
	int				m_cap;
	A				m_alloc;

	void			defaults();
	void			destroy();
	static int		getCapacity( int length );
	static void		rcopy( T* dst, const T* src, int count );
	static void		copy( T* dst, const T* src, int count );
};


#include "Vector.inl"


} // util


#endif // _UTIL_VECTOR_H
