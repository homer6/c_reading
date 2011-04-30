#ifndef _PS_FORCE_H
#define _PS_FORCE_H


#include <lang/Object.h>


namespace math {
	class Vector3;}


namespace ps
{


/** 
 * Abstract base class for a force affecting particles. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Force :
	public lang::Object
{
public:
	/** Applies the force to the particles. */
	virtual void	apply( float time, float dt,
						math::Vector3* positions, 
						math::Vector3* velocities,
						math::Vector3* forces,
						int count ) = 0;
};


} // ps


#endif // _PS_FORCE_H
