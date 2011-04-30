template <class T> Allocator<T>::Allocator()
{
	m_group = mem_Group_create(__FILE__,__LINE__);
}

template <class T> Allocator<T>::Allocator( const char* file, int line )
{
	m_group = mem_Group_create(file,line);
}

template <class T> Allocator<T>::~Allocator()
{
	mem_Group_release(m_group);
}

template <class T> Allocator<T>::Allocator( const Allocator<T>& other )
{
	m_group = mem_Group_copy( other.m_group );
}

template <class T> Allocator<T>& Allocator<T>::operator=( const Allocator<T>& other )
{
	void* group = mem_Group_copy( other.m_group );
	mem_Group_release(m_group);
	m_group = group;
	return *this;
}

template <class T> T* Allocator<T>::allocate( int n, void* )
{
	void* mem = mem_Group_allocate( m_group, sizeof(T)*n );
	T* item0 = reinterpret_cast<T*>(mem); 
	int i = 0;

	try
	{
		T* item = item0; 
		for ( ; i < n ; ++i ) 
		{
			void* itemmem = item; 
			::new(itemmem) T;
			++item;
		} 
		return item0;
	}
	catch ( ... )
	{
		--i;
		T* item = item0 + i;
		for ( ; i >= 0 ; --i ) 
		{
			item->~T();
			--item;
		} 
		mem_Group_free( m_group, mem, sizeof(T)*n );
		throw;
	}
}

template <class T> void Allocator<T>::deallocate( T* p, int n )
{
	void* mem = p;
	p += n-1;
	for ( int i = 0 ; i < n ; ++i ) 
	{
		p->~T(); 
		--p;
	}
	mem_Group_free( m_group, mem, sizeof(T)*n );
}

template <class T> T* Allocator<T>::construct( void* p, const T& v )
{
	return ::new(p) T(v);
}
    
template <class T> void Allocator<T>::destroy( T* p )
{
	p->~T(); 
	p = p;
}

template <class T> const char* Allocator<T>::name() const
{
	return mem_Group_name( m_group );
}

template <class T> int Allocator<T>::bytesInUse() const
{
	return mem_Group_bytesInUse( m_group );
}

template <class T> int Allocator<T>::blocksInUse() const
{
	return mem_Group_blocksInUse( m_group );
}
