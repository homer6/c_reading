#ifndef _GAME_H
#define _GAME_H


#include <lang/Object.h>


namespace id {
	class InputDriver;}

namespace io {
	class InputStreamArchive;}

namespace sg {
	class Font;
	class Context;}

namespace sgu {
	class SceneManager;}

namespace snd {
	class SoundManager;}

namespace music {
	class MusicManager;}

namespace ps {
	class ParticleSystemManager;}

namespace script {
	class VM;}

namespace util {
	class ExProperties; }

class GameCamera;
class GameLevel;


/**
 * High level game class.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Game :
	public lang::Object
{
public:
	Game( script::VM* vm, io::InputStreamArchive* arch, sg::Context* context, sg::Font* dbgFont, util::ExProperties* cfg,
		snd::SoundManager*, ps::ParticleSystemManager*, music::MusicManager*, sgu::SceneManager*, id::InputDriver* inputDriver );

	~Game();

	/** Called at exit. */
	void	destroy();

	/** Called between rendered frames. */
	void	update( float dt );

	/** Called to render single frame. */
	void	render();

	/** Set true if arc ball camera is enabled. */
	void	setArcBallCameraEnabled( bool enabled );

	/** Called when window focus is lost. */
	void	focusLost();

	/** Sets active camera. */
	void	selectActiveCamera( int n );

	/** Sets main character invulnerable. */
	void	setInvulnerable();

	/** Flushes input. */
	void	resetInputState();

	/** Skips notice screen (displayed after loading). */
	void	skipNoticeScreen();

	/** Returns true if arc ball camera is enabled. */
	bool	arcBallCameraEnabled() const;

	/** Returns active camera if any. */
	GameCamera*	activeCamera() const;

	/** Returns current level. */
	GameLevel*	level() const;

private:
	class Impl;
	P(Impl) m_this;

	Game( const Game& );
	Game& operator=( const Game& );
};


#endif // _GAME_H
