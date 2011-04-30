template <class T, class A> Vector<T,A>::Vector( const A& alloc ) :
	m_alloc(alloc)
{
	defaults();
}

template <class T, class A> Vector<T,A>::Vector( const Vector<T,A>& other ) :
	m_alloc(other.m_alloc)
{
	defaults();
	if ( other.m_len > 0 )
	{
		setSize( other.m_len );
		copy( m_data, other.m_data, m_len );
	}
}

template <class T, class A> Vector<T,A>::~Vector()
{
	destroy();
}

template <class T, class A> Vector<T,A>& Vector<T,A>::operator=( const Vector<T,A>& other )
{
	if ( this != &other )
	{
		m_alloc = other.m_alloc;
		setSize( other.m_len );
		if ( other.m_len > 0 )
			copy( m_data, other.m_data, m_len );
	}
	return *this;
}

template <class T, class A> void Vector<T,A>::setSize( int length, const T& value )
{
	T* data = 0;
	int cap = 0;

	assert( length >= 0 );

	try
	{
		if ( length <= m_len )
		{
			while ( m_len > length )
				m_alloc.destroy( m_data + --m_len );
		}
		else if ( length <= m_cap )
		{
			while ( m_len < length )
				m_alloc.construct( m_data + m_len++, value );
		}
		else // length > m_cap
		{
			assert( length > m_cap );
			
			cap = getCapacity( length );
			if ( cap < m_cap*2 )
				cap = m_cap*2;

			data = m_alloc.allocate( cap, 0 );
			copy( data, m_data, m_len );
			int i;
			for ( i = m_len ; i < length ; ++i )
				data[i] = value;
			destroy();

			for ( i = length ; i < cap ; ++i )
				m_alloc.destroy( data+i );

			m_data = data;
			m_len = length;
			m_cap = cap;
		}
	}
	catch ( ... )
	{
		m_alloc.deallocate( data, cap );
		throw;
	}
}

template <class T, class A> void	Vector<T,A>::setSize( int length )
{
	setSize( length, T() );
}

template <class T, class A> void Vector<T,A>::clear()																
{
	setSize( 0 );
}

template <class T, class A> int Vector<T,A>::getCapacity( int length )
{
	int cap = length+1;
	cap += 15;
	cap &= ~15;
	assert( cap >= length );
	return cap;
}

template <class T, class A> void Vector<T,A>::rcopy( T* dst, const T* src, int count )
{
	for ( int i = count-1 ; i >= 0 ; --i )
		dst[i] = src[i];
}

template <class T, class A> void Vector<T,A>::copy( T* dst, const T* src, int count )
{
	T* dst1 = dst + count;
	while ( dst != dst1 )
		*dst++ = *src++;
}

template <class T, class A> void Vector<T,A>::add( const T& item )												
{
	int len = m_len; 
	setSize( m_len + 1 ); 
	m_data[len] = item;
}

template <class T, class A> void Vector<T,A>::add( int index, const T& item )
{
	assert( index >= 0 && index <= size() );
	T itemCopy( item );
	setSize( m_len + 1 );
	rcopy( m_data+(index+1), m_data+index, m_len-(index+1) );
	m_data[index] = itemCopy;
}

template <class T, class A> void Vector<T,A>::remove( int index )
{
	assert( index >= 0 && index < m_len );
	copy( m_data+index, m_data+(index+1), m_len-(index+1) );
	m_alloc.destroy( m_data+m_len-1 );
	--m_len;
}

template <class T, class A> void Vector<T,A>::remove( int begin, int end )
{
	assert( begin >= 0 && (begin < m_len || begin == end) );
	assert( end >= 0 && end <= m_len );
	assert( begin <= end );

	int count = end - begin;
	copy( m_data+begin, m_data+end, m_len-end );
	for ( int i = begin ; i < end ; ++i )
		m_alloc.destroy( m_data+i );
	m_len -= count;
}

template <class T, class A> void Vector<T,A>::trimToSize()
{
	T* tmp = 0;

	try
	{
		if ( m_len > 0 )
			tmp = new T[m_len];
		copy( tmp, m_data, m_len );
		destroy();
		
		m_data	= tmp;
		m_cap	= m_len;
	}
	catch ( ... )
	{
		delete[] tmp;
		throw;
	}
}

template <class T, class A> void Vector<T,A>::defaults()
{
	m_data = 0;
	m_len	= 0;
	m_cap	= 0;
}

template <class T, class A> void Vector<T,A>::destroy()
{
	while ( m_len < m_cap )
		m_alloc.construct( m_data + m_len++, T() );

	m_alloc.deallocate( m_data, m_cap );
	m_data = 0;
	m_cap = 0;
	m_len = 0;
}

template <class T, class A> int Vector<T,A>::indexOf( const T& item ) const
{
	for ( int index = 0 ; index < m_len ; ++index )
		if ( m_data[index] == item )
			return index;
	return -1;
}

template <class T, class A> int Vector<T,A>::indexOf( const T& item, int index ) const
{
	assert( index >= 0 && index < m_len );
	for ( ; index < m_len ; ++index )
		if ( m_data[index] == item )
			return index;
	return -1;
}

template <class T, class A> int Vector<T,A>::lastIndexOf( const T& item ) const
{
	for ( int index = m_len-1 ; index >= 0 ; --index )
		if ( m_data[index] == item )
			return index;
	return -1;
}

template <class T, class A> int Vector<T,A>::lastIndexOf( const T& item, int index ) const
{
	assert( index >= 0 && index < m_len );
	for ( ; index >= 0 ; --index )
		if ( m_data[index] == item )
			return index;
	return -1;
}
