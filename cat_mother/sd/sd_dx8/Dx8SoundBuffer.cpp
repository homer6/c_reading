#include "StdAfx.h"
#include "Dx8SoundDevice.h"
#include "Dx8SoundBuffer.h"
#include "error.h"
#include "toDx8.h"
#include "toString.h"
#include <sd/Errors.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sd;
using namespace math;

//-----------------------------------------------------------------------------

Dx8SoundBuffer::Dx8SoundBuffer() :
	m_refs(0)
{
	defaults();
}

Dx8SoundBuffer::~Dx8SoundBuffer()
{
	destroy();
}

void Dx8SoundBuffer::addReference() 
{
	InterlockedIncrement( &m_refs );
}
	
void Dx8SoundBuffer::release() 
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

int Dx8SoundBuffer::create( SoundDevice* device,
	int bytes, int samplesPerSec, int bitsPerSample, int channels, int usageFlags ) 
{
	assert( device );
	assert( bytes > 0 );
	assert( samplesPerSec > 0 );
	assert( bitsPerSample == 8 || bitsPerSample == 16 );
	assert( channels == 1 || channels == 2 );

	destroy();

	m_buffer			= 0;
	m_buffer3D			= 0;
	m_samples			= bytes / (bitsPerSample/8) / channels;
	m_samplesPerSec		= samplesPerSec;
	m_bitsPerSample		= bitsPerSample;
	m_channels			= channels;
	m_usageFlags		= usageFlags;
	m_dataSize			= bytes;
	m_data				= 0;
	m_dataRefs			= 0;
	m_dirty				= false;
	m_tm				= Matrix4x4(1.f);
	m_vel				= Vector3(0.f,0.f,0.f);
	m_minDistance		= 1.f;
	m_maxDistance		= FLT_MAX;

	if ( usageFlags & USAGE_STATIC )
	{
		m_data = new uint8_t[ m_dataSize ];
		m_dataRefs = new long(1);
		memset( m_data, 0, m_dataSize );
	}

	Dx8SoundDevice*			dev		= static_cast<Dx8SoundDevice*>( device );
	IDirectSound8*			ds		= dev->ds();
	IDirectSoundBuffer*		buffer	= 0;
	assert( ds );

	// buffer wave format
	WAVEFORMATEX fmt;
	memset( &fmt, 0, sizeof(fmt) );
	fmt.cbSize = 0;
	fmt.nAvgBytesPerSec = samplesPerSec * channels * (bitsPerSample/8);
	fmt.nBlockAlign = (WORD)(bitsPerSample * channels / 8);
	fmt.nChannels = (WORD)channels;
	fmt.nSamplesPerSec = samplesPerSec;
	fmt.wBitsPerSample = (WORD)bitsPerSample;
	fmt.wFormatTag = WAVE_FORMAT_PCM;

	// creation flags
	int creationFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY; // DSBCAPS_CTRLPAN
	if ( creationFlags & USAGE_STATIC )
		creationFlags |= DSBCAPS_STATIC;
	if ( usageFlags & USAGE_CONTROL3D )
		creationFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;

	// create the direct sound buffer, and only request the flags needed
	// since each requires some overhead and limits if the buffer can 
	// be hardware accelerated
	DSBUFFERDESC desc;
	memset( &desc, 0, sizeof(desc) );
	desc.dwSize          = sizeof(DSBUFFERDESC);
	desc.dwFlags         = creationFlags;
	desc.dwBufferBytes   = bytes;
	desc.lpwfxFormat     = &fmt;
	desc.guid3DAlgorithm = DS3DALG_DEFAULT;
	//desc.guid3DAlgorithm = DS3DALG_HRTF_FULL;

	HRESULT hr = ds->CreateSoundBuffer( &desc, &buffer, 0 );
	if ( hr != DS_OK )
	{
		error( "CreateSoundBuffer failed: %s", toString(hr) );
		return ERROR_GENERIC;
	}

	// get new interfaces
	int err = getInterfaces( buffer );
	buffer->Release();
	buffer = 0;
	if ( err )
	{
		destroy();
		return err;
	}

	return ERROR_NONE;
}

int Dx8SoundBuffer::getInterfaces( IDirectSoundBuffer* buffer )
{
	assert( buffer );

	// NOTE: Cloned sound buffer do not seem to have IDirectSoundBuffer8 interface

	// get DirectX8 interface
	/*HRESULT hr = buffer->QueryInterface( IID_IDirectSoundBuffer8, (void**)&m_buffer );
	if ( hr != DS_OK )
	{
		error( "QueryInterface IID_IDirectSoundBuffer8 failed: %s", toString(hr) );
		return ERROR_GENERIC;
	}*/
	m_buffer = buffer;
	m_buffer->AddRef();

	// get 3D interface
	if ( m_usageFlags & USAGE_CONTROL3D )
	{
		HRESULT hr = m_buffer->QueryInterface( IID_IDirectSound3DBuffer8, (void**)&m_buffer3D );
		if ( hr != DS_OK )
		{
			error( "QueryInterface IID_IDirectSound3DBuffer8 failed: %s", toString(hr) );
			return ERROR_GENERIC;
		}
	}

	return ERROR_NONE;
}

int Dx8SoundBuffer::duplicate( sd::SoundDevice* device, sd::SoundBuffer* source )
{
	destroy();

	Dx8SoundBuffer*			src		= static_cast<Dx8SoundBuffer*>( source );
	Dx8SoundDevice*			dev		= static_cast<Dx8SoundDevice*>( device );
	IDirectSound8*			ds		= dev->ds();
	IDirectSoundBuffer*		buffer	= 0;
	assert( ds );

	m_buffer = 0;
	m_buffer3D = 0;
	m_samples = src->m_samples;
	m_samplesPerSec = src->m_samplesPerSec;
	m_bitsPerSample = src->m_bitsPerSample;
	m_channels = src->m_channels;
	m_usageFlags = src->m_usageFlags;
	m_dataSize = src->m_dataSize;
	m_data = src->m_data;
	m_dataRefs = src->m_dataRefs; if (m_dataRefs) *m_dataRefs += 1;
	m_dirty = src->m_dirty;
	m_tm = src->m_tm;
	m_vel = src->m_vel;
	m_minDistance = src->m_minDistance;
	m_maxDistance = src->m_maxDistance;

	// duplicate DirectSound buffer
	HRESULT hr = ds->DuplicateSoundBuffer( src->m_buffer, &buffer );
	if ( hr != DS_OK )
	{
		error( "IDirectSound8 DuplicateSoundBuffer failed: %s", toString(hr) );
		return ERROR_GENERIC;
	}

	// get new interfaces
	int err = getInterfaces( buffer );
	buffer->Release();
	buffer = 0;
	if ( err )
	{
		destroy();
		return err;
	}

	return ERROR_NONE;
}

void Dx8SoundBuffer::destroy()
{
	destroyDeviceObject();

	if ( m_data )
	{
		if ( 0 == InterlockedDecrement(m_dataRefs) )
		{
			delete[] m_data;
			m_data = 0;

			delete m_dataRefs;
			m_dataRefs = 0;

			m_dataSize = 0;
		}
	}

	defaults();
}

void Dx8SoundBuffer::defaults()
{
	m_buffer = 0;
	m_buffer3D = 0;
	m_samples = 0;
	m_samplesPerSec = 0;
	m_bitsPerSample = 0;
	m_channels = 0;
	m_usageFlags = 0;
	m_dataSize = 0;
	m_data = 0;
	m_dataRefs = 0;
	m_dirty = false;
	m_tm = Matrix4x4(1.f);
	m_vel = Vector3(0,0,0);
	m_minDistance = 1.f;
	m_maxDistance = FLT_MAX;
}

void Dx8SoundBuffer::destroyDeviceObject()
{
	if ( m_buffer3D )
	{
		m_buffer3D->Release();
		m_buffer3D = 0;
	}

	if ( m_buffer )
	{
		m_buffer->Release();
		m_buffer = 0;
	}
}

void Dx8SoundBuffer::setCurrentPosition( int offset )
{
	HRESULT hr = m_buffer->SetCurrentPosition( offset );
	if ( hr != DS_OK )
		error( "IDirectSoundBuffer8 SetCurrentPosition failed: %s", toString(hr) );
}

void Dx8SoundBuffer::play( int flags ) 
{
	if ( m_dirty )
		if ( load() )
			return;

	DWORD playFlags = 0;
	if ( flags & PLAY_LOOPING )
		playFlags |= DSBPLAY_LOOPING;

	HRESULT hr = m_buffer->Play( 0, 0, playFlags );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 Play failed: %s", toString(hr) );
		if ( hr == DSERR_BUFFERLOST )
			handleLostBuffer();
	}
}
	
void Dx8SoundBuffer::stop()
{
	HRESULT hr = m_buffer->Stop();
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 Stop failed: %s", toString(hr) );
		return;
	}
}

int Dx8SoundBuffer::lock( int offset, int bytes, 
	void** data1, int* bytes1, 
	void** data2, int* bytes2,
	int flags ) 
{
	assert( !locked() );
	assert( offset >= 0 && offset < m_dataSize );
	assert( offset+bytes > 0 && offset+bytes <= m_dataSize );
	assert( data1 && bytes1 );

	if ( m_usageFlags & USAGE_STATIC )
	{
		*data1 = m_data + offset;
		*bytes1 = bytes;
		if ( data2 )
			*data2 = 0;
		if ( bytes2 )
			*bytes2 = 0;
		m_dirty = true;
		m_usageFlags |= USAGE_LOCKED;
		return ERROR_NONE;
	}
	else
	{
		assert( m_usageFlags & USAGE_DYNAMIC );
		return lockDevice( offset, bytes, data1, bytes1, data2, bytes2, flags );
	}
}

void Dx8SoundBuffer::unlock( void* data1, int bytes1, 
	void* data2, int bytes2 ) 
{
	assert( locked() );

	if ( m_usageFlags & USAGE_DYNAMIC )
		unlockDevice( data1, bytes1, data2, bytes2 );

	m_usageFlags &= ~USAGE_LOCKED;

	if ( m_buffer )
		load();
}

void Dx8SoundBuffer::setFrequency( int samplesPerSec ) 
{
	assert( samplesPerSec > 0 );

	HRESULT hr = m_buffer->SetFrequency( samplesPerSec );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 SetFrequency failed: %s", toString(hr) );
		return;
	}
}

void Dx8SoundBuffer::setPan( int pan ) 
{
	assert( pan >= -10000 && pan <= 10000 );

	HRESULT hr = m_buffer->SetPan( pan );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 SetPan failed: %s", toString(hr) );
		return;
	}
}

void Dx8SoundBuffer::setVolume( int vol ) 
{
	assert( vol >= -10000 && vol <= 0 );

	HRESULT hr = m_buffer->SetVolume( vol );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 SetVolume failed: %s", toString(hr) );
		return;
	}
}

void Dx8SoundBuffer::commit()
{
	if ( m_buffer3D )
	{
		Matrix3x3 rot = m_tm.rotation();

		DS3DBUFFER param;
		memset( &param, 0, sizeof(param) );
		param.dwSize = sizeof(param);
		toDx8( m_tm.translation(), param.vPosition );
		toDx8( m_vel, param.vVelocity );
		param.dwInsideConeAngle = DS3D_DEFAULTCONEANGLE;
		param.dwOutsideConeAngle = DS3D_DEFAULTCONEANGLE;
		toDx8( rot.getColumn(2), param.vConeOrientation );
		param.lConeOutsideVolume = DS3D_DEFAULTCONEOUTSIDEVOLUME;
		param.flMinDistance = m_minDistance;
		param.flMaxDistance = m_maxDistance;
		param.dwMode = DS3DMODE_NORMAL;

		/*m_buffer3D->SetMinDistance( param.flMinDistance, DS3D_DEFERRED );
		m_buffer3D->SetMaxDistance( param.flMaxDistance, DS3D_DEFERRED );
		m_buffer3D->SetPosition( param.vPosition.x, param.vPosition.y, param.vPosition.z, DS3D_DEFERRED );
		//m_buffer3D->SetVelocity( param.vVelocity.x, param.vVelocity.y, param.vVelocity.z, DS3D_DEFERRED );*/

		HRESULT hr = m_buffer3D->SetAllParameters( &param, DS3D_DEFERRED );
		if ( hr != DS_OK )
			error( "IDirectSound3DBuffer8 SetAllParemeters failed: %s", toString(hr) );
	}
}

void Dx8SoundBuffer::setMaxDistance( float dist ) 
{
	assert( m_usageFlags & USAGE_CONTROL3D );
	assert( m_buffer3D );

	m_maxDistance = dist;
}

void Dx8SoundBuffer::setMinDistance( float dist ) 
{
	assert( m_usageFlags & USAGE_CONTROL3D );
	assert( m_buffer3D );

	m_minDistance = dist;
}

void Dx8SoundBuffer::setTransform( const math::Matrix4x4& tm )
{
	assert( m_usageFlags & USAGE_CONTROL3D );
	assert( m_buffer3D );

	m_tm = tm;
}

void Dx8SoundBuffer::setVelocity( const math::Vector3& v ) 
{
	assert( m_usageFlags & USAGE_CONTROL3D );
	assert( m_buffer3D );

	m_vel = v;
}

int Dx8SoundBuffer::frequency() const 
{
	DWORD v = 0;
	HRESULT hr = m_buffer->GetFrequency( &v );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 GetFrequency failed: %s", toString(hr) );
		return 0;
	}
	return (int)v;
}

int Dx8SoundBuffer::pan() const 
{
	long v = 0;
	HRESULT hr = m_buffer->GetPan( &v );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 GetPan failed: %s", toString(hr) );
		return 0;
	}
	return (int)v;
}

int Dx8SoundBuffer::volume() const 
{	
	long v = 0;
	HRESULT hr = m_buffer->GetVolume( &v );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 GetVolume failed: %s", toString(hr) );
		return 0;
	}
	return (int)v;
}

float Dx8SoundBuffer::maxDistance() const 
{
	return m_maxDistance;
}

float Dx8SoundBuffer::minDistance() const 
{
	return m_minDistance;
}

const math::Matrix4x4& Dx8SoundBuffer::transform() const 
{
	return m_tm;
}

const math::Vector3& Dx8SoundBuffer::velocity() const 
{
	return m_vel;
}

void Dx8SoundBuffer::getCurrentPosition( int* play, int* write ) const
{
	assert( sizeof(DWORD) == sizeof(int) && (play || write) );
	HRESULT hr = m_buffer->GetCurrentPosition( reinterpret_cast<DWORD*>(play), reinterpret_cast<DWORD*>(write) );
	if ( hr != DS_OK )
		error( "IDirectSoundBuffer8 GetCurrentPosition failed: %s", toString(hr) );
}

bool Dx8SoundBuffer::locked() const
{
	return 0 != (m_usageFlags & USAGE_LOCKED);
}

bool Dx8SoundBuffer::playing() const
{
	return getStatus( DSBSTATUS_PLAYING );
}

int Dx8SoundBuffer::bytes() const
{
	return m_dataSize;
}

int Dx8SoundBuffer::samples() const
{
	return m_samples;
}

int Dx8SoundBuffer::channels() const
{
	return m_channels;
}

int Dx8SoundBuffer::bitsPerSample() const
{
	return m_bitsPerSample;
}

int Dx8SoundBuffer::lockDevice( int offset, int bytes, 
	void** data1, int* bytes1, 
	void** data2, int* bytes2,
	int flags ) 
{
	assert( data1 && bytes1 );
	assert( sizeof(int) == sizeof(DWORD) );

	DWORD lockFlags = 0;
	if ( flags & LOCK_FROMWRITECURSOR )
		lockFlags |= DSBLOCK_FROMWRITECURSOR;

	HRESULT hr = m_buffer->Lock( offset, bytes, data1, (DWORD*)bytes1, data2, (DWORD*)bytes2, lockFlags );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 Lock failed: %s", toString(hr) );
		if ( hr == DSERR_BUFFERLOST )
		{
			handleLostBuffer();
			return ERROR_BUFFERLOST;
		}
		return ERROR_GENERIC;
	}

	m_usageFlags |= USAGE_LOCKED;
	return ERROR_NONE;
}

void Dx8SoundBuffer::unlockDevice( void* data1, int bytes1, 
	void* data2, int bytes2 ) 
{
	assert( locked() );

	HRESULT hr = m_buffer->Unlock( data1, bytes1, data2, bytes2 );
	if ( hr != DS_OK )
	{
		error( "IDirectSoundBuffer8 Unlock failed: %s", toString(hr) );
		return;
	}

	m_usageFlags &= ~USAGE_LOCKED;
}

void Dx8SoundBuffer::handleLostBuffer()
{
	assert( m_buffer );

	HRESULT hr = m_buffer->Restore();
	if ( hr != DS_OK )
		error( "Sound buffer Restore() failed: %s", toString(hr) );

	m_dirty = true;
}

bool Dx8SoundBuffer::getStatus( DWORD statusFlags ) const
{
	DWORD stat = 0;
	
	HRESULT hr = m_buffer->GetStatus( &stat );
	if ( hr != DS_OK )
		error( "IDirectSoundBuffer8 GetStatus failed: %s", toString(hr) );

	return 0 != (stat & statusFlags);
}

int Dx8SoundBuffer::load()
{
	if ( m_usageFlags & USAGE_STATIC )
	{
		void* data = 0;
		int bytes = 0;
		int err = lockDevice( 0, m_dataSize, &data, &bytes, 0, 0, 0 );
		if ( err )
			return err;

		assert( bytes == m_dataSize );
		memcpy( data, m_data, bytes );

		unlockDevice( data, bytes, 0, 0 );
		m_dirty = false;
	}

	return ERROR_NONE;
}

int Dx8SoundBuffer::usageFlags() const
{
	return m_usageFlags;
}
