#include "SoundManager.h"
#include "Sound.h"
#include "SoundLoader.h"
#include <sd/SoundDriver.h>
#include <sd/SoundDevice.h>
#include <sd/SoundBuffer.h>
#include <io/InputStreamArchive.h>
#include <lang/Float.h>
#include <lang/Exception.h>
#include <lang/DynamicLinkLibrary.h>
#include <util/Vector.h>
#include <util/Hashtable.h>
#include <math/Matrix4x4.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sd;
using namespace sg;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace snd
{


class SoundManager::SoundManagerImpl :
	public Object
{
public:
	class ActiveSound
	{
	public:
		enum State
		{
			STATE_INIT,
			STATE_PLAY
		};

		P(Sound)	obj;
		float		fadeTime;
		State		state;

		ActiveSound() : 
			obj(0), 
			fadeTime(0.f),
			state(STATE_INIT) 
		{
		}

		ActiveSound( Sound* s ) :
			obj(s),
			fadeTime(0.f),
			state(STATE_INIT) 
		{
		}
	};

	SoundManagerImpl( InputStreamArchive* arch ) :
		m_dll( "sd_dx8" ),
		m_drv( 0 ),
		m_dev( 0 ),
		m_arch( arch ),
		m_scene( 0 ),
		m_prototypes( Allocator< HashtablePair< String, P(Sound) > >(__FILE__) ),
		m_freed( Allocator< HashtablePair< String, P(Vector< P(Sound) >) > >(__FILE__) ),
		m_loadBuffer( Allocator<P(Sound>)(__FILE__) ),
		m_active( Allocator<ActiveSound>(__FILE__) ),
		m_soundLoader( 0 )
	{
		// init sound drv
		const char* drvname = "sd_dx8";
		createSoundDriverFunc createSoundDriver = (createSoundDriverFunc)m_dll.getProcAddress( "createSoundDriver" );
		if ( !createSoundDriver )
			throw Exception( Format("Corrupted sound library driver: {0}", drvname) );
		
		// check sound drv version
		getSoundDriverVersionFunc getSoundDriverVersion = (getSoundDriverVersionFunc)m_dll.getProcAddress( "getSoundDriverVersion" );
		if ( !getSoundDriverVersion )
			throw Exception( Format("Old sound library driver: {0}", drvname) );
		int ver = getSoundDriverVersion();
		if ( ver != SoundDriver::VERSION )
			throw Exception( Format("Wrong version ({1,#}) of the sound library driver: {0}", drvname, ver) );

		// init sound drv
		m_drv = (*createSoundDriver)();
		if ( !m_drv )
			throw Exception( Format("Failed to init sound library driver: {0}", drvname) );

		// init sound device
		m_dev = m_drv->createSoundDevice();
		int err = m_dev->create( 32, 44*1024, 16, 2 );
		if ( err )
			throw Exception( Format("Failed to init sound device") );

		// init sound loader
		m_soundLoader = new SoundLoader( m_drv, m_dev, arch );
	}

	~SoundManagerImpl()
	{
		clear();
	}

	void setScene( Node* scene ) 
	{
		m_scene = scene;
	}

	void update( Node* listener, float dt, float timeScale ) 
	{
		// update active
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			ActiveSound& active = m_active[i];
			P(Sound) obj = active.obj;

			obj->update( dt, timeScale );

			if ( active.state == ActiveSound::STATE_INIT )
			{
				if ( active.fadeTime > 0.f )
					obj->fadeIn( active.fadeTime );
				else
					obj->play();
				active.state = ActiveSound::STATE_PLAY;
			}
		}

		// remove inactive
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			ActiveSound& active = m_active[i];
			P(Sound) obj = active.obj;

			if ( !obj->playing() && !obj->paused() && active.state == ActiveSound::STATE_PLAY )
			{
				obj->unlink();
				m_freed[ obj->name() ]->add( obj );
				m_active.remove( i );
				--i;
			}
		}

		// get 3D sound listener properties from listener
		if ( listener )
			updateListener( listener, dt );

		// apply changes to device
		m_dev->commit();
	}

	void updateListener( Node* listener, float )
	{
		assert( listener );

		// update tm
		Matrix4x4 tm = listener->worldTransform();
		m_dev->setTransform( tm );
	}

	Sound* play( const String& name, Node* refobj, float fadeTime ) 
	{
		assert( m_scene );

		// init type
		load( name );
		P( Vector< P(Sound) > ) freed = m_freed[name];

		// create instance
		P(Sound) obj = 0;
		if ( freed->isEmpty() )
		{
			// create new
			obj = static_cast<Sound*>( m_prototypes[name]->clone() );
			obj->setName( name );
		}
		else
		{
			// reset old
			obj = freed->lastElement();
			freed->setSize( freed->size()-1 );
			obj->stop();
		}

		// link to anchor
		if ( refobj )
			obj->linkTo( refobj );
		else if ( m_scene )
			obj->linkTo( m_scene );

		// activate
		ActiveSound active( obj );
		active.fadeTime = fadeTime;
		m_active.add( active );
		return obj;
	}

	void stop( const String& name, Node* refobj, float fadeTime ) 
	{
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			ActiveSound& active = m_active[i];
			P(Sound) obj = active.obj;

			if ( obj->name() == name && (!refobj || refobj == obj->parent()) )
			{
				if ( fadeTime > 0.f )
					obj->fadeOut( fadeTime );
				else
					obj->stop();
			}
		}
	}

	void setListenerParam( float distanceFactor, float dopplerFactor, float rolloffFactor )
	{
		assert( m_dev );

		m_dev->setDistanceFactor( distanceFactor );
		m_dev->setDopplerFactor( dopplerFactor );
		m_dev->setRolloffFactor( rolloffFactor );
	}

	void pauseActive( bool enabled )
	{
		for ( int i = 0 ; i < m_active.size() ; ++i )
			m_active[i].obj->pause( enabled );
	}

	void removeActive()
	{
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			m_active[i].obj->stop();
			m_active[i].obj->unlink();
		}

		m_active.clear();
	}

	void removeFreedSounds()
	{
		for ( HashtableIterator< String, P(Vector< P(Sound) >) > it = m_freed.begin() ; it != m_freed.end() ; ++it )
		{
			P(Vector< P(Sound) >) buf = it.value();
			if ( buf )
			{
				for ( int i = 0 ; i < buf->size() ; ++i )
					(*buf)[i]->unlink();
				buf->clear();
			}
		}
		m_freed.clear();
	}

	void removePrototypes()
	{
		for ( HashtableIterator< String, P(Sound) > it = m_prototypes.begin() ; it != m_prototypes.end() ; ++it )
		{
			P(Sound) obj = it.value();
			if ( obj )
				obj->unlink();
		}
		m_freed.clear();
	}

	void clearLoadBuffer()
	{
		for ( int i = 0 ; i < m_loadBuffer.size() ; ++i )
			m_loadBuffer[i]->unlink();
		m_loadBuffer.clear();
	}

	void clear()
	{
		removeActive();
		clearLoadBuffer();
		removeFreedSounds();
		removePrototypes();
	}

	void setVolume( float v )
	{
		assert( m_dev );

		m_dev->setVolume( v );
	}

	int getActiveCount( const String& name, Node* refobj ) const
	{
		if ( !refobj )
			refobj = m_scene;

		int count = 0;
		for ( int i = 0 ; i < m_active.size() ; ++i )
		{
			const ActiveSound& active = m_active[i];
			if ( active.obj->name() == name && refobj == active.obj->parent() )
				++count;
		}
		return count;
	}
	
	void load( const String& name )
	{
		if ( !m_freed[name] )
		{
			clearLoadBuffer();
			m_soundLoader->loadSounds( name, m_loadBuffer );

			for ( int i = 0 ; i < m_loadBuffer.size() ; ++i )
			{
				P(Sound) obj = m_loadBuffer[i];
				m_prototypes[obj->name()] = obj;
				m_freed[obj->name()] = new Vector< P(Sound) >( Allocator<P(Sound)>(__FILE__) );
			}

			// add also template container if multiple were loaded
			if ( m_loadBuffer.size() > 1 )
				m_freed[name] = new Vector< P(Sound) >( Allocator<P(Sound)>(__FILE__) );
		}
	}

	Sound* getActiveSound( int i ) const
	{
		return m_active[i].obj;
	}

	int	activeSounds() const
	{
		return m_active.size();
	}

private:
	DynamicLinkLibrary							m_dll;
	P(SoundDriver)								m_drv;
	P(SoundDevice)								m_dev;
	P(InputStreamArchive)						m_arch;
	P(Node)										m_scene;
	Hashtable< String, P(Sound) >				m_prototypes;
	Hashtable< String, P(Vector< P(Sound) >) >	m_freed;
	Vector<P(Sound)>							m_loadBuffer;
	Vector<ActiveSound>							m_active;
	P(SoundLoader)								m_soundLoader;

	SoundManagerImpl( const SoundManagerImpl& );
	SoundManagerImpl& operator=( const SoundManagerImpl& );
};

//-----------------------------------------------------------------------------

SoundManager::SoundManager( InputStreamArchive* arch )
{
	m_this = new SoundManagerImpl( arch );
}

SoundManager::~SoundManager() 
{
}

void SoundManager::setScene( Node* scene ) 
{
	m_this->setScene( scene );
}

void SoundManager::update( Node* listener, float dt, float timeScale ) 
{
	m_this->update( listener, dt, timeScale );
}

void SoundManager::load( const lang::String& name )
{
	m_this->load( name );
}

Sound* SoundManager::play( const String& name, Node* refobj ) 
{
	return m_this->play( name, refobj, 0.f );
}

Sound* SoundManager::fadeIn( const String& name, Node* refobj, float fadeTime ) 
{
	return m_this->play( name, refobj, fadeTime );
}

void SoundManager::fadeOut( const String& name, Node* refobj, float fadeTime ) 
{
	m_this->stop( name, refobj, fadeTime );
}

void SoundManager::stop( const String& name, Node* refobj ) 
{
	m_this->stop( name, refobj, 0.f );
}

void SoundManager::clear() 
{
	m_this->clear();
}

void SoundManager::setVolume( float v )
{
	m_this->setVolume( v );
}

void SoundManager::removeActive()
{
	m_this->removeActive();
}

void SoundManager::pauseActive( bool enabled )
{
	m_this->pauseActive( enabled );
}

void SoundManager::setListenerParam( float distanceFactor, float dopplerFactor, float rolloffFactor )
{
	m_this->setListenerParam( distanceFactor, dopplerFactor, rolloffFactor );
}

int SoundManager::getActiveCount( const String& name, Node* refobj ) const
{
	return m_this->getActiveCount( name, refobj );
}

Sound* SoundManager::getActiveSound( int i ) const
{
	return m_this->getActiveSound( i );
}

int	SoundManager::activeSounds() const
{
	return m_this->activeSounds();
}

void SoundManager::destroy()
{
	m_this = 0;
}


} // snd
