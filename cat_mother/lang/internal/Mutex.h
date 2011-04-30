#ifndef _LANG_MUTEX_H
#define _LANG_MUTEX_H


namespace lang
{


/** 
 * Mutual exclusion synchronization object. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Mutex
{
public:
	///
	Mutex();

	///
	~Mutex();

	/** Acquires exclusive access to the mutex. */
	void	lock();

	/** Releases exclusive access to the mutex. */
	void	unlock();

	/** Atomic increment operation for reference counting. */
	static void		incrementRC( long* value );

	/** Atomic decrement operation for reference counting. */
	static long		decrementRC( long* value );

	/** Atomic test-and-set operation. */
	static long		testAndSet( long* value, long newValue=1 );

private:
	class MutexImpl;
	MutexImpl* m_this;

	Mutex( const Mutex& );
	Mutex& operator=( const Mutex& );
};


} // lang


#endif // _LANG_MUTEX_H
