#include "GameBoxTrigger.h"
#include "ScriptUtil.h"
#include <assert.h>
#include <math/Intersection.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GameBoxTrigger> GameBoxTrigger::sm_methods[] =
{
	//ScriptMethod<GameBoxTrigger>( "funcName", script_funcName ),
	ScriptMethod<GameBoxTrigger>( "getDimensions", script_getDimensions ),
	ScriptMethod<GameBoxTrigger>( "setDimensions", script_setDimensions ),
};

//-----------------------------------------------------------------------------

GameBoxTrigger::GameBoxTrigger( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr ) :
	GameObject( vm, arch, soundMgr, 0, 0 ),
	m_methodBase( -1 ),
	m_dim( 0, 0, 0 ),
	m_affectedObjects( new Vector<GameObject*>( Allocator<GameObject*>(__FILE__) ) ),
	m_previousAffectedObjects( new Vector<GameObject*>( Allocator<GameObject*>(__FILE__) ) )
{
	m_methodBase = ScriptUtil<GameBoxTrigger,GameObject>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
	setCollidable( true );
}

GameBoxTrigger::~GameBoxTrigger()
{
}

void GameBoxTrigger::update( float dt )
{
	GameObject::update( dt );

	P(Vector<GameObject*>) vec = m_affectedObjects;
	m_affectedObjects = m_previousAffectedObjects;
	m_affectedObjects->clear();
	m_previousAffectedObjects = vec;
}

void GameBoxTrigger::setDimensions( const math::Vector3& dim )
{
	m_dim = dim;
}

const Vector3& GameBoxTrigger::dimensions() const
{
	return m_dim;
}

bool GameBoxTrigger::addAffectedObject( GameObject* obj )
{
	bool newAffectedObject = false;

	if ( m_affectedObjects->indexOf(obj) == -1 )
		m_affectedObjects->add( obj );

	if ( m_previousAffectedObjects->indexOf(obj) == -1 )
	{
		m_previousAffectedObjects->add( obj );
		newAffectedObject = true;
	}

	return newAffectedObject;
}

int GameBoxTrigger::methodCall( script::VM* vm, int i )
{
	return ScriptUtil<GameBoxTrigger,GameObject>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int	GameBoxTrigger::script_setDimensions( script::VM* vm, const char* funcName )
{
	float v[3];
	int retv = getParams( vm, funcName, "dimensions (x,y,z)", v, 3 );
	m_dim = Vector3( v[0], v[1], v[2] );
	setBoundSphere( m_dim.length() );
	return retv;
}

int	GameBoxTrigger::script_getDimensions( script::VM* vm, const char* funcName )
{
	vm->pushNumber( m_dim[ getIndex(vm,funcName,0,3) ] );
	return 1;
}

