#ifndef _SGU_LOOKATCONTROLLER_H
#define _SGU_LOOKATCONTROLLER_H


#include <anim/Control.h>


namespace sg {
	class Node;}

namespace math {
	class Vector3;}


namespace sgu
{


/** 
 * Rotation controller for targeting scene graph node to another node. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LookAtControl :
	public anim::Control
{
public:
	/** 
	 * Sets source to look at target node. 
	 * Note: References to nodes are weak.
	 */
	explicit LookAtControl( sg::Node* source, sg::Node* target );

	/** Sets the target node with custom up-direction. */
	LookAtControl( sg::Node* source, sg::Node* target, const math::Vector3& up );

	/** 
	 * Gets (quaternion) value of the controller at specified time.  
	 * @param time Time of value to retrieve.
	 * @param value [out] Receives controller value.
	 * @param size Number of floats in the controller value. (4)
	 * @param hint Last returned hint key to speed up operation. (unused)
	 * @return New hint key. (always 0)
	 */
	int		getValue( float time, float* value, int size, int hint=0 ) const;

	/** Returns number of floats in the controller value. (4) */
	int		channels() const;
	
	/** Returns target node. */
	sg::Node*	target() const;

private:
	sg::Node*	m_source;
	sg::Node*	m_target;
	float		m_up[3];

	LookAtControl( const LookAtControl& );
	LookAtControl& operator=( const LookAtControl& );
};


} // sgu


#endif // _SGU_LOOKATCONTROLLER_H
