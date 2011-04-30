#ifndef _SD_SOUNDBUFFER_H
#define _SD_SOUNDBUFFER_H


namespace math {
	class Matrix4x4;
	class Vector3;}


namespace sd
{


class SoundDevice;


/**
 * Interface for managing sound buffers.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundBuffer
{
public:
	/** Usage options. */
	enum UsageFlags
	{
		/** Sound buffer is locked (this flag is used by implementations only). */
		USAGE_LOCKED		= 1,
		/** Sound buffer is used for static content. */
		USAGE_STATIC		= 2,
		/** Sound buffer is used for streaming or other dynamic content. Cannot be set if USAGE_STATIC is used. */
		USAGE_DYNAMIC		= 4,
		/** Sound buffer is controlled in 3D. */
		USAGE_CONTROL3D		= 8,
	};

	/** Options for playing sounds. */
	enum PlayFlags
	{
		/** Loop to the beginning when the sound reaches end. */
		PLAY_LOOPING	= 1,
	};

	/** Options for locking sound buffers. */
	enum LockFlags
	{
		/** Locks sound buffer from current write cursor position. Offset parameter of lock() is ignored. */
		LOCK_FROMWRITECURSOR	= 1,
	};

	/** Increments reference count by one. */
	virtual void		addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void		release() = 0;

	/** 
	 * Allocates sound buffer. 
	 * @param device Sound device object.
	 * @param bytes Number of bytes in the buffer.
	 * @param samplesPerSec Playing frequency, samples per second.
	 * @param bitsPerSamples Number of bits per sample.
	 * @param channel Number of channels per sample.
	 * @param usageFlags Usage flags, see UsageFlags.
	 * @return Error code or 0 if ok.
	 */
	virtual int			create( sd::SoundDevice* device,
							int bytes, int samplesPerSec, int bitsPerSample, int channels, int usageFlags ) = 0;

	/** 
	 * Duplicates sound buffer from the other buffer.
	 * Duplicate has unique parameters but shares data with the original buffer.
	 * @return Error code or 0 if ok.
	 */
	virtual int			duplicate( sd::SoundDevice* device, sd::SoundBuffer* source ) = 0;

	/** Deinitializes the sound buffer explicitly. */
	virtual void		destroy() = 0;

	/** Sets current playing position in bytes from the start of the buffer. */
	virtual void		setCurrentPosition( int offset ) = 0;

	/** 
	 * Starts playing the sound. 
	 * @param flags Playing options, see PlayFlags.
	 */
	virtual void		play( int flags ) = 0;

	/** Stops playing the sound. */
	virtual void		stop() = 0;

	/**
	 * Locks (part of) the sound buffer.
	 * @param offset First byte to be locked.
	 * @param bytes Number of bytes to be locked.
	 * @param data1 [out] Receives the first part of the locked data.
	 * @param bytes1 [out] Receives number of bytes in the first part of the locked data.
	 * @param data2 [out] Receives the second part of the locked data.
	 * @param bytes2 [out] Receives number of bytes in the second part of the locked data.
	 * @param flags See LockFlags.
	 * @return Error code or 0 if ok.
	 */
	virtual int			lock( int offset, int bytes, 
							void** data1, int* bytes1, 
							void** data2, int* bytes2, int flags ) = 0;

	/** 
	 * Unlocks (part of) the sound buffer. 
	 * @param data1 Pointer returned by lock().
	 * @param bytes1 Number of bytes written to the first data block.
	 * @param data2 Pointer returned by lock().
	 * @param bytes2 Number of bytes written to the second data block.
	 */
	virtual void		unlock( void* data1, int bytes1, 
							void* data2, int bytes2 ) = 0;

	/** Sets playing frequences, samples per second. */
	virtual void		setFrequency( int samplesPerSec ) = 0;

	/** 
	 * Sets relative volume between left and right channels, 1/100 of desibels. 
	 * -10000 represents full left, 10000 full right.
	 * @param pan Volume balance in 1/100 of desibels, range from -10000 to 10000.
	 */
	virtual void		setPan( int pan ) = 0;

	/** 
	 * Sets volume attenuation of the sound, 1/100 of desibels.
	 * -10000 is minimum volume (-100dB), 0 is maximum.
	 * @param vol Volume attanuation, -10000=min, 0=max.
	 */
	virtual void		setVolume( int vol ) = 0;

	/** Applies all changes done to sound buffer to the device object. */
	virtual void		commit() = 0;

	/** 
	 * Sets maximum distance when the sound can be heard at all.
	 * Default is FLT_MAX ('infinite').
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 * After all changes have been done call commit() method to apply changes.
	 */
	virtual void		setMaxDistance( float dist ) = 0;

	/** 
	 * Sets maximum distance when the sound can be heard at full volume. 
	 * Default is 1.0.
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 * After all changes have been done call commit() method to apply changes.
	 */
	virtual void		setMinDistance( float dist ) = 0;

	/** 
	 * Sets sound transformation in distance units. 
	 * Default is identity matrix.
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 * After all changes have been done call commit() method to apply changes.
	 */
	virtual void		setTransform( const math::Matrix4x4& tm ) = 0;

	/** 
	 * Sets sound velocity in distance units. 
	 * Default is zero vector.
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 * After all changes have been done call commit() method to apply changes.
	 */
	virtual void		setVelocity( const math::Vector3& v ) = 0;

	/** 
	 * Returns current play/write cursor positions. 
	 * If other of the parameters is not required 0 can be passed in.
	 */
	virtual void		getCurrentPosition( int* play, int* write ) const = 0;

	/** Returns true if the sound buffer is locked (for writing). */
	virtual bool		locked() const = 0;

	/** Returns true if the sound buffer is playing. */
	virtual bool		playing() const = 0;

	/** 
	 * Returns number of bytes in the sound buffer. 
	 * This number equals to samples()*channels()*bitsPerSample()/8
	 */
	virtual int			bytes() const = 0;

	/** Returns number of channels in the sound buffer. */
	virtual int			channels() const = 0;

	/** Returns number of bits per sample. */
	virtual int			bitsPerSample() const = 0;

	/** Returns playing frequences, samples per second. */
	virtual int			frequency() const = 0;

	/** 
	 * Returns relative volume between left and right channels, 1/100 of desibels. 
	 * -10000 represents full left, 10000 full right.
	 */
	virtual int			pan() const = 0;

	/** 
	 * Returns volume attenuation of the sound, 1/100 of desibels.
	 * -10000 is minimum volume (-100dB), 0 is maximum.
	 */
	virtual int			volume() const = 0;

	/** 
	 * Returns maximum distance when the sound can be heard at all. 
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 */
	virtual float		maxDistance() const = 0;

	/** 
	 * Returns maximum distance when the sound can be heard at full volume. 
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 */
	virtual float		minDistance() const = 0;

	/**
	 * Returns sound buffer usage flags.
	 * @see UsageFlags
	 */
	virtual int			usageFlags() const = 0;

	/** 
	 * Returns sound transformation in distance units. 
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 */
	virtual const math::Matrix4x4&	transform() const = 0;

	/** 
	 * Returns sound velocity in distance units. 
	 * This method can be used only if the sound buffer was created with USAGE_CONTROL3D flag set.
	 */
	virtual const math::Vector3&	velocity() const = 0;

protected:
	SoundBuffer() {}
	virtual ~SoundBuffer() {}

private:
	SoundBuffer( const SoundBuffer& );
	SoundBuffer& operator=( const SoundBuffer& );
};


} // sd


#endif // _SD_SOUNDBUFFER_H
