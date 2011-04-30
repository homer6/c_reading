#ifndef _PS_HALFSPHERE_H
#define _PS_HALFSPHERE_H


#include <ps/Shape.h>
#include <math/Vector3.h>


namespace ps
{


/** 
 * Positioned half sphere. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class HalfSphere :
	public Shape
{
public:
	/// 
	HalfSphere( const math::Vector3& pos, float r );

	/** Returns random point inside the shape. */
	void			getRandomPoint( math::Vector3* point );

private:
	math::Vector3	m_pos;
	float			m_r;

	HalfSphere( const HalfSphere& );
	HalfSphere& operator=( const HalfSphere& );
};


} // ps


#endif // _PS_HALFSPHERE_H
