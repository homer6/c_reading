#ifndef _SND_SOUNDLOCK_H
#define _SND_SOUNDLOCK_H


namespace snd
{


class Sound;


/** 
 * Locks sound object data. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundLock
{
public:
	/** 
	 * Locks object vertex data with specified lock mode. 
	 * @exception SoundLockException
	 */
	SoundLock( Sound* obj, int offset, int bytes, void** data1, int* bytes1, void** data2, int* bytes2 );

	/** Unlocks sound. */
	~SoundLock();

private:
	Sound*	m_obj;	// note! this must be weak pointer because otherwise all models must be allocated to heap
	void*	m_data1;
	void*	m_data2;
	int		m_bytes1;
	int		m_bytes2;

	SoundLock();
	SoundLock( const SoundLock& );
	SoundLock& operator=( const SoundLock& );
};


} // snd


#endif // _SND_SOUNDLOCK_H
