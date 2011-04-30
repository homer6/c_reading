#ifndef _GAMENOISEMANAGER_H
#define _GAMENOISEMANAGER_H


#include "GameScriptable.h"
#include "GameNoise.h"
#include <util/Vector.h>


/** 
 * Manager of GameNoises. 
 * @see GameNoise
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameNoiseManager :
	public GameScriptable
{
public:
	GameNoiseManager( script::VM* vm, io::InputStreamArchive* arch );

	void		update( float dt );

	GameNoise*	createNoise( GameObject* source, float noiseLevel, 
					float fadeDistanceStart, float fadeDistanceEnd, 
					float fadeOutTimeStart, float fadeOutTimeEnd );

	void		removeNoisesBySource( GameObject* source );

	void		removeNoises();

	void		removeNoise( GameNoise* noise );

	int			noises() const;
	
	GameNoise*	getNoise( int i ) const;

private:
	int										m_methodBase;
	static ScriptMethod<GameNoiseManager>	sm_methods[];

	util::Vector< P(GameNoise) >			m_noises;
	util::Vector< P(GameNoise) >			m_noiseCache;

	int		methodCall( script::VM* vm, int i );
	int		script_noises( script::VM* vm, const char* funcName );
	int		script_createNoise( script::VM* vm, const char* funcName );

	GameNoiseManager( const GameNoiseManager& );
	GameNoiseManager& operator=( const GameNoiseManager& );
};


#include "GameNoiseManager.inl"


#endif // _GAMENOISEMANAGER_H
