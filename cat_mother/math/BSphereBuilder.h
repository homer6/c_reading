#ifndef _MATH_BSPHEREBUILDER_H
#define _MATH_BSPHEREBUILDER_H


#include <math/BSphere.h>


namespace math
{


/** 
 * Bounding sphere builder. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSphereBuilder
{
public:
	/** Begins computing bounding sphere from set of points. */
	BSphereBuilder();

	/** 
	 * Returns true if more passes over the points is needed.
	 * Call BEFORE each pass over the points.
	 */
	bool	nextPass();

	/**
	 * Adds points to the bounding box computation.
	 */
	void	addPoints( const math::Vector3* points, int count );

	/** 
	 * Returns computed bounding sphere.
	 */
	const BSphere&	sphere() const;

private:
	int				m_pass;
	int				m_points;
	math::Vector3	m_center;
	float			m_maxRadius;
	BSphere			m_sphere;
	bool			m_sphereValid;
};


} // math


#endif // _MATH_BSPHEREBUILDER_H
