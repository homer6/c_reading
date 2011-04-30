#ifndef _SND_SOUND_H
#define _SND_SOUND_H


#include <sg/Node.h>
#include <snd/SoundFormat.h>
#include <lang/String.h>


namespace sd {
	class SoundDriver;
	class SoundDevice;
	class SoundBuffer;}


namespace snd
{


class SoundLock;


/** 
 * Scene grap sound playback.
 *
 * Note about sound world space transform and other properties:
 * Sound buffer device properties are updated in update().
 * In time when the update() is called, the parent node might
 * not be in actual scene (for example if the character is not visible
 * where the sound is parented). This requires that the parent
 * transform is still always valid even though it would not be rendered.
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Sound :
	public sg::Node
{
public:
	/** Usage options. */
	enum UsageFlags
	{
		/** Sound buffer is used for static content. */
		USAGE_STATIC		= 2,
		/** Sound buffer is used for streaming or other dynamic content. Cannot be set if USAGE_STATIC is used. */
		USAGE_DYNAMIC		= 4,
		/** Sound buffer is controlled in 3D. */
		USAGE_CONTROL3D		= 8,
	};

	/**
	 * Initializes sound for to specified size (bytes), frequency, bits/sample, channels and usage flags.
	 * @see SoundLock for filling in sound data.
	 * @see UsageFlags
	 * @exception Exception
	 */
	Sound( sd::SoundDriver* drv, sd::SoundDevice* dev, int bytes, const SoundFormat& format, int usageFlags );

	/** 
	 * Creates clone of other sound. Clone uses shared sound data, but other parameters can be different. 
	 * @exception Exception
	 */
	Sound( const Sound& other );

	///
	~Sound();

	/** Returns clone of sound. Clone uses shared sound data, but other parameters can be different. */
	sg::Node*	clone() const;

	/** Pause toggle. */
	void		pause( bool enabled );

	/** 
	 * Sets current playing position.
	 * If sound is stopped then playback will start at specified time next time play() is called.
	 */
	void		setCurrentPosition( float time );

	/** Enables/disables looping. */
	void		setLooping( bool enabled );

	/** Sets sound playing frequency, Hz. Must be in range [100,100000]. */
	void		setFrequency( int freq );

	/** 
	 * Sets maximum distance when the sound can be heard at all.
	 * Default is FLT_MAX ('infinite').
	 */
	void		setMaxDistance( float dist );

	/** 
	 * Sets maximum distance when the sound can be heard at full volume. 
	 * Default is 1.0.
	 */
	void		setMinDistance( float dist );

	/** 
	 * Sets volume attenuation of the sound, desibels.
	 * -100 is minimum volume (-100dB), 0 is maximum.
	 */
	void		setVolume( float vol );

	/** Updates device sound object parameters. */
	void		update( float dt, float timeScale );

	/** Starts playing the sound. */
	void		play();

	/** Stops the sound playing. */
	void		stop();

	/** 
	 * Starts playing the sound by fading.
	 * Note that update() must be called regularly this to work.
	 */
	void		fadeIn( float time );

	/** 
	 * Stops playing the sound by fading.
	 * Note that update() must be called regularly this to work.
	 */
	void		fadeOut( float time );

	/** Returns true if sound is playing. */
	bool		playing() const;

	/** Returns sound playing frequency, Hz, in range [100,100000]. */
	int			frequency() const;

	/** 
	 * Returns volume attenuation of the sound, desibels.
	 * -100 is minimum volume (-100dB), 0 is maximum.
	 */
	float		volume() const;

	/** 
	 * Returns maximum distance when the sound can be heard at all. 
	 */
	float		maxDistance() const;

	/** 
	 * Returns maximum distance when the sound can be heard at full volume. 
	 */
	float		minDistance() const;

	/** Returns true if the sound has been initialized. */
	bool		initialized() const;

	/** Returns true if sound is paused. */
	bool		paused() const;

	/** Returns sound string description. */
	lang::String	stateString() const;

private:
	friend class SoundLock;

	enum State
	{
		STATE_STOP,
		STATE_PLAY,
		STATE_FADEIN,
		STATE_FADEOUT,
	};

	P(sd::SoundBuffer)	m_sb;
	P(sd::SoundDriver)	m_drv;
	P(sd::SoundDevice)	m_dev;
	int					m_playFlags;
	float				m_vol;
	int					m_freq;
	State				m_state;
	SoundFormat			m_format;
	float				m_fade;			// [0,1]
	float				m_fadeTime;
	bool				m_paused;
	int					m_pausedPos;

	void	create( int bytes, const SoundFormat& format, int usageFlags );
	void	destroy();

	/** Converts dB's to 1/100 dB's in range [-100dB,0dB]. */
	static int		getDrvVolume( float dB );

	/** Interpolates volume in log scale. */
	static float	interpVolume( float u, float vol0, float vol1 );

	Sound& operator=( const Sound& );
};


} // snd


#endif // _SND_SOUND_H
