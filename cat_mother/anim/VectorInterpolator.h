#ifndef _ANIM_VECTORINTERPOLATOR_H
#define _ANIM_VECTORINTERPOLATOR_H


#include <anim/Interpolator.h>


namespace anim
{


/** 
 * Key-framed animation of scalar value channels. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class VectorInterpolator :
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
		/** Catmull-Rom spline interpolation. */
		INTERPOLATE_CATMULLROM,
	};

	/** Creates empty animation with specified number of channels. */
	explicit VectorInterpolator( int channels );

	/** Creates a copy of other interpolator. */
	VectorInterpolator( const VectorInterpolator& other );

	/** Sets used interpolation type. Default is INTERPOLATE_CATMULLROM. */
	void	setInterpolation( InterpolationType interp );

	/** 
	 * Gets value of the controller at specified time. 
	 * @param time Time of value to retrieve.
	 * @param value [out] Receives controller value.
	 * @param size Number of floats in the controller value.
	 * @param hint Last returned hint key to speed up operation. (optional)
	 * @return New hint key.
	 */
	int		getValue( float time, float* value, int size, int hint ) const;

	/** Returns used interpolation type. */
	InterpolationType	interpolation() const;

private:
	InterpolationType	m_interp;

	VectorInterpolator& operator=( const VectorInterpolator& );
};


} // anim


#endif // _ANIM_VECTORINTERPOLATOR_H
