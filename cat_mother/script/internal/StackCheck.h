#include <script/VM.h>
#include <assert.h>


namespace script
{


/** 
 * Check that stack size has changed specified number of items at the end of scope. 
 * _DEBUG build only.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class StackCheck
{
public:
	enum Flags
	{
		DESTRUCTOR_CHECK,
		EXPLICIT_CHECK
	};

#ifdef _DEBUG

	StackCheck( VM* vm, int items, const char* filename, int opt=DESTRUCTOR_CHECK ) :
		m_state(vm), m_items(items), m_filename(filename), m_opt(opt), m_top(vm->top())
	{
	}

	~StackCheck()
	{
		if ( DESTRUCTOR_CHECK == m_opt )
			check();
	}

	void check()
	{
		int top = m_state->top();
		assert( m_top+m_items == top ); top=top;
	}

#else

	StackCheck( VM*, int, const char* )
	{
	}

	StackCheck( VM*, int, const char*, int )
	{
	}

	~StackCheck()
	{
	}

	void check()
	{
	}

#endif
private:
#ifdef _DEBUG
	
	VM*	m_state;
	int				m_items;
	const char*		m_filename;
	int				m_opt;
	int				m_top;

#endif

	StackCheck( const StackCheck& );
	StackCheck& operator=( const StackCheck& );
};


} // script
