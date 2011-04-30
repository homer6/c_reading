inline void GameObject::setVisible( bool visible )
{
	m_visible = visible;
}

inline void	GameObject::setRenderedInFrame( int frame )
{
	m_renderedInFrame = frame;
}

inline const math::Vector3& GameObject::position() const
{
	return m_ms.pos;
}

inline const math::Matrix3x3& GameObject::rotation() const										
{
	return m_ms.rot;
}

inline const math::Vector3& GameObject::velocity() const
{
	return m_ms.vel;
}

inline void GameObject::setVelocity( const math::Vector3& vel )
{
	m_ms.vel = vel;
}

inline int GameObject::renderCount() const
{
	return m_renderCount;
}

inline float GameObject::boundSphere() const
{
	return m_boundSphere;
}

inline bool GameObject::visible() const
{
	return m_visible;
}

inline int GameObject::renderedInFrame() const
{
	return m_renderedInFrame;
}
