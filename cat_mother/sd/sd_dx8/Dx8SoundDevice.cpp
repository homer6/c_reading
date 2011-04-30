#include "StdAfx.h"
#include "Dx8SoundDevice.h"
#include "error.h"
#include "toDx8.h"
#include "toString.h"
#include <sd/Errors.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sd;
using namespace math;

//-----------------------------------------------------------------------------

Dx8SoundDevice::Dx8SoundDevice() :
	m_refs(0),
	m_ds(0),
	m_primaryBuffer(0),
	m_listener(0),
	m_maxSimultSounds(0),
	m_tm(1.f),
	m_vel(0,0,0),
	m_distanceFactor(1.f),
	m_rolloffFactor(1.f),
	m_dopplerFactor(1.f)
{
}

Dx8SoundDevice::~Dx8SoundDevice()
{
	destroy();
}

void Dx8SoundDevice::addReference() 
{
	InterlockedIncrement( &m_refs );
}

void Dx8SoundDevice::release() 
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

int Dx8SoundDevice::create( int maxSimultSounds, int samplesPerSec, int bitsPerSample, int channels ) 
{
	m_maxSimultSounds = maxSimultSounds;

	HWND		hwnd		= GetActiveWindow();
	DWORD		coop		= DSSCL_PRIORITY;

	// check that we have active window
	if ( !hwnd )
	{
		error( "No active window" );
		return ERROR_NOWINDOW;
	}

	// create IDirectSound using the primary sound device
	HRESULT hr = DirectSoundCreate8( 0, &m_ds, 0 );
	if ( hr != DS_OK )
	{
		error( "DirectSoundCreate8 failed: %s", toString(hr) );
		return ERROR_DIRECTXNOTINSTALLED;
	}

	// set DirectSound coop level
	hr = m_ds->SetCooperativeLevel( hwnd, coop );
	if ( hr != DS_OK )
	{
		error( "SetCooperativeLevel failed: %s", toString(hr) );
		destroy();
		return ERROR_DIRECTXNOTINSTALLED;
	}

	// init primary buffer
	int err = createPrimaryBuffer( channels, samplesPerSec, bitsPerSample );
	if ( err )
	{
		destroy();
		return err;
	}

	return ERROR_NONE;
}

void Dx8SoundDevice::destroy() 
{
	destroyDeviceObject();
}

void Dx8SoundDevice::destroyDeviceObject()
{
	if ( m_listener )
	{
		m_listener->Release();
		m_listener = 0;
	}

	if ( m_primaryBuffer )
	{
		m_primaryBuffer->Release();
		m_primaryBuffer = 0;
	}

	if ( m_ds )
	{
		m_ds->Release();
		m_ds = 0;
	}
}

void Dx8SoundDevice::commit() 
{
	if ( m_listener )
	{
		Matrix3x3 rot = m_tm.rotation();

		DS3DLISTENER param;
		memset( &param, 0, sizeof(param) );
		param.dwSize = sizeof(param);
		param.flDistanceFactor = m_distanceFactor;
		param.flDopplerFactor = m_dopplerFactor;
		param.flRolloffFactor = m_rolloffFactor;
		toDx8( rot.getColumn(2), param.vOrientFront );
		toDx8( rot.getColumn(1), param.vOrientTop );
		toDx8( m_tm.translation(), param.vPosition );
		toDx8( m_vel, param.vVelocity );

		/*m_listener->SetDistanceFactor( param.flDistanceFactor, DS3D_DEFERRED );
		m_listener->SetDopplerFactor( param.flDopplerFactor, DS3D_DEFERRED );
		m_listener->SetRolloffFactor( param.flRolloffFactor, DS3D_DEFERRED );
		m_listener->SetOrientation( param.vOrientFront.x, param.vOrientFront.y, param.vOrientFront.z,
			param.vOrientTop.x, param.vOrientTop.y, param.vOrientTop.z, DS3D_DEFERRED );
		m_listener->SetPosition( param.vPosition.x, param.vPosition.y, param.vPosition.z, DS3D_DEFERRED );
		//m_listener->SetVelocity( param.vVelocity.x, param.vVelocity.y, param.vVelocity.z, DS3D_DEFERRED );*/

		HRESULT hr;
		hr = m_listener->SetAllParameters( &param, DS3D_DEFERRED );
		if ( hr != DS_OK )
			error( "SetAllParameters failed: %s", toString(hr) );

		hr = m_listener->CommitDeferredSettings();
		if ( hr != DS_OK )
			error( "CommitDeferredSettings failed: %s", toString(hr) );
	}
}

void Dx8SoundDevice::setDistanceFactor( float v ) 
{
	assert( v >= FLT_MIN && v <= FLT_MAX );

	m_distanceFactor = v;
}

void Dx8SoundDevice::setDopplerFactor( float v ) 
{
	assert( v >= 0.f && v <= 10.f );

	m_dopplerFactor = v;
}

void Dx8SoundDevice::setRolloffFactor( float v ) 
{
	assert( v >= 0.f && v <= 10.f );

	m_rolloffFactor = v;
}

void Dx8SoundDevice::setVolume( float v )
{
	assert( m_primaryBuffer );

	//m_primaryBuffer->SetVolume( getDrvVolume( v ) );
}

void Dx8SoundDevice::setTransform( const Matrix4x4& tm ) 
{
	m_tm = tm;
}

void Dx8SoundDevice::setVelocity( const Vector3& vel ) 
{
	m_vel = vel;
}

float Dx8SoundDevice::distanceFactor() const 
{
	return m_distanceFactor;
}

float Dx8SoundDevice::dopplerFactor() const 
{
	return m_dopplerFactor;
}

float Dx8SoundDevice::rolloffFactor() const 
{
	return m_rolloffFactor;
}

const Matrix4x4& Dx8SoundDevice::transform() const 
{
	return m_tm;
}

const Vector3& Dx8SoundDevice::velocity() const 
{
	return m_vel;
}

int Dx8SoundDevice::createPrimaryBuffer( int channels, int samplesPerSec, int bitsPerSample )
{
	assert( m_ds );
	assert( channels == 1 || channels == 2 );
	assert( samplesPerSec >= 8*1024 && samplesPerSec <= 48*1024 );
	assert( bitsPerSample == 8 || bitsPerSample == 16 || bitsPerSample == 32 );

	// get the primary buffer 
	DSBUFFERDESC desc;
	memset( &desc, 0, sizeof(desc) );
	desc.dwSize				= sizeof(DSBUFFERDESC);
	desc.dwFlags			= DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
	desc.dwBufferBytes		= 0;
	desc.lpwfxFormat		= 0;
    
	HRESULT hr = m_ds->CreateSoundBuffer( &desc, &m_primaryBuffer, 0 );
	if ( hr != DS_OK )
	{
		error( "CreateSoundBuffer (primary buffer) failed: %s", toString(hr) );
		return ERROR_GENERIC;
	}

	// set primary buffer format
	WAVEFORMATEX fmt;
	memset( &fmt, 0, sizeof(desc) );
	fmt.wFormatTag      = WAVE_FORMAT_PCM; 
	fmt.nChannels       = (WORD)channels; 
	fmt.nSamplesPerSec  = samplesPerSec; 
	fmt.wBitsPerSample  = (WORD)bitsPerSample; 
	fmt.nBlockAlign     = (WORD)(fmt.wBitsPerSample / 8 * fmt.nChannels);
	fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

	hr = m_primaryBuffer->SetFormat( &fmt );
	if ( hr != DS_OK )
	{
		error( "SetFormat (primary buffer) failed: %s", toString(hr) );
		return ERROR_GENERIC;
	}

	// get 3D listener interface
	hr = m_primaryBuffer->QueryInterface( IID_IDirectSound3DListener, (void**)&m_listener );
	if ( hr != DS_OK )
	{
		error( "QueryInterface (primary buffer) IDirectSound3DListener failed: %s", toString(hr) );
		return ERROR_GENERIC;
	}

	return 0;
}

int Dx8SoundDevice::getDrvVolume( float dB )
{
	dB *= 100.f;
	if ( dB < -10000.f )
		dB = -10000.f;
	else if ( dB > 0.f )
		dB = 0.f;
	int v = (int)dB;
	return v;
}
