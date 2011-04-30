#ifndef _SG_EDGEHASH_H
#define _SG_EDGEHASH_H


namespace sg
{


class Edge;


/** 
 * Edge hash/compare function object. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class EdgeHash
{
public:
	/** 
	 * Creates a hash/compare function with specified vertex granularity. 
	 * @param zeroDistance Maximum distance between two vertices to be considered equal.
	 */
	EdgeHash( float zeroDistance=0.f );

	/** Returns true if the edges are equal. */
	bool operator()( const Edge& a, const Edge& b ) const;

	/** Returns edge hash code. */
	int operator()( const Edge& e ) const;

private:
	float	m_zeroDistance;				// d
	float	m_zeroDistanceSquared;		// d*d
	float	m_zeroDistanceInverse2;		// 1/(2*d)
};


} // sg


#endif // _SG_EDGEHASH_H
