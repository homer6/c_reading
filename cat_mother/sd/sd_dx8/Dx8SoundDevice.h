#ifndef _DX8SOUNDDEVICE_H
#define _DX8SOUNDDEVICE_H


#include "DrvObject.h"
#include <sd/SoundDevice.h>
#include <math/Matrix4x4.h>


/**
 * Interface for managing sound device.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx8SoundDevice :
	public sd::SoundDevice,
	public DrvObject
{
public:
	Dx8SoundDevice();
	~Dx8SoundDevice();
	
	void		addReference();

	void		release();

	int			create( int maxSimultSounds, int samplesPerSec, int bitsPerSample, int channels );

	void		destroy();

	void		destroyDeviceObject();

	void		commit();

	void		setDistanceFactor( float v );

	void		setDopplerFactor( float v );

	void		setRolloffFactor( float v );

	void		setTransform( const math::Matrix4x4& tm );

	void		setVelocity( const math::Vector3& vel );
	
	void		setVolume( float v );

	float		distanceFactor() const;

	float		dopplerFactor() const;

	float		rolloffFactor() const;

	const math::Matrix4x4&	transform() const;

	const math::Vector3&	velocity() const;

	IDirectSound8*	ds()	{return m_ds;}

	static int getDrvVolume( float dB );

private:
	long						m_refs;
	IDirectSound8*				m_ds;
	IDirectSoundBuffer*			m_primaryBuffer;
	IDirectSound3DListener8*	m_listener;
	int							m_maxSimultSounds;

	math::Matrix4x4				m_tm;
	math::Vector3				m_vel;
	float						m_distanceFactor;
	float						m_rolloffFactor;
	float						m_dopplerFactor;

	int		createPrimaryBuffer( int channels, int samplesPerSec, int bitsPerSample );

	Dx8SoundDevice( const Dx8SoundDevice& );
	Dx8SoundDevice& operator=( const Dx8SoundDevice& );
};


#endif // _DX8SOUNDDEVICE_H
