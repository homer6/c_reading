#ifndef _DX8SOUNDDRIVER_H
#define _DX8SOUNDDRIVER_H


#include <sd/SoundDriver.h>


/**
 * DirectX implementation of SoundDriver interface.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx8SoundDriver :
	public sd::SoundDriver
{
public:
	Dx8SoundDriver();
	~Dx8SoundDriver();

	void						addReference();

	void						release();

	void						destroy();

	sd::SoundBuffer*			createSoundBuffer();

	sd::SoundDevice*			createSoundDevice();

private:
	long	m_refs;

	Dx8SoundDriver( const Dx8SoundDriver& );
	Dx8SoundDriver& operator=( const Dx8SoundDriver& );
};


#endif // _DX8SOUNDDRIVER_H
