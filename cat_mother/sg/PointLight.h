#ifndef _SG_POINTLIGHT_H
#define _SG_POINTLIGHT_H


#include <sg/Light.h>


namespace sg
{


/**
 * Dynamic point light source in the scene graph. 
 * Good for simulating near omni light sources like for example candles.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PointLight : 
	public Light
{
public:
	///
	PointLight();

	/** Creates a copy of this object. */
	PointLight( const PointLight& other );

	Node*	clone() const;

	/** Applies this light source to the active rendering device. */
	void	apply();

	/** 
	 * Sets distance in world space beyond which the light has no effect. 
	 * Maximum allowable value is maxRange(). 
	 * Updates node bounding volume.
	 */
	void	setRange( float range );

	/** 
	 * Sets values specifying how the light intensity changes over distance. 
	 * Light effect is computed using following formula:<br>
	 * <i>L = 1 / ( a0 + a1*distance + a2/distance^2 )</i><br>
	 */
	void	setAttenuation( float a0, float a1, float a2 );

	/**  
	 * Returns distance in world space beyond which the light has no effect. 
	 * Maximum allowable value is maxRange(). 
	 */
	float	range() const;

	/** 
	 * Returns attenuation coefficients.
	 * @see setAttenuation
	 */
	void	getAttenuation( float* a0, float* a1, float* a2 ) const;

	/** Returns maximum allowable effect range for a light. */
	static float	maxRange();

private:
	float	m_atten[3];
	float	m_range;

	void	assign( const PointLight& other );

	PointLight& operator=( const PointLight& );
};


} // sg


#endif // _SG_POINTLIGHT_H
