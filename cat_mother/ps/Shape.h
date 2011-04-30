#ifndef _PS_SHAPE_H
#define _PS_SHAPE_H


#include <lang/Object.h>


namespace math {
	class Vector3;}


namespace ps
{


/** 
 * Abstract base class geometric shapes. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Shape :
	public lang::Object
{
public:
	/** Returns random point inside the shape. */
	virtual void	getRandomPoint( math::Vector3* point ) = 0;
};


} // ps


#endif // _PS_SHAPE_H
