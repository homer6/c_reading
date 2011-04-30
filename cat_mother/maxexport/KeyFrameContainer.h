#ifndef _KEYFRAMECONTAINER_H
#define _KEYFRAMECONTAINER_H


#include "KeyFrame.h"
#include <lang/Object.h>


class Interpolation;


/** 
 * Animation key frame container. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class KeyFrameContainer :
	public lang::Object
{
public:
	/** Constructs an empty animation with specified number of channels. */
	explicit KeyFrameContainer( int channels, KeyFrame::DataType data );

	/** Copy by value. */
	KeyFrameContainer( const KeyFrameContainer& other );

	///
	~KeyFrameContainer();

	/** Copy by value. */
	KeyFrameContainer&		operator=( const KeyFrameContainer& other );

	/** Inserts a new key to the animation. */
	void					insertKey( const KeyFrame& key );

	/** Erases a key from the animation. */
	void					removeKey( int index );

	/** Removes all keys from the container. */
	void					clear();
	
	/** Sets behaviour of the animation beyond the last frame. */
	void					setEndBehaviour( KeyFrame::BehaviourType type );

	/** Returns ith key frame in the animation. */
	KeyFrame&				getKey( int index );

	/** Returns behaviour of the animation beyond the last frame. */
	KeyFrame::BehaviourType	endBehaviour() const;

	/** Returns number of channels in the animation. */
	int						channels() const;

	/** Returns animation length (seconds). */
	float					length() const;

	/** Returns number of key frames in the animation. */
	int						keys() const;

	/** Returns ith key frame in the animation. */
	const KeyFrame&			getKey( int index ) const;

	/** 
	 * Returns interpolated channel values at specified time. 
	 * The animation must have at least one key frame defined.
	 *
	 * @param time Time (seconds) of requested sample.
	 * @param values [out] Receives interpolated values.
	 * @param channels Number of channels to get.
	 * @param hintKey Hint index of the last retrieved value.
	 * @return New hint index.
	 */
	int						getValue( float time, float* values, int channels, int hintKey=0 ) const;

	/** Return animation data type. */
	KeyFrame::DataType		data() const;

	/** Returns animation interpolation type. */
	KeyFrame::InterpolationType	interpolation() const;

private:
	class KeyFrameContainerImpl;
	P(KeyFrameContainerImpl) m_this;
};


#endif // _KEYFRAMECONTAINER_H
