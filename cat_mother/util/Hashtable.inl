int Hashtable_getLargerInt( int n );

//-----------------------------------------------------------------------------

template <class K, class T, class F, class E, class A> Hashtable<K,T,F,E,A>::Hashtable( const A& alloc ) :
	m_alloc(alloc)
{
	defaults();
}

template <class K, class T, class F, class E, class A> Hashtable<K,T,F,E,A>::Hashtable( 
	int initialCapacity, float loadFactor, 
	const T& defaultValue, const F& hashFunc, const E& equalFunc, const A& alloc ) :
	m_alloc(alloc)
{
	assert( loadFactor >= 0.01f && loadFactor <= 0.99f );
	assert( initialCapacity > 0 );

	defaults();
	m_cap = Hashtable_getLargerInt( initialCapacity );
	m_data = allocateTable( m_cap );
	m_loadFactor = loadFactor;
	m_entries = 0;
	m_entryLimit = (int)(m_cap * m_loadFactor);
	m_defaultValue = defaultValue;
	m_hashFunc = hashFunc;
	m_equalFunc = equalFunc;
}

template <class K, class T, class F, class E, class A> Hashtable<K,T,F,E,A>::Hashtable( const Hashtable<K,T,F,E,A>& other ) :
	m_alloc(other.m_alloc)
{
	defaults();
	*this = other;
}

template <class K, class T, class F, class E, class A> Hashtable<K,T,F,E,A>::~Hashtable()
{
	destroy();
}

template <class K, class T, class F, class E, class A> Hashtable<K,T,F,E,A>& Hashtable<K,T,F,E,A>::operator=( const Hashtable<K,T,F,E,A>& other )
{
	if ( this != &other )
	{
		destroy();
		m_alloc = other.m_alloc;
		if ( other.m_entries > 0 )
		{
			int cap = other.m_cap;
			HashtablePair<K,T>* data = allocateTable( cap );
			other.rehashTo( data, cap );
			m_cap = cap;
			m_data = data;
			m_loadFactor = other.m_loadFactor;
			m_entries = other.m_entries;
			m_entryLimit = other.m_entryLimit;
			m_defaultValue = other.m_defaultValue;
		}
	}
	return *this;
}

template <class K, class T, class F, class E, class A> T& Hashtable<K,T,F,E,A>::get( const K& key )
{
	HashtablePair<K,T>* pair = getPair( m_data, m_cap, key );
	if ( !pair->used )
	{
		pair->used = true;
		++m_entries;
	}
	return pair->value;
}	

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::put( const K& key, const T& value )
{
	if ( m_entries+1 >= m_entryLimit )
		grow();
	HashtablePair<K,T>* pair = getPair( m_data, m_cap, key );
	pair->value = value;
	if ( !pair->used )
	{
		pair->used = true;
		++m_entries;
	}
}

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::remove( const K& key )
{
	int slot = (m_hashFunc(key) & 0x7FFFFFFF) % m_cap;
	HashtablePair<K,T>* first = &m_data[slot];

	HashtablePair<K,T>* prevPair = 0;
	HashtablePair<K,T>* nextPair = 0;
	for ( HashtablePair<K,T>* pair = first ; pair ; pair = nextPair )
	{
		nextPair = pair->next;

		if ( pair->used && m_equalFunc(pair->key,key) )
		{
			pair->used = false;
			--m_entries;

			if ( pair != first )
			{
				assert( prevPair );
				prevPair->next = pair->next;
				m_alloc.deallocate( pair, 1 );
			}
		}

		prevPair = pair;
	}
}

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::clear()
{
	for ( int i = 0 ; i < m_cap ; ++i )
	{
		HashtablePair<K,T>* nextPair = 0;
		for ( HashtablePair<K,T>* pair = &m_data[i] ; pair ; pair = nextPair )
		{
			nextPair = pair->next;
			if ( pair->used )
			{
				pair->used = false;
				--m_entries;
			}
		}
	}

	assert( 0 == m_entries );
}

template <class K, class T, class F, class E, class A> T& Hashtable<K,T,F,E,A>::operator[]( const K& key )
{
	if ( m_entries+1 >= m_entryLimit )
		grow();
	HashtablePair<K,T>* pair = getPair( m_data, m_cap, key );
	if ( !pair->used )
	{
		pair->used = true;
		++m_entries;
	}
	return pair->value;
}

template <class K, class T, class F, class E, class A> const T& Hashtable<K,T,F,E,A>::get( const K& key ) const
{
	HashtablePair<K,T>* pair = getPair( m_data, m_cap, key );
	if ( !pair->used )
	{
		pair->used = true;
		++m_entries;
	}
	return pair->value;
}

template <class K, class T, class F, class E, class A> bool Hashtable<K,T,F,E,A>::containsKey( const K& key ) const
{
	if ( m_entries > 0 )
	{
		int slot = (m_hashFunc(key) & 0x7FFFFFFF) % m_cap;
		HashtablePair<K,T>* pair = &m_data[slot];
		for ( ; pair ; pair = pair->next )
			if ( pair->used && m_equalFunc(pair->key,key) )
				return true;
	}
	return false;
}

template <class K, class T, class F, class E, class A> bool Hashtable<K,T,F,E,A>::isEmpty() const
{
	return 0 == m_entries;
}

template <class K, class T, class F, class E, class A> int Hashtable<K,T,F,E,A>::size() const
{
	return m_entries;
}

template <class K, class T, class F, class E, class A> const T& Hashtable<K,T,F,E,A>::operator[]( const K& key ) const
{
	if ( m_entries+1 >= m_entryLimit )
		const_cast< Hashtable<K,T,F,E,A>* >(this)->grow();
	HashtablePair<K,T>* pair = getPair( m_data, m_cap, key );
	if ( !pair->used )
	{
		pair->used = true;
		++m_entries;
	}
	return pair->value;
}

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::grow()
{
	int cap = Hashtable_getLargerInt( m_cap );
	HashtablePair<K,T>* data = allocateTable( cap );
	m_collisions = 0;
	
	for ( int i = 0 ; i < m_cap ; ++i )
	{
		HashtablePair<K,T>* nextPair = 0;
		for ( HashtablePair<K,T>* pair = &m_data[i] ; pair ; pair = nextPair )
		{
			nextPair = pair->next;
			if ( pair->used )
			{
				HashtablePair<K,T>* newPair = getPair( data, cap, pair->key );
				newPair->value = pair->value;
				newPair->used = true;
			}
			pair->next = 0;
			if ( m_data+i != pair )
				m_alloc.deallocate( pair, 1 );
		}
	}

	deallocateTable( m_data, m_cap );
	m_cap = cap;
	m_data = data;
	m_entryLimit = (int)( m_cap * m_loadFactor );
}

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::rehashTo( HashtablePair<K,T>* data, int cap ) const
{
	for ( int i = 0 ; i < m_cap ; ++i )
	{
		HashtablePair<K,T>* nextPair = 0;
		for ( HashtablePair<K,T>* pair = &m_data[i] ; pair ; pair = nextPair )
		{
			nextPair = pair->next;
			if ( pair->used )
			{
				HashtablePair<K,T>* newPair = getPair( data, cap, pair->key );
				newPair->value = pair->value;
				newPair->used = true;
			}
		}
	}
}

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::destroy()
{
	if ( m_data )
	{
		deallocateTable( m_data, m_cap );
		defaults();
	}
}

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::defaults()
{
	m_cap			= 0;
	m_data			= 0;
	m_loadFactor	= 0.75f;
	m_entries		= 0;
	m_entryLimit	= 0;
	m_defaultValue	= T();
	m_collisions	= 0;
	m_hashFunc		= F();
}

template <class K, class T, class F, class E, class A> HashtablePair<K,T>* Hashtable<K,T,F,E,A>::getPair( HashtablePair<K,T>* data, int cap, const K& key ) const
{
	int slot = (m_hashFunc(key) & 0x7FFFFFFF) % cap;
	HashtablePair<K,T>* first = &data[slot];
	HashtablePair<K,T>* unused = 0;
	
	for ( HashtablePair<K,T>* pair = first ; pair ; pair = pair->next )
	{
		if ( !pair->used )
			unused = pair;
		else if ( m_equalFunc(pair->key,key) )
			return pair;
	}

	if ( !unused )
	{
		++m_collisions;
		unused = m_alloc.allocate(1);
		unused->used = false;
		unused->next = first->next;
		first->next = unused;
	}
	unused->key = key;
	unused->value = m_defaultValue;
	return unused;
}

template <class K, class T, class F, class E, class A> HashtablePair<K,T>* Hashtable<K,T,F,E,A>::allocateTable( int cap ) const
{
	return m_alloc.allocate( cap );
}

template <class K, class T, class F, class E, class A> void Hashtable<K,T,F,E,A>::deallocateTable( HashtablePair<K,T>* data, int cap ) const
{
	for ( int i = 0 ; i < cap ; ++i )
	{
		HashtablePair<K,T>* nextPair = 0;
		for ( HashtablePair<K,T>* pair = data[i].next ; pair ; pair = nextPair )
		{
			nextPair = pair->next;
			m_alloc.deallocate( pair, 1 );
		}
	}
	m_alloc.deallocate( data, cap );
}

template <class K, class T, class F, class E, class A> int Hashtable<K,T,F,E,A>::collisions() const
{
	return m_collisions;
}

template <class K, class T, class F, class E, class A> HashtableIterator<K,T> Hashtable<K,T,F,E,A>::begin() const
{
	return HashtableIterator<K,T>( m_data, m_cap, 0 );
}

template <class K, class T, class F, class E, class A> HashtableIterator<K,T> Hashtable<K,T,F,E,A>::end() const
{
	return HashtableIterator<K,T>();
}

