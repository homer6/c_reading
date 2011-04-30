#ifndef _GAMEBOXTRIGGER_H
#define _GAMEBOXTRIGGER_H


#include "GameObject.h"


/** 
 * Box-shaped trigger. Triggers call specific script function when some object collides against them.
 * Types of objects that can collide against specific trigger can be controlled with CollisionInfo collision masks.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameBoxTrigger :
	public GameObject
{
public:
	GameBoxTrigger( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr );

	~GameBoxTrigger();

	/** Updates trigger affected object list. */
	void					update( float dt );

	/** Sets box dimensions. */
	void					setDimensions( const math::Vector3& dim );

	/** 
	 * Registers object to be affected by the trigger. 
	 * @return true if object was not registered before.
	 */
	bool					addAffectedObject( GameObject* obj );

	/** Returns trigger dimensions. */
	const math::Vector3&	dimensions() const;

private:
	// scripting
	int									m_methodBase;
	static ScriptMethod<GameBoxTrigger>	sm_methods[];

	math::Vector3						m_dim;
	P(util::Vector<GameObject*>)		m_affectedObjects;
	P(util::Vector<GameObject*>)		m_previousAffectedObjects;

	int		methodCall( script::VM* vm, int i );
	/** Sets trigger dimensions. */
	int		script_setDimensions( script::VM* vm, const char* funcName );
	/** Returns dimenions against ith trigger axis. */
	int		script_getDimensions( script::VM* vm, const char* funcName );

	GameBoxTrigger( const GameBoxTrigger& );
	GameBoxTrigger& operator=( const GameBoxTrigger& );
};


#endif // _GAMEBOXTRIGGER_H
