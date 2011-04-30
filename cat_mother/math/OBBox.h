#ifndef _MATH_OBBOX_H
#define _MATH_OBBOX_H


#include <math/Matrix4x4.h>


namespace math
{


/** 
 * Oriented bounding box. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class OBBox
{
public:
	/** Creates an empty bounding box to origin. */
	OBBox();

	/** Sets bounding box rotation. */
	void	setRotation( const math::Matrix3x3& rot );

	/** Sets bounding box center point translation. */
	void	setTranslation( const math::Vector3& center );

	/** 
	 * Sets bounding box dimensions. 
	 * Dimensions specify 'radius' of the box along rotation axes.
	 */
	void	setDimensions( const math::Vector3& dim );

	/** 
	 * Gets box vertex positions.
	 * Faces formed by vertices are front=(0,1,2,3), back=(5,4,7,6),
	 * left=(4,0,3,7), right=(1,5,6,2), top=(4,5,1,0) and bottom=(3,2,6,7).
	 * @param tm Transformation to apply to the returned vertices.
	 * @param buffer [out] Receives corner vertices.
	 * @param bufferSize Size of the output buffer. Must be at least 8.
	 * @return Number of corner vertices (8).
	 */
	int		getVertices( const math::Matrix4x4& tm, 
				math::Vector3* buffer, int bufferSize ) const;

	/** Returns bounding box center point translation. */
	math::Vector3			translation() const;

	/** Returns bounding box rotation. */
	math::Matrix3x3			rotation() const;

	/** Returns bounding box transform. */
	const math::Matrix4x4&	transform() const										{return m_tm;}

	/** Returns bounding box dimensions. */
	const math::Vector3&	dimensions() const										{return m_dim;}

private:
	math::Matrix4x4		m_tm;
	math::Vector3		m_dim;
};


} // math


#endif // _MATH_OBBOX_H
