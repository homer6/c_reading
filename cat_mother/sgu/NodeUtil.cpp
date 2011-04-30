#include "NodeUtil.h"
#include <sg/Node.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace anim;
using namespace lang;

//-----------------------------------------------------------------------------

namespace sgu
{


Node* NodeUtil::findNodeByName( Node* root, const String& name )
{
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		if ( node->name() == name )
			return node;
	}
	return 0;
}

void NodeUtil::setNodePreBehaviour( sg::Node* node, Interpolator::BehaviourType behaviour )
{
	Interpolator* intp = dynamic_cast<Interpolator*>( node->positionController() );
	if ( intp )
		intp->setPreBehaviour( behaviour );

	intp = dynamic_cast<Interpolator*>( node->rotationController() );
	if ( intp )
		intp->setPreBehaviour( behaviour );

	intp = dynamic_cast<Interpolator*>( node->scaleController() );
	if ( intp )
		intp->setPreBehaviour( behaviour );
}

void NodeUtil::setNodeEndBehaviour( sg::Node* node, Interpolator::BehaviourType behaviour )
{
	Interpolator* intp = dynamic_cast<Interpolator*>( node->positionController() );
	if ( intp )
		intp->setEndBehaviour( behaviour );

	intp = dynamic_cast<Interpolator*>( node->rotationController() );
	if ( intp )
		intp->setEndBehaviour( behaviour );

	intp = dynamic_cast<Interpolator*>( node->scaleController() );
	if ( intp )
		intp->setEndBehaviour( behaviour );
}

void NodeUtil::setHierarchyEndBehaviour( sg::Node* root, Interpolator::BehaviourType behaviour )
{
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
		setNodeEndBehaviour( node, behaviour );
}

void NodeUtil::setHierarchyPreBehaviour( sg::Node* root, Interpolator::BehaviourType behaviour )
{
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
		setNodePreBehaviour( node, behaviour );
}

float NodeUtil::getHierarchyBoundSphere( sg::Node* root )
{
	float maxr = 0.f;

	P(Node) parent = root->parent();
	P(Node) rootNode = root;
	root->unlink();
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		float r = node->boundSphere() + node->worldTransform().translation().length();
		if ( r > maxr )
		{
			maxr = r;
		}
	}
	if ( parent )
		root->linkTo( parent );

	return maxr;
}


} // sgu
