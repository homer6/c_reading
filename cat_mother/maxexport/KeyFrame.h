#ifndef _KEYFRAME_H
#define _KEYFRAME_H


/**
 * Animation key frame. Each key frame in an animation has the value of 
 * the animation channels associated. In addition the key frame has tension,
 * contunity and bias spline controls, an interpolation type and 
 * the time at which the key is located.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class KeyFrame
{
public:
	/** Constants related to the class. */
	enum Constants
	{
		/** Maximum number of channels in a key. */
		MAX_CHANNELS = 4
	};

	/** 
	 * Key interpolation method. 
	 * All types might not be supported by every controller,
	 * in that case the controller falls back to 'next best one'.
	 */
	enum InterpolationType
	{
		/** Point sampling. */
		INTERPOLATE_STEPPED,
		/** Linear interpolation. */
		INTERPOLATE_LINEAR,
		/** Catmull-Rom spline interpolation. */
		INTERPOLATE_CATMULLROM,
	};

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

	/**
	 * Type of key frame data.
	 */ 
	enum DataType
	{
		/** Scalar data. Number of channels must be [1,MAX_CHANNELS]. See KeyFrame. */
		DATA_SCALAR,
		/** Quaternion data. Number of channels must be 4. */
		DATA_QUATERNION
	};

	/** Time (seconds) of the key. */
    float				time;
	/** Used interpolation method. */
	InterpolationType	interpolation;
	/** Tension parameter used in Kochanek-Bartel spline interpolation. */
    float				tension;
	/** Continuity parameter used in Kochanek-Bartel spline interpolation. */
	float				continuity;
	/** Bias parameter used in Kochanek-Bartel spline interpolation. */
	float				bias;

	///
	KeyFrame();

	///
	KeyFrame( float time, InterpolationType interpolation, const float* values, int channels );

	/** Sets value of ith channel in the key. */
	void	setChannel( int i, float value );

	/** Sets values of specified number of channels. */
	void	setChannels( const float* values, int count );

	/** Returns value of ith channel in the key. */
	float	getChannel( int i ) const												{return m_channels[i];}

	/** Returns values of specified number of channels. */
	void	getChannels( float* values, int count ) const							{for ( int i = 0 ; i < count ; ++i ) values[i] = m_channels[i];}

	/** Returns true if this key is before the other. */
	bool	operator<( const KeyFrame& other ) const								{return time < other.time;}

	/** Returns true if this key time equals to other key time. */
	bool	operator==( const KeyFrame& other ) const								{return time == other.time;}

	/** Returns true if this key time inequals to other key time. */
	bool	operator!=( const KeyFrame& other ) const								{return time != other.time;}

private:
	float	m_channels[MAX_CHANNELS];
};


#endif // _KEYFRAME_H
