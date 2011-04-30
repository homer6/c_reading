inline LockMode::LockMode() : 
	m_state( LOCK_NONE ) 
{
}

inline LockMode::LockMode( State state ) :
	m_state( state )
{
}

inline bool	LockMode::locked() const															
{
	return m_state != LOCK_NONE;
}

inline bool LockMode::canRead() const															
{
	return 0 != (unsigned(m_state) & unsigned(LOCK_READ));
}

inline bool LockMode::canWrite() const														
{
	return 0 != (unsigned(m_state) & unsigned(LOCK_WRITE));
}

inline bool LockMode::operator==( const LockMode& other ) const								
{
	return m_state == other.m_state;
}

inline bool LockMode::operator!=( const LockMode& other ) const								
{
	return m_state != other.m_state;
}
