#include "GameMusic.h"
#include "ScriptUtil.h"
#include <util/ExProperties.h>
#include <music/MusicManager.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace lang;
using namespace music;
using namespace script;

//-----------------------------------------------------------------------------

const String GameMusic::MUSIC_PATH = "data/music/";

ScriptMethod<GameMusic> GameMusic::sm_methods[] =
{
	//ScriptMethod<GamePlayer>( "funcName", script_funcName ),
	ScriptMethod<GameMusic>( "fadeIn", script_fadeIn ),
	ScriptMethod<GameMusic>( "play", script_play ),
	ScriptMethod<GameMusic>( "stop", script_stop ),
	ScriptMethod<GameMusic>( "setVolume", script_setVolume ),
};

//-----------------------------------------------------------------------------

GameMusic::GameMusic( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, music::MusicManager* musicMgr, util::ExProperties* cfg ) :
	GameScriptable( vm, arch, soundMgr, 0 ),
	m_musicMgr( musicMgr )
{
	m_methodBase = ScriptUtil<GameMusic,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
	m_maxVolume = cfg->getFloat("Music.MaxVolume");
}

GameMusic::~GameMusic()
{
	m_musicMgr->stop();
}

void GameMusic::update( float dt ) 
{
	GameScriptable::update( dt );
	m_musicMgr->update( dt );
}

int GameMusic::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<GameMusic,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

Node* GameMusic::getRenderObject( Camera* ) 
{
	return 0;
}

int GameMusic::script_play( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects music file name", funcName) );
	
	String fname = vm->toString(1);
	m_musicMgr->play( MUSIC_PATH + fname );
	return 0;
}

int GameMusic::script_fadeIn( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects music file name and fade time", funcName) );

	String	fname	= vm->toString(1);
	float	time	= vm->toNumber(2);

	m_musicMgr->fadeIn( MUSIC_PATH + fname, time );
	return 0;
}

int GameMusic::script_stop( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} stops playing music", funcName) );

	m_musicMgr->stop();
	return 0;
}

int GameMusic::script_setVolume( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 || vm->isNumber(1) )
		throw ScriptException( Format("{0} sets music volume, pass 1 parameter", funcName) );
	
	float volume = vm->toNumber(1);
	float musicvolume = MusicManager::interpVolume( volume / 100.f, -100.f, m_maxVolume );
	m_musicMgr->setVolume(musicvolume);
	return 0;
}
