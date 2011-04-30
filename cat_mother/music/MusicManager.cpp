#include "MusicManager.h"
#include "MusicPlayer.h"
#include "COMInitializer.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <util/Hashtable.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace music
{


class MusicManager::MusicManagerImpl :
	public Object
{
public:
	MusicManagerImpl() :
		m_COMInitializer(),
		m_tracks( Allocator< HashtablePair<String,P(MusicPlayer)> >(__FILE__) ),
		m_current( 0 ),
		m_fadeIn( 0 ),
		m_fade( 0 ),
		m_fadeTime( 0 ),
		m_normalVol( 0 ),
		m_state( STATE_STOP ),
		m_paused( false )
	{
	}

	~MusicManagerImpl()
	{
		for ( HashtableIterator<String,P(MusicPlayer)> it = m_tracks.begin() ; it != m_tracks.end() ; ++it )
		{
			if ( it.value() )
				it.value()->stop();
		}
	}

	void load( const String& name )
	{
		if ( !m_tracks[name] )
		{
			P(MusicPlayer) player = new MusicPlayer;
			player->setSourceFile( name );
			m_tracks[name] = player;
		}
	}

	void pause( bool enabled )
	{
		if ( m_paused != enabled )
		{
			if ( m_current )
				m_current->pause( enabled );
			if ( m_fadeIn )
				m_fadeIn->pause( enabled );
			m_paused = enabled;
		}
	}

	void update( float dt ) 
	{
		if ( m_paused )
			return;

		// update fade
		if ( m_fadeTime > 0.f )
		{
			m_fade += dt / m_fadeTime;
			if ( m_fade > 1.f )
				m_fade = 1.f;
		}

		// update music repeat
		if ( m_current && m_current->stopped() ) 
			m_current->play();
		if ( m_fadeIn && m_fadeIn->stopped() ) 
			m_fadeIn->play();

		// update state
		switch ( m_state )
		{
		case STATE_STOP:
			break;

		case STATE_PLAY:
			break;

		case STATE_FADEIN:{
			float vol0 = interpVolume( m_fade, m_normalVol, -100.f );
			float vol1 = interpVolume( m_fade, -100.f, m_normalVol );
			
			if ( m_current )
				m_current->setVolume( getDrvVolume(vol0) );
			m_fadeIn->setVolume( getDrvVolume(vol1) );

			if ( m_fade >= 1.f )
			{
				if ( m_current )
					m_current->stop();
				m_current = m_fadeIn;
				m_fadeIn = 0;
				m_state = STATE_PLAY;
			}
			break;}
		}
	}

	void play( const String& name ) 
	{
		load( name );
		if ( m_tracks[name] != m_current )
		{
			stop();
			m_current = m_tracks[name];
			m_current->setVolume( getDrvVolume(m_normalVol) );
			m_current->play();
			m_state = STATE_PLAY;
		}
	}

	void stop() 
	{
		if ( m_current )
		{
			m_current->stop();
			m_current = 0;
		}

		if ( m_fadeIn )
		{
			m_fadeIn->stop();
			m_fadeIn= 0;
		}

		m_fade = 0.f;
		m_fadeTime = 0.f;
		m_state = STATE_STOP;
	}

	void fadeIn( const String& name, float time ) 
	{
		if ( time < Float::MIN_VALUE )
		{
			play( name );
			return;
		}

		switch ( m_state )
		{
		case STATE_STOP:
		case STATE_PLAY:
			load( name );
			if ( m_tracks[name] != m_current )
			{
				m_fadeIn = m_tracks[name];
				m_fadeIn->setVolume( -10000 );
				m_fadeIn->play();
				m_fade = 0.f;
				m_fadeTime = time;
				m_state = STATE_FADEIN;
			}
			break;

		case STATE_FADEIN:
			load( name );
			if ( m_tracks[name] != m_current )
			{
				stop();
				m_fadeIn = m_tracks[name];
				m_fadeIn->setVolume( -10000 );
				m_fadeIn->play();
				m_fade = 0.f;
				m_fadeTime = time;
				m_state = STATE_FADEIN;
			}
			break;

		case STATE_FADEOUT:
			break;
		}
	}

	void fadeOut( float ) 
	{
		stop();
	}

	void setVolume( float vol )
	{
		m_normalVol = vol;
		if ( m_state == STATE_PLAY )
		{
			m_current->setVolume(getDrvVolume(m_normalVol));
		}
	}

	static float interpVolume( float u, float vol0, float vol1 )
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

	static int getDrvVolume( float dB )
	{
		dB *= 100.f;
		if ( dB < -10000.f )
			dB = -10000.f;
		else if ( dB > 0.f )
			dB = 0.f;
		int v = (int)dB;
		return v;
	}

private:
	enum State
	{
		STATE_STOP,
		STATE_PLAY,
		STATE_FADEIN,
		STATE_FADEOUT
	};

	COMInitializer						m_COMInitializer;
	Hashtable<String,P(MusicPlayer)>	m_tracks;
	P(MusicPlayer)						m_current;
	P(MusicPlayer)						m_fadeIn;
	float								m_fade;
	float								m_fadeTime;
	float								m_normalVol;
	State								m_state;
	bool								m_paused;

	MusicManagerImpl( const MusicManagerImpl& );
	MusicManagerImpl& operator=( const MusicManagerImpl& );
};

//-----------------------------------------------------------------------------

MusicManager::MusicManager()
{
	m_this = new MusicManagerImpl;
}

MusicManager::~MusicManager()
{
}


void MusicManager::update( float dt ) 
{
	m_this->update( dt );
}

void MusicManager::play( const String& name ) 
{
	m_this->play( name );
}

void MusicManager::stop() 
{
	m_this->stop();
}

void MusicManager::fadeIn( const String& name, float time ) 
{
	m_this->fadeIn( name, time );
}

void MusicManager::fadeOut( float time ) 
{
	m_this->fadeOut( time );
}

void MusicManager::pause( bool enabled )
{
	m_this->pause( enabled );
}

void MusicManager::setVolume( float vol )
{
	m_this->setVolume( vol );
}

float MusicManager::interpVolume( float u, float vol0, float vol1 )
{
	return MusicManager::MusicManagerImpl::interpVolume( u, vol0, vol1 );
}
/*
MusicManager::MusicManager()
{
}

MusicManager::~MusicManager()
{
}


void MusicManager::update( float dt ) 
{
}

void MusicManager::play( const String& name ) 
{
}

void MusicManager::stop() 
{
}

void MusicManager::fadeIn( const String& name, float time ) 
{
}

void MusicManager::fadeOut( float time ) 
{
}

void MusicManager::pause( bool enabled )
{
}

void MusicManager::setVolume( float vol )
{
}

float MusicManager::interpVolume( float u, float vol0, float vol1 )
{
	return MusicManager::MusicManagerImpl::interpVolume( u, vol0, vol1 );
}
*/


} // music
