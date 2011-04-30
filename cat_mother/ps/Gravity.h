#ifndef _PS_GRAVITY_H
#define _PS_GRAVITY_H


#include <ps/Force.h>
#include <math/Vector3.h>


namespace ps
{


/** 
 * Classic gravity affecting particles. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Gravity :
	public Force
{
public:
	/** Mass of the Earth, kg. */
	static const float		EARTH_RADIUS;

	/** Mean radius of the Earth. */
	static const float		EARTH_MASS;

	/** Mean radius of the Moon. */
	static const float		MOON_RADIUS;

	/** Mass of the Moon. */
	static const float		MOON_MASS;

	/** Constructs 'standard' gravity of specified strength. */
	explicit Gravity( float value );

	/** 
	 * Constructs Newton gravity from positioned mass. 
	 * All particles has unit masses.
	 */
	Gravity( const math::Vector3& pos, float mass );

	/** Applies the force to the particles. */
	void	apply( float time, float dt,
				math::Vector3* positions, 
				math::Vector3* velocities,
				math::Vector3* forces,
				int count );

private:
	bool			m_std;
	float			m_g;
	math::Vector3	m_pos;
	float			m_mass;

	Gravity( const Gravity& );
	Gravity& operator=( const Gravity& );
};


} // ps


#endif // _PS_GRAVITY_H
