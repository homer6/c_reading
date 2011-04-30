template <class K, class T> HashtableIterator<K,T>::HashtableIterator( HashtablePair<K,T>* data, int cap, int index )
{
	assert( index >= 0 && index <= cap ); 
	m_data = data;
	m_last = cap-1; 
	m_index = index-1; 
	m_item = 0; 
	this->operator++();
}

template <class K, class T> HashtableIterator<K,T>&	HashtableIterator<K,T>::operator++()
{
	do 
	{
		if ( m_item )
			m_item = m_item->next;
		if ( !m_item && m_index < m_last )
			m_item = &m_data[ ++m_index ];
	} while ( m_item && !m_item->used );
	return *this;
}
