#ifndef _DX8SOUNDBUFFER_H
#define _DX8SOUNDBUFFER_H


#include "DrvObject.h"
#include <sd/SoundBuffer.h>
#include <math/Matrix4x4.h>
#include <stdint.h>


/**
 * DirectX implementation of SoundBuffer interface.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx8SoundBuffer :
	public sd::SoundBuffer,
	public DrvObject
{
public:
	Dx8SoundBuffer();
	~Dx8SoundBuffer();

	void		addReference();
	
	void		release();
	
	int			create( sd::SoundDevice* device, int bytes, int samplesPerSec, int bitsPerSample, int channels, int usageFlags );

	int			duplicate( sd::SoundDevice* device, sd::SoundBuffer* source );

	void		destroy();

	void		destroyDeviceObject();

	void		setCurrentPosition( int offset );

	void		play( int flags=0 );
	
	void		stop();

	int			lock( int offset, int bytes, 
					void** data1, int* count1, 
					void** data2, int* count2, int flags );

	void		unlock( void* data1, int bytes1, 
					void* data2, int bytes2 );

	void		setFrequency( int samplesPerSec );

	void		setPan( int pan );

	void		setVolume( int vol );

	void		commit();

	void		setMaxDistance( float dist );

	void		setMinDistance( float dist );

	void		setTransform( const math::Matrix4x4& tm );

	void		setVelocity( const math::Vector3& v );

	void		getCurrentPosition( int* play, int* write ) const;

	bool		locked() const;

	bool		playing() const;

	int			bytes() const;

	int			samples() const;

	int			channels() const;

	int			bitsPerSample() const;

	int			frequency() const;

	int			pan() const;

	int			volume() const;

	float		maxDistance() const;

	float		minDistance() const;

	int			usageFlags() const;

	const math::Matrix4x4&	transform() const;

	const math::Vector3&	velocity() const;

private:
	long					m_refs;
	IDirectSoundBuffer*	m_buffer;
	IDirectSound3DBuffer8*	m_buffer3D;
	int						m_samples;
	int						m_samplesPerSec;
	int						m_bitsPerSample;
	int						m_channels;
	int						m_usageFlags;
	int						m_dataSize;			// bytes
	uint8_t*				m_data;
	long*					m_dataRefs;
	bool					m_dirty;
	
	math::Matrix4x4			m_tm;
	math::Vector3			m_vel;
	float					m_minDistance;
	float					m_maxDistance;

	int		lockDevice( int offset, int bytes, 
				void** data1, int* count1, 
				void** data2, int* count2, int flags );

	void	unlockDevice( void* data1, int bytes1, 
				void* data2, int bytes2 );

	int		getInterfaces( IDirectSoundBuffer* buffer );

	int		load();

	void	handleLostBuffer();

	void	defaults();

	bool	getStatus( DWORD statusFlags ) const;

	Dx8SoundBuffer( const Dx8SoundBuffer& );
	Dx8SoundBuffer& operator=( const Dx8SoundBuffer& );
};


#endif // _DX8SOUNDBUFFER_H
