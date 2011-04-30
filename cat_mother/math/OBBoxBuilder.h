#ifndef _MATH_OBBOXBUILDER_H
#define _MATH_OBBOXBUILDER_H


#include <math/OBBox.h>
#include <math/Matrix3x3.h>


namespace math
{


/** 
 * Oriented bounding box builder. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class OBBoxBuilder
{
public:
	/** Begins computing oriented bounding box from set of points. */
	OBBoxBuilder();

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
	 * Returns computed bounding box.
	 */
	const OBBox&	box() const;

private:
	int				m_pass;
	int				m_points;
	math::Vector3	m_mean;
	math::Matrix3x3	m_cov;
	math::Matrix3x3	m_rot;
	math::Vector3	m_rotv[3];
	math::Vector3	m_min;
	math::Vector3	m_max;
	OBBox			m_box;
	bool			m_boxValid;
};


} // math


#endif // _MATH_OBBOXBUILDER_H
