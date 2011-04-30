#ifndef _SG_DUMMY_H
#define _SG_DUMMY_H


#include <sg/Node.h>
#include <pix/Colorf.h>


namespace sg
{


/**
 * Base class for dynamic light sources in the scene graph.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dummy : 
	public Node
{
public:
	/** Maximum light range. */
	static const float	MAX_RANGE;

	/** Maximum (spotlight) cone angle. */
	static const float	MAX_CONE_ANGLE;

	///
	Dummy();

	/** Copy by value. */
	Dummy( const Dummy& other );

	/** Copy by value. */
	Node*	clone() const;

	/** Sets box corners. */
	void	setBox( const math::Vector3& boxMin, const math::Vector3& boxMax );

	/** Returns box min corner. */
	const math::Vector3&	boxMin() const;

	/** Returns box max corner. */
	const math::Vector3&	boxMax() const;

private:
	math::Vector3	m_boxMin;
	math::Vector3	m_boxMax;

	Dummy& operator=( const Dummy& other );
};


} // sg


#endif // _SG_DUMMY_H
