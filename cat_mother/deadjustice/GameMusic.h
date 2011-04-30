#ifndef _GAMEMUSIC_H
#define _GAMEMUSIC_H


#include "GameScriptable.h"
#include <util/Vector.h>


namespace ps {
	class ParticleSystemManager;}

namespace sgu {	
	class SceneManager;}

namespace util {
	class ExProperties;}

namespace music {
	class MusicManager;}

namespace snd {
	class SoundManager;}


/** 
 * Background MP3 music player. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameMusic :
	public GameScriptable
{
public:
	static const lang::String MUSIC_PATH;

	GameMusic( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, music::MusicManager* musicMgr, util::ExProperties* cfg );

	~GameMusic();

	/** 
	 * Update music playback.
	 * @exception Exception
	 */
	void		update( float dt );

	/**
	 * When a (C++) method is called from a script, this function
	 * is executed and unique method identifier is passed as parameter.
	 * Derived classes must override this if they add new scriptable methods.
	 * @param vm Script virtual machine executing the method.
	 * @param i Unique identifier (index) of the called method.
	 * @return Number of arguments returned in the script stack.
	 */
	int			methodCall( script::VM* vm, int i );

	/** Returns object to be used in rendering. */
	sg::Node*	getRenderObject( sg::Camera* camera );

private:
	P(music::MusicManager)			m_musicMgr;
	float							m_maxVolume;

	// scripting
	int								m_methodBase;
	static ScriptMethod<GameMusic>	sm_methods[];

	int		script_play( script::VM* vm, const char* funcName );
	int		script_fadeIn( script::VM* vm, const char* funcName );
	int		script_stop( script::VM* vm, const char* funcName );
	int		script_setVolume( script::VM* vm, const char* funcName );

	GameMusic( const GameMusic& );
	GameMusic& operator=( const GameMusic& );
};


#endif // _GAMEMUSIC_H
