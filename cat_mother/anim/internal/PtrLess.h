/** 
 * Compares array objects with less than operator. 
 * Used for sorting index list.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class PtrLess
{
public:
	PtrLess( const T* array ) :
		m_array(array)
	{
	}

	bool operator()( int a, int b ) const
	{
		return m_array[a] < m_array[b];
	}

private:
	const T* m_array;
};
