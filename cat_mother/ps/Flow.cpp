#include "Flow.h"
#include <io/IOException.h>
#include <io/CommandReader.h>
#include <io/FileInputStream.h>
#include <io/InputStreamReader.h>
#include <io/InputStreamArchive.h>
#include <sg/Texture.h>
#include <lang/Float.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

namespace ps
{


/** Loads texture from archive or from file. */
static P(Texture) loadTex( const String& str, InputStreamArchive* zip )
{
	if ( zip )
	{
		P(InputStream) texin = zip->getInputStream( str );
		P(Texture) tex = new Texture( texin, str );
		texin->close();
		return tex;
	}
	else
	{
		P(InputStream) texin = new FileInputStream( str );
		P(Texture) tex = new Texture( texin, str );
		texin->close();
		return tex;
	}
}

//-----------------------------------------------------------------------------

Flow::Flow()
{
	startRadius		= 0.f;
	endRadius		= 0.f;
}

Flow::Flow( const lang::String& filename )
{
	P(FileInputStream) in = new FileInputStream( filename );
	load( in, 0 );
	in->close();
}

Flow::Flow( InputStream* in, InputStreamArchive* zip )
{
	load( in, zip );
}

Flow::Flow( const Flow& other ) :
	PathParticleSystem( other )
{
	startRadius = other.startRadius;
	endRadius	= other.endRadius;
	pathSource	= other.pathSource;
	pathTarget	= other.pathTarget;
}

void Flow::load( InputStream* in, InputStreamArchive* zip )
{
	Flow* ps = this;
	
	P(InputStreamReader) inreader = new InputStreamReader( in );
	P(CommandReader) reader = new CommandReader( inreader, in->toString() );

	ps->setParticleLifeTime( 100e3f );

	String str;
	while ( reader->readString(str) )
	{
		if ( str.startsWith("#") )
		{
			reader->readLine( str );
			continue;
		}
		else if ( str == "ObjectName" )
		{
			reader->readLine( str );
			ps->setName( str );
		}
		else if ( str == "EmissionRate" )
		{
			float emissionRate = reader->readFloat();
			ps->setEmissionRate( emissionRate );
		}
		else if ( str == "ParticleLifeTime" )
		{
			float particleLifeTime = reader->readFloat();
			ps->setParticleLifeTime( particleLifeTime );
		}
		else if ( str == "SystemLifeTime" )
		{
			float systemLifeTime = reader->readFloat();
			ps->setSystemLifeTime( systemLifeTime );
		}
		else if ( str == "MaxParticles" )
		{
			int maxParticles = reader->readInt();
			ps->setMaxParticles( maxParticles );
		}
		else if ( str == "Size" )
		{
			float minSize = reader->readFloat();
			float maxSize = reader->readFloat();
			ps->setParticleMinSize( minSize );
			ps->setParticleMaxSize( maxSize );
		}
		else if ( str == "Kill" )
		{
			reader->readString( str );
			KillType killType = ParticleSystem::KILL_RANDOM;
			if ( str == "RANDOM" )
				killType = ParticleSystem::KILL_RANDOM;
			else if ( str == "OLDEST" )
				killType = ParticleSystem::KILL_OLDEST;
			else if ( str == "NOTHING" )
				killType = ParticleSystem::KILL_NOTHING;
			ps->setKillType( killType );
		}
		else if ( str == "Image" )
		{
			reader->readLine( str );
			ps->setImage( loadTex(str,zip) );
		}
		else if ( str == "ImageAnim" )
		{
			reader->readString( str );
			P(Texture) tex = loadTex( str, zip );
			int rows = reader->readInt();
			int cols = reader->readInt();
			int frames = reader->readInt();
			float fps = reader->readFloat();
			reader->readString( str );
			BehaviourType end = BEHAVIOUR_LOOP;
			if ( str == "LOOP" )
				end = BEHAVIOUR_LOOP;
			else if ( str == "MIRROR" )
				end = BEHAVIOUR_MIRROR;
			else if ( str == "LIFE" )
				end = BEHAVIOUR_LIFE;
			else if ( str == "RANDOM" )
				end = BEHAVIOUR_RANDOM;
			ps->setImage( tex, rows, cols, frames, fps, end );
		}
		else if ( str == "ActivationTime" )
		{
			float t = reader->readFloat();
			Debug::println( "Particle system {0} uses deprecated command: ActivationTime {1}", in->toString(), t );
		}
		else if ( str == "Angle" )
		{
			float minAngle = Math::toRadians( reader->readFloat() );
			float maxAngle = Math::toRadians( reader->readFloat() );
			ps->setParticleMinRotation( minAngle );
			ps->setParticleMaxRotation( maxAngle );
		}
		else if ( str == "AngleSpeed" )
		{
			float minAngle = Math::toRadians( reader->readFloat() );
			float maxAngle = Math::toRadians( reader->readFloat() );
			ps->setParticleMinRotationSpeed( minAngle );
			ps->setParticleMaxRotationSpeed( maxAngle );
		}
		else if ( str == "Paths" )
		{
			int paths = reader->readInt();
			ps->setPaths( paths );
		}
		else if ( str == "Radius" )
		{
			ps->startRadius = reader->readFloat();
			ps->endRadius = reader->readFloat();
		}
		else if ( str == "SizeScale" )
		{
			float startScale = reader->readFloat();
			float endScale = reader->readFloat();
			ps->setParticleStartScale( startScale );
			ps->setParticleEndScale( endScale );
		}
		else if ( str == "Speed" )
		{
			float minSpeed = reader->readFloat();
			float maxSpeed = reader->readFloat();
			ps->setParticleMinSpeed( minSpeed );
			ps->setParticleMaxSpeed( maxSpeed );
		}
		else if ( str == "SequentialPathSelection" )
		{
			ps->setRandomPathSelection( false );
		}
		else if ( str == "PathSource" )
		{
			reader->readLine( ps->pathSource );
		}
		else if ( str == "PathTarget" )
		{
			reader->readLine( ps->pathTarget );
		}
		else
		{
			throw IOException( Format("Unknown flow command: {0}",str) );
		}
	}
}


} // ps
