#ifndef _GMPATCH_H
#define _GMPATCH_H


#include <math/Vector3.h>


/** 
 * Cubic 4x4 Bezier patch. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GmPatch
{
public:
	/** Constructs a patch with all control points (0,0,0). */
	GmPatch();

	/** Sets control point (i,j). */
	void					setControlPoint( int i, int j, const math::Vector3&	point );

	/** Returns control point (i,j). */
	const math::Vector3&	getControlPoint( int i, int j ) const;

private:
	math::Vector3	m_points[4][4];
};


#endif // _GMPATCH_H
