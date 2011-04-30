#ifndef _SG_EDGE_H
#define _SG_EDGE_H


#include <math/Vector3.h>


namespace sg
{


/** 
 * Polygon edge data structure used for finding adjacent polygons. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Edge
{
public:
	///
	Edge()																			{}

	/** Constructs an edge from two points. */
	Edge( const math::Vector3& v0, const math::Vector3& v1 )						: m_v0(v0), m_v1(v1) {}

	/** Returns the first edge point. */
	const math::Vector3&	v0() const												{return m_v0;}

	/** Returns the second edge point. */
	const math::Vector3&	v1() const												{return m_v1;}

private:
	math::Vector3	m_v0;
	math::Vector3	m_v1;
};


} // sg


#endif // _SG_EDGE_H
