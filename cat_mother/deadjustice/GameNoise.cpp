#include "GameNoise.h"
#include "GameCharacter.h"
#include "ScriptUtil.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <math/lerp.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameNoise> GameNoise::sm_methods[] =
{
	//ScriptMethod<GameNoise>( "funcName", script_funcName ),
	ScriptMethod<GameNoise>( "getSource", script_getSource ),
};

GameNoise::GameNoise( script::VM* vm, io::InputStreamArchive* arch ) :
	GameObject( vm, arch, 0, 0, 0 ),
	m_source( 0 ),
	m_noiseLevel( 0.f ),
	m_fadeDistanceStart( 0.f ),
	m_fadeDistanceEnd( 0.f ),
	m_fadeOutTimeStart( 0.f ),
	m_fadeOutTimeEnd( 0.f ),
	m_time( 0.f )
{
	m_methodBase = ScriptUtil<GameNoise,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

void GameNoise::init( GameObject* source, float noiseLevel, 
	float fadeDistanceStart, float fadeDistanceEnd,
	float fadeOutTimeStart, float fadeOutTimeEnd )
{
	m_source					= source;
	m_noiseLevel				= noiseLevel;
	m_fadeDistanceStart			= fadeDistanceStart;
	m_fadeDistanceEnd			= fadeDistanceEnd;
	m_fadeOutTimeStart			= fadeOutTimeStart;
	m_fadeOutTimeEnd			= fadeOutTimeEnd;
	m_time						= 0.f;
	m_fadeDistanceEndSquared	= fadeDistanceEnd*fadeDistanceEnd;

	setPosition( source->cell(), source->position() );
}

void GameNoise::deinit()
{
	m_source = 0;
	setPosition( 0, Vector3(0,0,0) );
}

void GameNoise::update( float dt )
{
	GameObject::update( dt );
	m_time += dt;
}

void GameNoise::setNoiseLevel( float v )
{
	m_noiseLevel = v;
}

void GameNoise::setFadeDistance( float start, float end )
{
	m_fadeDistanceStart = start;
	m_fadeDistanceEnd = end;
	m_fadeDistanceEndSquared = end*end;
}

void GameNoise::setFadeOutTime( float start, float end )
{
	m_fadeOutTimeStart = start;
	m_fadeOutTimeEnd = end;
}

float GameNoise::getLevelAt( const Vector3& hearingPos ) const
{
	float level = 0.f;
	float distanceSqr = (position() - hearingPos).lengthSquared();
	if ( distanceSqr < m_fadeDistanceEndSquared )
	{
		level = m_noiseLevel;
		float distance = Math::sqrt( distanceSqr );
		level *= lerp( 1.f, 0.f, (distance - m_fadeDistanceStart)/Math::max(m_fadeDistanceEnd - m_fadeDistanceStart,1e-6f) );
		level *= lerp( 1.f, 0.f, (m_time - m_fadeOutTimeStart)/Math::max(m_fadeOutTimeEnd - m_fadeOutTimeStart,1e-6f) );
	}
	return level;
}

int GameNoise::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<GameNoise, GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );	
}

int GameNoise::script_getSource( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns noise source", funcName) );

	vm->pushTable( m_source );
	return 1;
}

