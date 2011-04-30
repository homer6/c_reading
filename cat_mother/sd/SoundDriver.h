#ifndef _SD_SOUNDDRIVER_H
#define _SD_SOUNDDRIVER_H


namespace sd
{


class SoundBuffer;
class SoundDevice;


/**
 * Interface to platform dependent sound driver. 
 * The driver is used for creating other platform-dependent objects.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SoundDriver
{
public:
	/** Constants related to the sound driver. */
	enum Constants
	{
		/** Version number of the current sound engine drivers. */
		VERSION = 1
	};

	/** Increments reference count by one. */
	virtual void						addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void						release() = 0;

	/** Releases all objects allocated by the driver. */
	virtual void						destroy() = 0;

	/** Allocates a sound buffer object. Initial reference count is 0. Thread safe. */
	virtual sd::SoundBuffer*			createSoundBuffer() = 0;

	/** Allocates a sound device object. Initial reference count is 0. Thread safe. */
	virtual sd::SoundDevice*			createSoundDevice() = 0;

protected:
	SoundDriver() {}
	virtual ~SoundDriver() {}

private:
	SoundDriver( const SoundDriver& );
	SoundDriver& operator=( const SoundDriver& );
};


typedef SoundDriver* (*createSoundDriverFunc)();
typedef int	(*getSoundDriverVersionFunc)();


} // sd


#endif // _SD_SOUNDDRIVER_H
