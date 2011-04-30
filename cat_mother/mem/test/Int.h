/**
 * Class for testing correct constructor/destructor calling
 * in containers (leaks memory if container ctor/dtor is incorrect)
 */
class Int
{
public:
	Int( int x=0 )
	{
		m_x = new int(x);
	}

	Int( const Int& other )
	{
		m_x = new int(*other.m_x);
	}

	~Int()
	{
		delete m_x;
	}

	Int& operator=( const Int& other )
	{
		*m_x = *other.m_x;
		return *this;
	}
	
	operator int() const {return *m_x;}

	int hashCode() const {return *m_x & ~3;}

private:
	int* m_x;
};
