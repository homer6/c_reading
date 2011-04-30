inline bool Node::cachedWorldTransformValid() const
{
	for ( const Node* node = this ; node ; node = node->m_parent )
		if ( node->m_flags & NODE_WORLDTMDIRTY )
			return false;
	return true;
}

inline bool Node::enabled() const
{
	/*for ( const Node* node = this ; node ; node = node->m_parent )
		if ( !(node->m_flags & NODE_ENABLED) )
			return false;
	return true;*/
	return 0 != (m_flags & NODE_ENABLED);
}

inline bool Node::renderable() const
{
	return 0 != (m_flags & NODE_RENDERABLE);
}

inline void Node::setRenderedInLastFrame( bool enabled )
{
	m_flags &= ~NODE_RENDEREDINLASTFRAME;
	if ( enabled )
		m_flags |= NODE_RENDEREDINLASTFRAME;
}

inline float Node::cachedDistanceToCamera() const							
{
	return m_distanceToCamera;
}

inline bool Node::renderedInLastFrame() const
{
	return 0 != (m_flags & NODE_RENDEREDINLASTFRAME);
}
