#ifndef _SG_CURVE_H
#define _SG_CURVE_H


#include <sg/Primitive.h>
#include <util/Vector.h>
#include <math/Vector3.h>


namespace sg
{


/** 
 * Sequence of control points. Not rendered. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Curve :
	public Primitive
{
public:
	///
	Curve();

	///
	Curve( const Curve& other );

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** Destroys the object. */
	void	destroy();

	/** Does nothing. */
	void	draw();

	/** Adds a point to the curve. */
	void	addPoint( const math::Vector3& point );

	/** Returns ith point in the curve. */
	const math::Vector3&	getPoint( int index ) const;

	/** Returns number of points in the curve. */
	int		points() const;

	/** Computes bounding sphere of the curve points. */
	float	boundSphere() const;

private:
	util::Vector<math::Vector3>	m_points;

	Curve& operator=( const Curve& );
};


} // sg


#endif // _SG_CURVE_H
