#ifndef _SGU_NODEGROUPSET_H
#define _SGU_NODEGROUPSET_H


#include <lang/String.h>
#include <util/Hashtable.h>


namespace sg {
	class Node;}


namespace sgu
{



/** 
 * Set of (animated) node groups. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class NodeGroupSet :
	public lang::Object
{
public:
	/** Creates empty set. */
	NodeGroupSet();

	///
	~NodeGroupSet();

	/** Adds a group of nodes. */
	void		addGroup( const lang::String& group, sg::Node* root );

	/** 
	 * Returns root node of a group. 
	 * Assumes that all nodes of a same group are in same hierarchy.
	 */
	sg::Node*	getGroup( const lang::String& group );

	/** Returns named node in specified group (if any). */
	sg::Node*	getNode( const lang::String& group, const lang::String& name );

	/** Returns true if the group exist. */
	bool		hasGroup( const lang::String& group ) const;

private:
	typedef util::Hashtable<lang::String,P(sg::Node)> NodeSetType;
	typedef util::HashtableIterator<lang::String,P(sg::Node)> NodeSetIteratorType;

	util::Hashtable< lang::String, P(NodeSetType) > m_groups;

	NodeGroupSet( const NodeGroupSet& );
	NodeGroupSet& operator=( const NodeGroupSet& );
};


} // sgu


#endif // _SGU_NODEGROUPSET_H
