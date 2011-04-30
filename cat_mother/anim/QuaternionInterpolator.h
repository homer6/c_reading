#ifndef _ANIM_QUATERNIONINTERPOLATOR_H
#define _ANIM_QUATERNIONINTERPOLATOR_H


#include <anim/Interpolator.h>


namespace anim
{


/** 
 * Key-framed animation of quaternions. 
 * Quaternion is stored to channels as (x,y,z,w).
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class QuaternionInterpolator :
	public Interpolator
{
public:
	/** 
	 * Key value interpolation type. 
	 */
	enum InterpolationType
	{
		/** No interpolation. */
		INTERPOLATE_STEPPED,
		/** Linear interpolation. */
		INTERPOLATE_LINEAR,
	};

	/** Creates empty animation. */
	QuaternionInterpolator();

	/** Creates a copy of other interpolator. */
	QuaternionInterpolator( const QuaternionInterpolator& other );

	/** Sets used interpolation type. Default is INTERPOLATE_LINEAR. */
	void	setInterpolation( InterpolationType interp );

	/** 
	 * Gets value of the controller at specified time. 
	 * @param time Time of value to retrieve.
	 * @param value [out] Receives controller value.
	 * @param size Number of floats in the controller value. (4)
	 * @param hint Last returned hint key to speed up operation. (optional)
	 * @return New hint key.
	 */
	int		getValue( float time, float* value, int size, int hint ) const;

	/** Returns used interpolation type. */
	InterpolationType	interpolation() const;

private:
	InterpolationType	m_interp;

	QuaternionInterpolator& operator=( const QuaternionInterpolator& );
};


} // anim


#endif // _ANIM_QUATERNIONINTERPOLATOR_H
