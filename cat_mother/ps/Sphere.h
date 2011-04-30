#ifndef _PS_SPHERE_H
#define _PS_SPHERE_H


#include <ps/Shape.h>
#include <math/Vector3.h>


namespace ps
{


/** 
 * Positioned sphere. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Sphere :
	public Shape
{
public:
	/// 
	Sphere( const math::Vector3& pos, float r );

	/** Returns random point inside the shape. */
	void			getRandomPoint( math::Vector3* point );

private:
	math::Vector3	m_pos;
	float			m_r;

	Sphere( const Sphere& );
	Sphere& operator=( const Sphere& );
};


} // ps


#endif // _PS_SPHERE_H
