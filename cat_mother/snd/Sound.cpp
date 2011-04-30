#include "Sound.h"
#include <sd/SoundDriver.h>
#include <sd/SoundDevice.h>
#include <sd/SoundBuffer.h>
#include <snd/SoundFormat.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/Exception.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sd;
using namespace sg;
using namespace snd;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace snd
{


Sound::Sound( SoundDriver* drv, SoundDevice* dev, int bytes, const SoundFormat& format, int usageFlags ) :
	m_sb( 0 ),
	m_drv( drv ),
	m_dev( dev ),
	m_playFlags( 0 ),
	m_vol( 0.f ),
	m_state( STATE_STOP ),
	m_format( 0, 0, 0 ),
	m_fade( 0.f ),
	m_fadeTime( 0.f ),
	m_paused( false ),
	m_pausedPos( 0 )
{
	create( bytes, format, usageFlags );
}

Sound::Sound( const Sound& other ) :
	m_sb( 0 ),
	m_drv( other.m_drv ),
	m_dev( other.m_dev ),
	m_playFlags( other.m_playFlags ),
	m_vol( other.m_vol ),
	m_freq( other.m_freq ),
	m_state( STATE_STOP ),
	m_format( other.m_format ),
	m_fade( 0.f ),
	m_fadeTime( 0.f ),
	m_paused( false ),
	m_pausedPos( 0 )
{
	m_sb = m_drv->createSoundBuffer();
	int err = m_sb->duplicate( m_dev, other.m_sb );
	if ( err )
		throw Exception( Format("Failed to duplicate sound buffer {0}", other.name()) );
}

Sound::~Sound()
{
	destroy();

	m_dev = 0;
	m_drv = 0;
}

Node* Sound::clone() const
{
	return new Sound( *this );
}

void Sound::create( int bytes, const SoundFormat& format, int usageFlags )
{
	assert( !m_sb );

	int sbflags = 0;
	if ( usageFlags & USAGE_STATIC )
		sbflags |= SoundBuffer::USAGE_STATIC;
	if ( usageFlags & USAGE_DYNAMIC )
		sbflags |= SoundBuffer::USAGE_DYNAMIC;
	if ( usageFlags & USAGE_CONTROL3D )
		sbflags |= SoundBuffer::USAGE_CONTROL3D;

	m_sb = m_drv->createSoundBuffer();
	int err = m_sb->create( m_dev, bytes, format.samplesPerSec(), format.bitsPerSample(), format.channels(), sbflags );
	if ( err )
		throw Exception( Format("Failed to init sound buffer {0}", name()) );

	m_format = format;
}

void Sound::destroy()
{
	if ( m_sb )
	{
		m_sb->stop();
		m_sb->destroy();
		m_sb = 0;
	}
}

void Sound::update( float dt, float timeScale )
{
	assert( m_sb );

	// nothing to update?
	if ( m_paused || dt*timeScale <= Float::MIN_VALUE )
		return;

	// update playing speed
	int freq = (int)( (float)m_format.samplesPerSec() * timeScale );
	if ( freq < 100 )
		freq = 100;
	else if ( freq > 100000 )
		freq = 100000;
	if ( m_sb->frequency() != freq )
		m_sb->setFrequency( freq );

	// update 3D properties
	if ( SoundBuffer::USAGE_CONTROL3D & m_sb->usageFlags() )
	{
		// update tm
		Matrix4x4 tm = worldTransform();
		m_sb->setTransform( tm );
	}

	// update fading
	if ( m_fadeTime > 0.f )
	{
		m_fade += dt / m_fadeTime;
		if ( m_fade > 1.f )
			m_fade = 1.f;
	}

	// update (fading) state
	switch ( m_state )
	{
	case STATE_STOP:
		break;

	case STATE_PLAY:
		break;

	case STATE_FADEIN:
		assert( m_fadeTime > Float::MIN_VALUE );
		m_sb->setVolume( getDrvVolume( interpVolume(m_fade,-100.f,m_vol) ) );
		if ( m_fade >= 1.f )
			m_state = STATE_PLAY;
		break;

	case STATE_FADEOUT:
		assert( m_fadeTime > Float::MIN_VALUE );
		m_sb->setVolume( getDrvVolume( interpVolume(m_fade,m_vol,-100.f) ) );
		if ( m_fade >= 1.f )
			stop();
		break;
	}

	// commit changes to device
	m_sb->commit();
}

void Sound::setCurrentPosition( float startTime )
{
	assert( m_sb );

	// start offset
	int startByte = 0;
	if ( startTime > 0.f )
	{
		float bytesPerSample = (float)m_format.bitsPerSample()/8.f;
		float channels = (float)m_format.channels();
		float samplesPerSec = (float)m_format.samplesPerSec();
		startByte = (int)( startTime * samplesPerSec * channels * bytesPerSample );
		if ( startByte < 0 )
			startByte = 0;
		else if ( startByte >= m_sb->bytes() )
			startByte = m_sb->bytes()-1;
	}

	m_sb->setCurrentPosition( startByte );
}

void Sound::play()
{
	assert( m_sb );

//	Debug::println( "Playing sound {0}", name() );
	m_sb->play( m_playFlags );
	m_sb->setVolume( getDrvVolume(m_vol) );
	m_state = STATE_PLAY;
}

void Sound::stop()
{
	assert( m_sb );

	m_sb->stop();
	m_state = STATE_STOP;
}

void Sound::fadeIn( float time ) 
{
	assert( m_sb );
	assert( time > Float::MIN_VALUE );

	Debug::println( "Fading in sound {0} for {1} seconds", name(), time );

	if ( time < Float::MIN_VALUE )
	{
		play();
		return;
	}

	switch ( m_state )
	{
	case STATE_STOP:
		m_fade = 0.f;
		m_fadeTime = time;
		m_state = STATE_FADEIN;
		m_sb->setVolume( getDrvVolume(-100.f) );
		m_sb->play( m_playFlags );
		break;

	case STATE_PLAY:
		break;

	case STATE_FADEIN:
		break;

	case STATE_FADEOUT:
		m_fade = Math::min( Math::max( 1.f, 1.f-m_fade ), 0.f );
		m_fadeTime = time;
		m_state = STATE_FADEIN;
		break;
	}
}

void Sound::fadeOut( float time ) 
{
	assert( m_sb );
	assert( time > Float::MIN_VALUE );

	if ( time < Float::MIN_VALUE )
	{
		stop();
		return;
	}

	switch ( m_state )
	{
	case STATE_STOP:
		break;

	case STATE_PLAY:
		m_fade = 0.f;
		m_fadeTime = time;
		m_state = STATE_FADEOUT;
		break;

	case STATE_FADEIN:
		m_fade = Math::min( Math::max( 1.f, 1.f-m_fade ), 0.f );
		m_fadeTime = time;
		m_state = STATE_FADEOUT;
		break;

	case STATE_FADEOUT:
		break;
	}
}

bool Sound::playing() const 
{
	assert( m_sb );
	return m_sb && m_sb->playing();
}

void Sound::setLooping( bool enabled )
{
	assert( m_sb );

	m_playFlags &= ~SoundBuffer::PLAY_LOOPING;
	if ( enabled )
		m_playFlags |= SoundBuffer::PLAY_LOOPING;
}

void Sound::setFrequency( int freq )
{
	assert( m_sb );
	assert( freq >= 100 && freq < 100000 );

	m_format = SoundFormat( freq, m_format.bitsPerSample(), m_format.channels() );
}

void Sound::setVolume( float vol )
{
	assert( m_sb );
	assert( vol >= -100.f && vol <= 0.f );

	m_vol = vol;
}

float Sound::volume() const
{
	assert( m_sb );
	return m_vol;
}

void Sound::setMaxDistance( float dist )
{
	assert( m_sb );
	m_sb->setMaxDistance( dist );
}

void Sound::setMinDistance( float dist )
{
	assert( m_sb );
	m_sb->setMinDistance( dist );
}

float Sound::maxDistance() const
{
	assert( m_sb );
	return m_sb->maxDistance();
}

float Sound::minDistance() const
{
	assert( m_sb );
	return m_sb->minDistance();
}

int Sound::frequency() const
{
	assert( m_sb );
	return m_format.samplesPerSec();
}

bool Sound::initialized() const
{
	return m_sb != 0;
}

void Sound::pause( bool enabled )
{
	if ( enabled != m_paused )
	{
		m_paused = enabled;

		if ( enabled )
		{
			m_sb->getCurrentPosition( &m_pausedPos, 0 );
			m_sb->stop();
		}
		else
		{
			m_sb->setCurrentPosition( m_pausedPos );
			m_sb->play( m_playFlags );
		}
	}
}

bool Sound::paused() const
{
	return m_paused;
}

String Sound::stateString() const
{
	String pauseStr = (m_paused ? "(PAUSED)" : "");
	String posStr = "";
	if ( SoundBuffer::USAGE_CONTROL3D & m_sb->usageFlags() )
	{
		Vector3 pos = m_sb->transform().translation();
		posStr = Format( "({0} {1} {2}) maxDistance={3}", pos.x, pos.y, pos.z, m_sb->maxDistance() ).format();
	}

	switch ( m_state )
	{
	case STATE_STOP:		
		return Format( "{0} STOP {1}", name(), pauseStr ).format();

	case STATE_PLAY:		
		return Format( "{0} PLAY {1} {2}", name(), pauseStr, posStr ).format();

	case STATE_FADEIN:		
		return Format( "{0} FADEIN {1} {2} {3}", name(), m_fade, pauseStr, posStr ).format();

	case STATE_FADEOUT:		
		return Format( "{0} FADEOUT {1} {2} {3}", name(), m_fade, pauseStr, posStr ).format();

	default:
		return Format( "{0} INVALID", name() ).format();
	}
}

int Sound::getDrvVolume( float dB )
{
	dB *= 100.f;
	if ( dB < -10000.f )
		dB = -10000.f;
	else if ( dB > 0.f )
		dB = 0.f;
	int v = (int)dB;
	return v;
}

float Sound::interpVolume( float u, float vol0, float vol1 )
{
	assert( vol0 <= 0.f );
	assert( vol1 <= 0.f );
	assert( u >= 0.f && u <= 1.f );

	float offset = vol0;
	if ( vol1 > vol0 )
		offset = vol1;

	vol0 -= offset;
	vol1 -= offset;
	float v0 = Math::log( 1.f - vol0 );
	float v1 = Math::log( 1.f - vol1 );
	float v = v0 + (v1-v0)*u;
	float vol = 1.f - Math::exp(v);
	vol += offset;
	return vol;
}


} // snd
