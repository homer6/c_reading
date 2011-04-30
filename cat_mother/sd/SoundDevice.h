#ifndef _SD_SOUNDDEVICE_H
#define _SD_SOUNDDEVICE_H


namespace math {
	class Matrix4x4;
	class Vector3;}


namespace sd
{


/**
 * Interface for managing sound device.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundDevice
{
public:
	/** Increments reference count by one. */
	virtual void		addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void		release() = 0;

	/** 
	 * Allocates sound device. 
	 * @param maxSimultSounds Maximum number of simultaneous sounds.
	 * @param samplesPerSec Playing frequency, samples per second.
	 * @param bitsPerSamples Number of bits per sample.
	 * @param channel Number of channels per sample.
	 * @return Error code, or 0 if init ok.
	 */
	virtual int			create( int maxSimultSounds, int samplesPerSec, int bitsPerSample, int channels ) = 0;

	/** Deinitializes the sound device explicitly. */
	virtual void		destroy() = 0;

	/** Commits changes to the sound device. */
	virtual void		commit() = 0;

	/** 
	 * Sets distance scale factor. 
	 * Call commit() to apply all changes at once.
	 * @param v Distance factor in range [FLT_MIN,FLT_MAX]
	 */
	virtual void		setDistanceFactor( float v ) = 0;

	/** 
	 * Sets Doppler effect factor.
	 * Call commit() to apply all changes at once.
	 * @param v Doppler effect in range [0,10]. 0 is none, 1 is real-world, 10 is maximum.
	 */
	virtual void		setDopplerFactor( float v ) = 0;

	/** 
	 * Sets attenuation over distance.
	 * Call commit() to apply all changes at once.
	 * @param v Roll-off factor. 0 is none, 1 is real-world, 10 is maximum.
	 */
	virtual void		setRolloffFactor( float v ) = 0;

	/** 
	 * Sets listener world space transform.
	 * Call commit() to apply all changes at once.
	 */
	virtual void		setTransform( const math::Matrix4x4& tm ) = 0;

	/** 
	 * Sets listener world space velocity.
	 * Call commit() to apply all changes at once.
	 */
	virtual void		setVelocity( const math::Vector3& vel ) = 0;

	/**
	 * Sets sound device master volume
	 * @param v volume in decibels
	 */

	virtual void		setVolume( float v ) = 0;

	/** 
	 * Returns distance scale factor. 
	 */
	virtual float		distanceFactor() const = 0;

	/** 
	 * Returns Doppler effect factor.
	 * @return v Doppler effect in range [0,10]. 0 is none, 1 is real-world, 10 is maximum.
	 */
	virtual float		dopplerFactor() const = 0;

	/** 
	 * Returns attenuation over distance.
	 * @return Roll-off factor. 0 is none, 1 is real-world, 10 is maximum.
	 */
	virtual float		rolloffFactor() const = 0;

	/** 
	 * Returns listener world space transform.
	 */
	virtual const math::Matrix4x4&	transform() const = 0;

	/** 
	 * Returns listener world space velocity.
	 */
	virtual const math::Vector3&	velocity() const = 0;

protected:
	SoundDevice() {}
	virtual ~SoundDevice() {}

private:
	SoundDevice( const SoundDevice& );
	SoundDevice& operator=( const SoundDevice& );
};


} // sd


#endif // _SD_SOUNDDEVICE_H
