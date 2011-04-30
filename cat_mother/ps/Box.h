#ifndef _PS_BOX_H
#define _PS_BOX_H


#include <ps/Shape.h>
#include <math/Vector3.h>


namespace ps
{


/** 
 * Axis-aligned box. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Box :
	public Shape
{
public:
	/// 
	Box( const math::Vector3& minCorner, const math::Vector3& maxCorner );

	/** Returns random point inside the shape. */
	void			getRandomPoint( math::Vector3* point );

private:
	math::Vector3	m_min;
	math::Vector3	m_max;

	Box( const Box& );
	Box& operator=( const Box& );
};


} // ps


#endif // _PS_BOX_H
