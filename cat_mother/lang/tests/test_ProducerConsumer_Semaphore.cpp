#include <tester/Test.h>
#include <lang/Thread.h>
#include <lang/Array.h>
#include <lang/internal/Semaphore.h>
#include <util/Random.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef _MSC_VER
#include <config_msvc.h>
#endif

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

static Array<int>		s_flow;
static util::Random		s_rng;

//-----------------------------------------------------------------------------

namespace sem
{

class Stack :
	public Object
{
public:
	enum { MAX_ITEMS = 4 };

	Stack() :
		Object( OBJECT_INITMUTEX ),
		m_notFull(MAX_ITEMS), m_notEmpty(0), m_count(0)
	{
		m_db.setSize( MAX_ITEMS );
	}

	void push( int x, int id )
	{
		m_notFull.wait();
		assert( m_count >= 0 && m_count < MAX_ITEMS );
		{synchronized(this); m_db[m_count++] = x;
		s_flow.add(id); s_flow.add(x);}
		assert( m_count > 0 && m_count <= MAX_ITEMS );
		m_notEmpty.signal();
	}

	int pop( int id )
	{
		int x = -1;
		m_notEmpty.wait();
		assert( m_count > 0 && m_count <= MAX_ITEMS );
		{synchronized(this); x = m_db[--m_count];
		s_flow.add(-id); s_flow.add(x);}
		assert( m_count >= 0 && m_count < MAX_ITEMS );
		m_notFull.signal();
		return x;
	}

	int size() const
	{
		return m_count;
	}

private:
	Semaphore		m_notFull;
	Semaphore		m_notEmpty;
	int				m_count;
	Array<int>		m_db;

	Stack( const Stack& );
	Stack& operator=( const Stack& );
};

class Producer :
	public Thread
{
public:
	Producer( Stack* s, int x ) :
		m_s(s), m_x(x)
	{
		static int producers = 0;
		m_id = ++producers;
	}

	~Producer()
	{
	}

	void run()
	{
		for ( int i = 0 ; i < 10 ; ++i )
		{
			int x = i + m_x;
			m_s->push( x, m_id );
			sleep( (s_rng.nextInt() & 0xFF)+1 );
		}
	}

private:
	Stack*	m_s;
	int		m_x;
	int		m_id;

	Producer( const Producer& );
	Producer& operator=( const Producer& );
};

class Consumer :
	public Thread
{
public:
	Consumer( Stack* s ) :
		m_s(s)
	{
		static int consumers = 1;
		m_id = ++consumers;
	}

	~Consumer()
	{
	}

	void run()
	{
		for ( int i = 0 ; i < 10 ; ++i )
		{
			int x = m_s->pop(m_id);
			x = x;
		}
	}

private:
	Stack*	m_s;
	int		m_id;

	Consumer( const Consumer& );
	Consumer& operator=( const Consumer& );
};

} using namespace sem;

//-----------------------------------------------------------------------------

static int test()
{
	enum { THREADS = 5 };

	printf( "  Producer/Consumer test (using semaphores):\n" );
	P(Stack) s = new Stack;
	P(Thread) a[THREADS];
	P(Thread) b[THREADS];
	for ( int i = 0 ; i < THREADS ; ++i )
	{
		a[i] = new Producer( s, i*10 );
		a[i]->start();
		b[i] = new Consumer( s );
		b[i]->start();
	}
	for ( int i = 0 ; i < THREADS ; ++i )
	{
		a[i]->join();
		b[i]->join();
	}

	// results
	for ( int i = 0 ; i < s_flow.size() ; i += 2 )
	{
		int id = s_flow[i];
		int x = s_flow[i+1];
		if ( id > 0 )
			printf( "  P%i produced %i\n", id, x );
		else
			printf( "  C%i consumed %i\n", -id, x );
	}
	printf( "  s->size() == %i\n", s->size() );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg1( test, __FILE__ );
