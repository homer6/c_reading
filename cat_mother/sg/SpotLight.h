#ifndef _SG_SPOTLIGHT_H
#define _SG_SPOTLIGHT_H


#include <sg/Light.h>


namespace sg
{


/**
 * Dynamic spot light source in the scene graph. 
 * Good for simulating cone shaped light sources like for example flashlights.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SpotLight : 
	public Light
{
public:
	///
	SpotLight();

	/** Creates a copy of this object. */
	SpotLight( const SpotLight& other );

	Node*	clone() const;

	/** Applies this light source to the active rendering device. */
	void	apply();

	/** 
	 * Sets angle, in radians, of a fully illuminated spotlight cone. 
	 * @see setOuterCone
	 */
	void	setInnerCone( float angle );
	
	/** 
	 * Sets angle, in radians, defining the outer edge of the spotlight's outer cone. 
	 * Points outside this cone are not lit by the spotlight. 
	 * @see setInnerCone
	 */
	void	setOuterCone( float angle );

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
	 * Returns angle, in radians, of a fully illuminated spotlight cone. 
	 * @see setOuterCone
	 */
	float	innerCone() const;
	
	/** 
	 * Returns angle, in radians, defining the outer edge of the spotlight's outer cone.
	 * Points outside this cone are not lit by the spotlight. 
	 * @see innerCone
	 */
	float	outerCone() const;

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

	/** Returns maximum allowable spotlight cone angle in radians. */
	static float	maxConeAngle();

private:
	float	m_atten[3];
	float	m_range;
	float	m_inner;
	float	m_outer;

	void	assign( const SpotLight& other );

	SpotLight& operator=( const SpotLight& other );
};


} // sg


#endif // _SG_SPOTLIGHT_H
