#ifndef _LANG_THREAD_H
#define _LANG_THREAD_H


#include <lang/Object.h>


namespace lang
{


class ThreadImpl;


/**
 * Thread of execution in the program.
 * As starting a thread involves reference counting you should
 * always construct classes derived from Thread with operator new.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Thread :
	public lang::Object
{
public:
	///
	Thread();

	///
	~Thread();

	/**
	 * Starts the executing the thread. 
	 * The result is that two threads are running concurrently - 
	 * the current thread (which returns from the call to the start method)
	 * and the other thread (which executed run method of the derived class).
	 * Increments reference count of this object before the thread starts
	 * and decrements reference count after the thread has terminated.
	 * (so the thread object does not need to be referenced by the
	 * user after this method returns)
	 * @exception Exception
	 */
	void	start();

	/**
	 * Waits for the thread to die.
	 */
	void	join();

	/**
	 * Performs actual work of the thread.
	 * Derived classes override this method.
	 * Called by the start() method of Thread class.
	 */ 
	virtual void	run();

	/**
	 * Temporarily cease execution of current thread 
	 * for the specified number of milliseconds.
	 */
	static void		sleep( long millis );

private:
	P(ThreadImpl) m_this;

	Thread( const Thread& );
	Thread& operator=( const Thread& );
};


} // lang


#endif // _LANG_THREAD_H

