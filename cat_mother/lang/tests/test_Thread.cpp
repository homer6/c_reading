#include <tester/Test.h>
#include <lang/Thread.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

class TestThread :
	public Thread
{
public:
	TestThread( const char* name, long* value, long count, Object* mutex )
	{
		strcpy( m_name, name );
		m_value = value;
		m_count = count;
		m_mutex = mutex;
	}

	~TestThread()
	{
	}

	void run()
	{
		for ( int loop = 20 ; loop > 0 ; --loop )
		{
			synchronized( m_mutex );

			for ( long i = 0 ; i < m_count ; ++i )
				*m_value += 1;
		}
	}

private:
	char	m_name[256];
	long*	m_value;
	long	m_count;
	Object*	m_mutex;

	TestThread( const TestThread& );
	TestThread& operator=( const TestThread& );
};

//-----------------------------------------------------------------------------

static int test_Thread1()
{
	Object mutex( Object::OBJECT_INITMUTEX );
	long v = 0;
	P(TestThread) thread1 = new TestThread( "thread1", &v, 1000, &mutex );
	P(TestThread) thread2 = new TestThread( "thread2", &v, 2000, &mutex );
	P(TestThread) thread3 = new TestThread( "thread3", &v, 3000, &mutex );
	P(TestThread) thread4 = new TestThread( "thread4", &v, 4000, &mutex );
	P(TestThread) thread5 = new TestThread( "thread5", &v, 5000, &mutex );
	P(TestThread) thread6 = new TestThread( "thread6", &v, 6000, &mutex );
	P(TestThread) thread7 = new TestThread( "thread7", &v, 7000, &mutex );
	thread1->start();
	thread2->start();
	thread3->start();
	thread4->start();
	thread5->start();
	thread6->start();
	thread4->join();
	thread4->start();
	thread4->join();
	// now running: 1,2,3,5,6
	thread5->join();
	thread6->join();
	thread7->join();			// not started but should not cause problems
	Thread::sleep( 1000 );
	// now running: 1,2,3
	thread1->join();
	thread2->join();
	thread3->join();
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg1( test_Thread1, __FILE__ );
