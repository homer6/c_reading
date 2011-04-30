#ifndef _LANG_SEMAPHORE_H
#define _LANG_SEMAPHORE_H


namespace lang
{


/** 
 * Semaphore synchronization object. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Semaphore
{
public:
	/** Constructs semaphore with specified initial value. */
	Semaphore( int value=0 );

	///
	~Semaphore();

	/** Waits for the semaphore (P primitive). */
	void	wait();

	/** Signals the semaphore (V primitive). */
	void	signal();

private:
	class SemaphoreImpl;
	SemaphoreImpl* m_this;

	Semaphore( const Semaphore& );
	Semaphore& operator=( const Semaphore& );
};


} // lang


#endif // _LANG_SEMAPHORE_H
