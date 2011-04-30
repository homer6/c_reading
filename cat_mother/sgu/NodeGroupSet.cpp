#include "NodeGroupSet.h"
#include <sg/Node.h>
#include <dev/Profile.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace sgu
{


NodeGroupSet::NodeGroupSet() :
	m_groups( Allocator< HashtablePair<lang::String, P(NodeSetType)> >(__FILE__,__LINE__) )
{
}

NodeGroupSet::~NodeGroupSet()
{
}

void NodeGroupSet::addGroup( const String& group, Node* root )
{
	P(NodeSetType)& grp = m_groups[group];
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		if ( node->name().length() > 0 )
		{
			if ( !grp )
				grp = new NodeSetType( Allocator< HashtablePair<String,P(sg::Node)> >(__FILE__,__LINE__) );

			(*grp)[node->name()] = node;
		}
	}
}

Node* NodeGroupSet::getGroup( const String& group )
{
	//dev::Profile pr( "NodeGroupSet.getNode" );
	NodeSetType* grp = m_groups[group];
	if ( grp )
	{
		for ( NodeSetIteratorType it = grp->begin() ; it != grp->end() ; ++it )
		{
			if ( it.value() )
				return it.value()->root();
		}
	}
	return 0;
}

Node* NodeGroupSet::getNode( const String& group, const String& name )
{
	//dev::Profile pr( "NodeGroupSet.getNode" );
	NodeSetType* grp = m_groups[group];
	if ( grp )
		return (*grp)[name];
	return 0;
}

bool NodeGroupSet::hasGroup( const String& group ) const
{
	NodeSetType* grp = m_groups[group];
	return 0 != grp;
}


} // sgu
