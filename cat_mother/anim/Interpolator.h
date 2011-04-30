#ifndef _ANIM_INTERPOLATOR_H
#define _ANIM_INTERPOLATOR_H


#include <anim/Control.h>
#include <util/Vector.h>


namespace lang {
	class String;}


namespace anim
{


/** 
 * Base class for key-frame animations. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Interpolator :
	public Control
{
public:
	/** 
	 * End behaviour of the key frame animation.
	 */
	enum BehaviourType
	{
		/** Animation is reset to the beginning when it reaches the end. */
		BEHAVIOUR_RESET,
		/** Animation is stopped to the last frame when it reaches the end. */
		BEHAVIOUR_CONSTANT,
		/** Animation loops to the beginning when it reaches the end. */
		BEHAVIOUR_REPEAT,
		/** Animation oscillates between the beginning and the end. */
		BEHAVIOUR_OSCILLATE,
	};

	/** Creates empty animation with specified number of channels. */
	explicit Interpolator( int channels );

	/** Creates a copy of other interpolator. */
	Interpolator( const Interpolator& other );

	/** Sets number of keys. */
	void	setKeys( int count );

	/** Sets ith key frame. */
	void	setKeyValue( int i, const float* value, int size );

	/** Sets ith key time. Keys must be sorted to ascending order by time. */
	void	setKeyTime( int i, float time );

	/** Sets end behaviour of the animation. Default is BEHAVIOUR_REPEAT. */
	void	setEndBehaviour( BehaviourType behaviour );

	/** Sets pre behaviour of the animation. Default is BEHAVIOUR_REPEAT. */
	void	setPreBehaviour( BehaviourType behaviour );

	/** 
	 * Sorts keys to ascending order by time. Calls reorderKeys()
	 * method which allows derived classes to reorder their key-specific data.
	 */
	void	sortKeys();

	/** 
	 * Reorders data as a result of sorting. Override in derived classes
	 * if the derived class has additional key-specific data.
	 * Call also base class implementation of the function.
	 * @param order Array of sorted indices to old keys.
	 */
	virtual void	reorderKeys( const int* order );

	/** Returns ith key frame. */
	void			getKeyValue( int i, float* value, int size ) const;

	/** Returns pointer to ith key frame value. For optimization only. */
	const float*	getKeyValue( int i ) const;

	/** Returns ith key time. */
	float	getKeyTime( int i ) const;

	/** 
	 * Returns index of the last key frame before normalized time. 
	 * Normalized time has effect of animation pre/post behaviours,
	 * for example BEHAVIOUR_REPEAT causes getNormalizedTime to wrap time to
	 * range between first and last key frame times.
	 * @see getNormalizedTime
	 */
	int		findKey( float time, int hint ) const;

	/** 
	 * Returns animation time after pre/post behaviours. 
	 * Normalized time is between the time of the first key and the time of the last key.
	 * If the interpolator has only single key then normalized time equals to time of that key.
	 */
	float	getNormalizedTime( float time ) const;

	/**
	 * Returns current direction (+-1) of time flow. 
	 * Direction can be -1 if animation end behaviour is BEHAVIOUR_OSCILLATE and
	 * time is on the second half of the loop.
	 * If the interpolator has only one key then the direction is always positive.
	 */
	float	getTimeDirection( float time ) const;

	/** Returns number of channels in the animation. */
	int		channels() const														{return m_channels;}

	/** Returns number of keys. */
	int		keys() const															{return m_times.size();}

	/** Returns time of last key. */
	float	endTime() const;

	/** Returns end behaviour of the animation. */
	BehaviourType	endBehaviour() const;

	/** Returns end behaviour of the animation. */
	BehaviourType	preBehaviour() const;

	/** 
	 * Returns pointer to temporary buffer of specified size. 
	 * Should be used only to implement value interpolation (getValue).
	 * Call to getTempBuffer invalidates any previously returned pointer.
	 */
	float*			getTempBuffer( int size ) const;

	/** 
	 * Converts string to behaviour type. 
	 * @exception Exception If string does not match any behaviour.
	 */
	static BehaviourType	toBehaviour( const lang::String& str );

private:
	util::Vector<float>			m_times;
	util::Vector<float>			m_values;
	mutable util::Vector<float>	m_temp;
	int							m_channels;
	BehaviourType				m_endBehaviour;
	BehaviourType				m_preBehaviour;

	Interpolator& operator=( const Interpolator& );
};


} // anim


#endif // _ANIM_INTERPOLATOR_H
