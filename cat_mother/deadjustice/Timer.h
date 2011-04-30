#ifndef _TIMER_H
#define _TIMER_H


/** 
 * Stepped timer for updating the game with fixed intervals. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Timer
{
public:
	/** 
	 * Initializes timer with specified (sec) step size. 
	 * @param dt Timer update step size. (seconds)
	 */
	explicit Timer( float dt );

	/** Begins to update specified amount of (frame) time. */
	void	beginUpdate( float frameTime );

	/** Returns true if there is any interval to update. */
	bool	update();

	/** Returns update interval in seconds. */
	float	dt() const;

	/** Returns elapsed time in seconds. */
	float	time() const;

private:
	float	m_dt;
	float	m_time;
	float	m_frameTime;	// from beginUpdate, decreased by update() in dt() steps
};


#endif // _TIMER_H
