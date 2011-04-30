#ifndef _SGU_NODEUTIL_H
#define _SGU_NODEUTIL_H


#include <anim/Interpolator.h>


namespace sg {
	class Node;}

namespace lang {
	class String;}


namespace sgu
{


/** 
 * Node helper functions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class NodeUtil
{
public:
	/** 
	 * Finds named node from the hierarchy. 
	 * @return 0 if not found.
	 */
	static sg::Node*	findNodeByName( sg::Node* root, const lang::String& name );

	/**
	 * Sets end behaviour of transformation controllers in the node.
	 */
	static void			setNodeEndBehaviour( sg::Node* node, anim::Interpolator::BehaviourType behaviour );

	/**
	 * Sets pre behaviour of transformation controllers in the node.
	 */
	static void			setNodePreBehaviour( sg::Node* node, anim::Interpolator::BehaviourType behaviour );

	/**
	 * Sets end behaviour of all transformation controllers in the hierarchy.
	 */
	static void			setHierarchyEndBehaviour( sg::Node* root, anim::Interpolator::BehaviourType behaviour );

	/**
	 * Sets pre behaviour of all transformation controllers in the hierarchy.
	 */
	static void			setHierarchyPreBehaviour( sg::Node* root, anim::Interpolator::BehaviourType behaviour );

	/**
	 * Returns bounding sphere of node hierarchy.
	 */
	static float		getHierarchyBoundSphere( sg::Node* root );
};


} // sgu


#endif // _SGU_NODEUTIL_H
