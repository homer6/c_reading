#ifndef _SND_SOUNDLOADER_H
#define _SND_SOUNDLOADER_H


#include "Sound.h"
#include <util/Vector.h>
#include <util/Hashtable.h>


namespace io {
	class InputStreamArchive;}

namespace sd {
	class SoundDriver;
	class SoundDevice;
	class SoundBuffer;}


namespace snd
{


/** 
 * Loads sounds from descriptions.
 */
class SoundLoader :
	public lang::Object
{
public:
	SoundLoader( sd::SoundDriver* drv, sd::SoundDevice* dev, io::InputStreamArchive* arch );

	~SoundLoader();

	/** 
	 * Loads sounds by sound description file (.sf). 
	 * @param name Name of sound description to load.
	 * @param arch Archive to load from.
	 * @param drv Active sound driver.
	 * @param dev Active sound device.
	 * @param sounds [out] Receives loaded sounds.
	 * @exception Exception
	 * @exception IOException
	 */
	void	loadSounds( const lang::String& name, util::Vector<P(Sound)>& sounds );

	/**
	 * Loads WAV to sound.
	 * @see Sound::create
	 * @exception Exception
	 * @exception IOException
	 */
	void	loadWave( const lang::String& fname, int soundUsageFlags, P(Sound)* sound );

private:
	util::Hashtable< lang::String, P(Sound) >	m_sounds;
	P(io::InputStreamArchive)					m_arch;
	P(sd::SoundDriver)							m_drv;
	P(sd::SoundDevice)							m_dev;

	SoundLoader( const SoundLoader& );
	SoundLoader& operator=( const SoundLoader& );
};


} // snd


#endif // _SND_SOUNDLOADER_H
