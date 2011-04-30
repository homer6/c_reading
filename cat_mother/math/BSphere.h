#ifndef _MATH_BSPHERE_H
#define _MATH_BSPHERE_H


#include <math/Vector3.h>


namespace math
{


class Matrix4x4;


/** 
 * Positioned bounding sphere. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSphere
{
public:
	/** Creates an empty bounding sphere to origin. */
	BSphere();

	/** Creates a bounding sphere with specified center point and radius. */
	BSphere( const Vector3& center, float radius );

	/** Sets bounding sphere center point translation. */
	void	setTranslation( const Vector3& center );

	/** Sets bounding sphere radius. */
	void	setRadius( float r );

	/** Returns bounding sphere center point translation. */
	const Vector3&	translation() const;

	/** Returns bounding sphere radius. */
	float			radius() const;

private:
	Vector3			m_center;
	float			m_radius;
};


} // math


#endif // _MATH_BSPHERE_H
