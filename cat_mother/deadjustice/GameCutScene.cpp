#include "GameCutScene.h"
#include "ProjectileManager.h"
#include <ps/ParticleSystem.h>
#include <ps/ParticleSystemManager.h>
#include <snd/Sound.h>
#include <snd/SoundManager.h>
#include <sgu/NodeUtil.h>
#include <sgu/SceneManager.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <math/lerp.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace ps;
using namespace snd;
using namespace sgu;
using namespace anim;
using namespace lang;
using namespace util;
using namespace math;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameCutScene> GameCutScene::sm_methods[] =
{
	//ScriptMethod<GamePlayer>( "funcName", script_funcName ),
	ScriptMethod<GameCutScene>( "endTime", script_endTime ),
	ScriptMethod<GameCutScene>( "playScene", script_playScene ),
	ScriptMethod<GameCutScene>( "playObjectParticleSystem", script_playObjectParticleSystem ),
	ScriptMethod<GameCutScene>( "playObjectSound", script_playObjectSound ),
	ScriptMethod<GameCutScene>( "setEndTime", script_setEndTime ),
	ScriptMethod<GameCutScene>( "setCutSceneCameraFrontAndBackPlanes", script_setCutSceneCameraFrontAndBackPlanes ),
	ScriptMethod<GameCutScene>( "time", script_time ),
};

//-----------------------------------------------------------------------------

GameCutScene::GameCutScene( script::VM* vm, io::InputStreamArchive* arch,
	sgu::SceneManager* sceneMgr, snd::SoundManager* soundMgr, 
	ps::ParticleSystemManager* particleMgr, ProjectileManager* projectileMgr ) :
	GameScriptable( vm, arch, soundMgr, particleMgr ),
	m_sceneMgr( sceneMgr ),
	m_projectileMgr( projectileMgr ),
	m_time( 0.f ),
	m_endTime( 0.f ),
	m_scene( 0 ),
	m_sceneTime( 0.f ),
	m_sceneObjects( Allocator<SceneObject>(__FILE__) ),
	m_cameraFront( 0.1f ),
	m_cameraBack( 5000.f )

{
	m_methodBase = ScriptUtil<GameCutScene,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

GameCutScene::~GameCutScene()
{
	removeSceneObjects();
}

void GameCutScene::update( float dt )
{
	GameScriptable::update( dt );

	if ( m_scene )
	{
		// update scene playback
		m_sceneTime += dt;
		for ( Node* node = m_scene ; node ; node = node->nextInHierarchy() )
			node->setState( m_sceneTime );

		// update scene objects
		for ( int i = 0 ; i < m_sceneObjects.size() ; ++i )
		{
			SceneObject& so = m_sceneObjects[i];
			so.obj->setTransform( so.parent->worldTransform() );
		}
	}

	m_time += dt;
	if ( ended() )
		m_scene = 0;
}

void GameCutScene::skip()
{
	removeSceneObjects();
	removeTimerEvents();

	pushMethod( "deinit" );
	call( 0, 0 );
}

bool GameCutScene::ended() const
{
	return m_time > m_endTime;
}

void GameCutScene::setTime( float time )
{
	m_time = time;
}

float GameCutScene::time() const
{
	return m_time;
}

float GameCutScene::cameraFront() const
{
	return m_cameraFront;
}

float GameCutScene::cameraBack() const
{
	return m_cameraBack;
}

void GameCutScene::removeSceneObjects()
{
	// stop sounds and particles
	for ( int i = 0 ; i < m_sceneObjects.size() ; ++i )
	{
		SceneObject& so = m_sceneObjects[i];

		Sound* sound = dynamic_cast<Sound*>( so.obj.ptr() );
		if ( sound )
			sound->stop();

		ParticleSystem* particleSystem = dynamic_cast<ParticleSystem*>( so.obj.ptr() );
		if ( particleSystem )
			particleSystem->kill();
	}
	m_sceneObjects.clear();
}

int GameCutScene::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<GameCutScene,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int GameCutScene::script_endTime( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns cut scene end", funcName) );

	vm->pushNumber( m_endTime );
	return 1;
}

int GameCutScene::script_time( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns cut scene time", funcName) );

	vm->pushNumber( m_time );
	return 1;
}

int GameCutScene::script_setEndTime( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects cut scene end time in seconds", funcName) );

	m_endTime = vm->toNumber(1);
	return 0;
}

int GameCutScene::script_playScene( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects scene file name (.sg included)", funcName) );

	String sceneName = vm->toString(1);
	m_scene = m_sceneMgr->getScene( sceneName, SceneFile::LOAD_ANIMATIONS );
	m_sceneTime = 0.f;
	removeSceneObjects();

	NodeUtil::setHierarchyPreBehaviour( m_scene, Interpolator::BEHAVIOUR_CONSTANT );
	NodeUtil::setHierarchyEndBehaviour( m_scene, Interpolator::BEHAVIOUR_CONSTANT );

	Debug::println( "cutscene.playScene( {0} )", sceneName );
	return 0;
}

int GameCutScene::script_playObjectParticleSystem( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects particle system name and object name", funcName) );
	
	if ( !m_scene )
		throw ScriptException( Format("{0} requires scene animation to be started before calling", funcName) );

	String	name			= vm->toString(1) + String(".psf");
	String	objectName		= vm->toString(2);
	Node*	parent			= NodeUtil::findNodeByName( m_scene, objectName );

	if ( !parent )
		throw ScriptException( Format("{0}: Object {1} not found from scene {2}", funcName, objectName, m_scene->name()) );

	P(ParticleSystem) obj = particleSystemManager()->play( name, 0 );
	obj->setTransform( parent->worldTransform() );

	SceneObject so;
	so.obj = obj.ptr();
	so.parent = parent;
	m_sceneObjects.add( so );
	return 0;
}

int GameCutScene::script_playObjectSound( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_STRING, VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects sound name and object name", funcName) );

	if ( !m_scene )
		throw ScriptException( Format("{0} requires scene animation to be started before calling", funcName) );

	String	name			= vm->toString(1) + String(".sf");
	String	objectName		= vm->toString(2);
	Node*	parent			= NodeUtil::findNodeByName( m_scene, objectName );

	if ( !parent )
		throw ScriptException( Format("{0}: Object {1} not found from scene {2}", funcName, objectName, m_scene->name()) );

	P(Sound) obj = soundManager()->play( name, 0 );
	obj->setTransform( parent->worldTransform() );

	SceneObject so;
	so.obj = obj.ptr();
	so.parent = parent;
	m_sceneObjects.add( so );
	return 0;
}

int	GameCutScene::script_setCutSceneCameraFrontAndBackPlanes( script::VM* vm, const char* funcName )
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects cut scene camera front and back plane distances", funcName) );

	m_cameraFront = vm->toNumber(1);
	m_cameraBack = vm->toNumber(2);
	return 0;
}
