#ifndef _SND_SOUNDMANAGER_H
#define _SND_SOUNDMANAGER_H


#include <lang/Object.h>
#include <lang/String.h>


namespace io {
	class InputStreamArchive;}

namespace sg {
	class Node;}

namespace math {
	class Vector3;}


namespace snd
{


class Sound;


/** 
 * Manages sound playback. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundManager :
	public lang::Object
{
public:
	/** 
	 * Initializes a sound manager.
	 * Loads sounds from specified archive.
	 * @exception Exception
	 */
	explicit SoundManager( io::InputStreamArchive* arch );

	///
	~SoundManager();

	/** Deinitializes all sound and sound device. */
	void	destroy();

	/** Sets parent scene. */
	void	setScene( sg::Node* scene );

	/** 
	 * Updates all active sounds by specified time delta (seconds). 
	 * @param listener Active sound listener. Can be 0.
	 * @param dt Time interval to update (seconds).
	 * @param timeScale Used for applying slowmotion effect to sounds.
	 */
	void	update( sg::Node* listener, float dt, float timeScale=1.f );

	/** Loads a new sound effect to memory. */
	void	load( const lang::String& name );

	/** Starts a new sound effect in reference object space. */
	Sound*	play( const lang::String& name, sg::Node* refobj );

	/** Fades in a new sound effect in reference object space. */
	Sound*	fadeIn( const lang::String& name, sg::Node* refobj, float fadeTime );

	/** Fades out a sound effect. */
	void	fadeOut( const lang::String& name, sg::Node* refobj, float fadeTime );

	/** Stops a sound effect. */
	void	stop( const lang::String& name, sg::Node* refobj );

	/** Sets 3D listener parameters. */
	void	setListenerParam( float distanceFactor, float dopplerFactor, float rolloffFactor );

	/** Removes all managed sounds. */
	void	clear();

	/** Sets primary buffer volume. */
	void	setVolume( float v );

	/** 
	 * Toggle pause for CURRENTLY active sounds.
	 * New sounds can be played without pause affecting them.
	 */
	void	pauseActive( bool enabled );

	/** Removes sounds that are currently playing. */
	void	removeActive();

	/** Returns number of active sounds with specified name in reference object space. */
	int		getActiveCount( const lang::String& name, sg::Node* refobj ) const;

	/** Returns ith active sound. */
	Sound*	getActiveSound( int i ) const;

	/** Returns total number of active sounds. */
	int		activeSounds() const;


private:
	class SoundManagerImpl;
	P(SoundManagerImpl) m_this;

	SoundManager( const SoundManager& );
	SoundManager& operator=( const SoundManager& );
};


} // snd


#endif // _SND_SOUNDMANAGER_H
