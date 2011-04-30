#ifndef _MUSIC_MUSICMANAGER_H
#define _MUSIC_MUSICMANAGER_H


#include <lang/Object.h>
#include <lang/String.h>


namespace music
{


/** 
 * Manages background music playback. 
 * Supported music file formats are platform dependent.
 * On Win32 MP3 and WAV are supported, see DirectShow documentation for more file formats.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MusicManager :
	public lang::Object
{
public:
	/** 
	 * Initializes music playback manager.
	 * @exception Exception
	 */
	MusicManager();

	~MusicManager();

	/** 
	 * Pause on/off. 
	 * Note that music does not automatically go to pause mode when
	 * the task is switched to another application.
	 */
	void	pause( bool enabled );

	/**
	 * Sets normal music volume attenuation in dB.
	 * @param vol Volume attenuation in range [-100,0].
	 */
	void	setVolume( float vol );
	
	/**
	 * Loads specified music file.
	 * @exception Exception
	 */
	void	load( const lang::String& name );

	/** 
	 * Updates music playback (fading effects). 
	 * @exception Exception
	 */
	void	update( float dt );

	/** 
	 * Starts playing specified music file. 
	 * @exception Exception
	 */
	void	play( const lang::String& name );

	/** 
	 * Stops playing music. 
	 * @exception Exception
	 */
	void	stop();

	/** 
	 * Fades in music in specified time. 
	 * @exception Exception
	 */
	void	fadeIn( const lang::String& name, float time );

	/** 
	 * Fades out music in specified time. 
	 * @exception Exception
	 */
	void	fadeOut( float time );

	
	static float interpVolume( float u, float vol0, float vol1 );

private:
	class MusicManagerImpl;
	P(MusicManagerImpl)	m_this;

	MusicManager( const MusicManager& );
	MusicManager& operator=( const MusicManager& );
};


} // music


#endif // _MUSIC_MUSICMANAGER_H
