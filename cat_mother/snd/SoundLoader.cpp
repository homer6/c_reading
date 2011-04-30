#include "Sound.h"
#include "SoundLock.h"
#include "SoundLoader.h"
#include <sd/SoundBuffer.h>
#include <io/File.h>
#include <io/IOException.h>
#include <io/InputStream.h>
#include <io/CommandReader.h>
#include <io/InputStreamReader.h>
#include <io/InputStreamArchive.h>
#include <sd/SoundDriver.h>
#include <sd/SoundDevice.h>
#include <snd/SoundFile.h>
#include <snd/SoundFormat.h>
#include <lang/Debug.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sd;
using namespace sg;
using namespace snd;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace snd
{


SoundLoader::SoundLoader( sd::SoundDriver* drv, sd::SoundDevice* dev, io::InputStreamArchive* arch ) :
	m_sounds( Allocator< HashtablePair< lang::String, P(Sound) > >(__FILE__) ),
	m_arch( arch ),
	m_drv( drv ),
	m_dev( dev )
{
}

SoundLoader::~SoundLoader()
{
}

void SoundLoader::loadSounds( const lang::String& name, Vector<P(Sound)>& sounds )
{
	// read description file commands
	P(InputStream) in = m_arch->getInputStream( name );
	P(InputStreamReader) inreader = new InputStreamReader( in );
	P(CommandReader) reader = new CommandReader( inreader, in->toString() );

	int		soundCount		= 1;
	String	nameFmt			= name;
	bool	waveLoaded		= false;
	int		soundUsageFlags = Sound::USAGE_STATIC;

	String str;
	while ( reader->readString(str) )
	{
		// comment?
		if ( str.startsWith("#") )
		{
			reader->readLine( str );
			continue;
		}

		// commands before WaveFile

		if ( str == "Count" )
		{
			// multiple sounds to be loaded from single description
			if ( waveLoaded )
				throw IOException( Format("Sound {0} command {1} must be before WaveFile command", name, str) );
			
			if ( !reader->readString( nameFmt ) )
				throw IOException( Format("Sound {0} command Count number must be after name format string", name) );

			soundCount = reader->readInt();
			if ( soundCount < 1 || soundCount > 100 )
				throw IOException( Format("Sound {0} command Count must be in range [1,100], was {1}", name, soundCount) );
			continue;
		}
		else if ( str == "Control3D" )
		{
			// 3D audio
			if ( waveLoaded )
				throw IOException( Format("Sound {0} command {1} must be before WaveFile command", name, str) );

			soundUsageFlags |= Sound::USAGE_CONTROL3D;
			continue;
		}
		else if ( str == "WaveFile" )
		{
			reader->readLine( str );
			for ( int i = 0 ; i < soundCount ; ++i )
			{
				// use template sound name and index [1,n] to format actual sound name
				String fname = Format( str, i+1 ).format();
				String objName = Format( nameFmt, i+1 ).format();
				P(Sound) obj = 0;
				loadWave( fname, soundUsageFlags, &obj );
				obj->setName( objName );
				sounds.add( obj );
			}
			waveLoaded = true;
			continue;
		}

		// commands after WaveFile

		if ( !waveLoaded )
			throw IOException( Format("Sound {0} command WaveFile must be before command {1}", name, str) );

		if ( str == "Frequency" )
		{
			int freq = reader->readInt();
			if ( freq < 100 || freq > 100000 )
				throw IOException( Format("Sound {0} command Frequency not in valid range [100,100000]", name) );

			for ( int i = 0 ; i < sounds.size() ; ++i )
			{
				if ( !sounds[i]->initialized() )
					throw IOException( Format("Sound {0} command WaveFile must be before Frequency", name) );

				sounds[i]->setFrequency( freq );
			}
		}
		else if ( str == "Looping" )
		{
			bool looping = ( 0 != reader->readInt() );

			for ( int i = 0 ; i < sounds.size() ; ++i )
			{
				if ( !sounds[i]->initialized() )
					throw IOException( Format("Sound command WaveFile must be before Looping in {0}", name) );

				sounds[i]->setLooping( looping );
			}
		}
		else if ( str == "Volume" )
		{
			float v = reader->readFloat();
			if ( v < -100.f || v > 0.f )
				throw IOException( Format("Sound {0} command Volume {1} out of range [-100,0]", name, v) );

			for ( int i = 0 ; i < sounds.size() ; ++i )
			{
				if ( !sounds[i]->initialized() )
					throw IOException( Format("Sound command WaveFile must be before Volume in {0}", name) );

				sounds[i]->setVolume( v );
			}
		}
		else if ( str == "MaxDistance" )
		{
			float v = reader->readFloat();
			if ( v < 0.f || v > 1e10f )
				throw IOException( Format("Sound {0} command MaxDistance {1} out of range [0,1e10]", name, v) );

			for ( int i = 0 ; i < sounds.size() ; ++i )
			{
				if ( !sounds[i]->initialized() )
					throw IOException( Format("Sound command WaveFile must be before MaxDistance in {0}", name) );

				sounds[i]->setMaxDistance( v );
			}
		}
		else if ( str == "MinDistance" )
		{
			float v = reader->readFloat();
			if ( v < 0.f || v > 1e10f )
				throw IOException( Format("Sound {0} command MinDistance {1} out of range [0,1e10]", name, v) );

			for ( int i = 0 ; i < sounds.size() ; ++i )
			{
				if ( !sounds[i]->initialized() )
					throw IOException( Format("Sound command WaveFile must be before MinDistance in {0}", name) );

				sounds[i]->setMinDistance( v );
			}
		}
		else
		{
			throw IOException( Format("Unknown sound command: {0}",str) );
		}
	}

	in->close();
}

void SoundLoader::loadWave( const String& fname, int soundUsageFlags, P(Sound)* sound )
{
	String name = File(fname).getName();
	if ( m_sounds[name] )
	{
		Debug::println( "Instancing wave {0}", name );
		*sound = static_cast<Sound*>( m_sounds[name]->clone() );
	}
	else
	{
		SoundFile soundFile( fname, m_arch );
		SoundFormat soundFormat = soundFile.format();

		P(Sound) obj = new Sound( m_drv, m_dev, soundFile.size(), soundFormat, soundUsageFlags );

		void* data = 0;
		int bytes = 0;
		SoundLock lk( obj, 0, soundFile.size(), &data, &bytes, 0, 0 );
		soundFile.read( data, bytes );

		m_sounds[name] = obj;
		*sound = obj;
	}
}


} // snd
