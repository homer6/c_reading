#ifndef _GD_LOCKMODE_H
#define _GD_LOCKMODE_H


namespace gd
{


/**
 * Device memory lock mode. 
 * Used also when locking vertex buffers, index buffers and textures.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LockMode
{
public:
	/** 
	 * State of the lock.
	 */
	enum State
	{
		/** No access. */
		LOCK_NONE		= 0,
		/** Read only access. */
		LOCK_READ		= 1,
		/** Write only access. */
		LOCK_WRITE		= 2,
		/** Both read and write access. */
		LOCK_READWRITE	= 3,
	};

	/** Constructs lock to LOCK_NONE state. */
	LockMode();

	/** Constructs lock to specified state. */
	LockMode( State state );

	/** Returns true if the lock is not in LOCK_NONE state. */
	bool	locked() const;

	/** Returns true if the lock owner has read access. */
	bool	canRead() const;

	/** Returns true if the lock owner has write access. */
	bool	canWrite() const;

	/** Returns true if the lock states are equal. */
	bool	operator==( const LockMode& other ) const;

	/** Returns true if the lock states are not equal. */
	bool	operator!=( const LockMode& other ) const;

private:
	State	m_state;
};


#include "LockMode.inl"


} // gd


#endif // _GD_LOCKMODE_H
