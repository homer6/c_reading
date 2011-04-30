#include "GameNoiseManager.h"
#include "GameObject.h"
#include "ScriptUtil.h"
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;
using namespace util;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameNoiseManager> GameNoiseManager::sm_methods[] =
{
	ScriptMethod<GameNoiseManager>( "noises", script_noises )
};

//-----------------------------------------------------------------------------

GameNoiseManager::GameNoiseManager( VM* vm, io::InputStreamArchive* arch ) :
	GameScriptable( vm, arch, 0, 0 ),
	m_methodBase( -1 ),
	m_noises( Allocator<P(GameNoise)>(__FILE__) ),
	m_noiseCache( Allocator<P(GameNoise)>(__FILE__) )
{
	m_methodBase = ScriptUtil<GameNoiseManager,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

void GameNoiseManager::update( float dt )
{
	GameScriptable::update( dt );

	// remove old
	for ( int i = 0 ; i < m_noises.size() ; ++i )
	{
		if ( m_noises[i]->ended() )
		{
			P(GameNoise) noise = m_noises[i];
			noise->deinit();
			m_noiseCache.add( noise );
			m_noises.remove( i-- );
		}
	}

	// update
	for ( int i = 0 ; i < m_noises.size() ; ++i )
		m_noises[i]->update( dt );
}

GameNoise* GameNoiseManager::createNoise( GameObject* source, float noiseLevel,
	float fadeDistanceStart, float fadeDistanceEnd, 
	float fadeOutTimeStart, float fadeOutTimeEnd )
{
	assert( !source || source->noiseManager() == this );

	P(GameNoise) noise = 0;
	if ( m_noiseCache.size() > 0 )
	{
		noise = m_noiseCache.lastElement();
		m_noiseCache.remove( m_noiseCache.size()-1 );
	}
	else
	{
		noise = new GameNoise( vm(), archive() );
	}

	noise->init( source, noiseLevel, fadeDistanceStart, fadeDistanceEnd, fadeOutTimeStart, fadeOutTimeEnd );
	m_noises.add( noise );
	return noise;
}

void GameNoiseManager::removeNoisesBySource( GameObject* source )
{
	for ( int i = 0 ; i < m_noises.size() ; )
	{
		if ( m_noises[i]->noiseSource() == source )
		{
			m_noises.remove( i );
		}
		else
		{
			++i;
		}
	}
}

void GameNoiseManager::removeNoises()
{
	m_noises.clear();
	m_noiseCache.clear();
}

void GameNoiseManager::removeNoise( GameNoise* noise )
{
	int i = m_noises.indexOf( noise );
	if ( i != -1 )
	{
		noise->deinit();
		m_noiseCache.add( noise );
		m_noises.remove( i );
	}
}

int	GameNoiseManager::noises() const
{
	return m_noises.size();
}

int GameNoiseManager::methodCall( VM* vm, int i )
{
	return ScriptUtil<GameNoiseManager,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	GameNoiseManager::script_noises( VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns number of active noises", funcName) );

	vm->pushNumber( (float)noises() );
	return 1;
}

int	GameNoiseManager::script_createNoise( VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_TABLE, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects noise source game object, noise level, fade distance start, fade distance end, fade out time start, fade out time end", funcName) );

	int param = 1;
	GameObject*		source				= dynamic_cast<GameObject*>( getThisPtr(vm, param++) );
	float			noiseLevel			= vm->toNumber( param++ );
	float			fadeDistanceStart	= vm->toNumber( param++ );
	float			fadeDistanceEnd		= vm->toNumber( param++ );
	float			fadeOutTimeStart	= vm->toNumber( param++ );
	float			fadeOutTimeEnd		= vm->toNumber( param++ );

	if ( !source )
		throw ScriptException( Format("{0} expects noise source game object, noise level, fade distance start, fade distance end, fade out time start, fade out time end", funcName) );

	GameNoise* noise = createNoise( source, noiseLevel, fadeDistanceStart, fadeDistanceEnd, fadeOutTimeStart, fadeOutTimeEnd );
	vm->pushTable( noise );
	return 1;
}
