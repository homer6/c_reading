#ifndef _GAMEWINDOW_H
#define _GAMEWINDOW_H


#include <win/FrameWindow.h>
#include <lang/DynamicLinkLibrary.h>


namespace io {
	class InputStreamArchive;}

namespace ps {
	class ParticleSystemManager;}

namespace sg {
	class Font;
	class Context;}

namespace sgu {
	class SceneManager; }

namespace snd {
	class SoundManager; }

namespace id {
	class InputDriver; }

namespace music {
	class MusicManager; }

namespace script {
	class VM;}

namespace util {
	class ExProperties; }

class Game;


/** 
 * Game main window. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameWindow :
	public win::FrameWindow
{
public:
	explicit GameWindow( io::InputStreamArchive* arch, util::ExProperties* cfg );
	~GameWindow();

	/** Called before main loop. */
	void		init( const char* wndTitle, HINSTANCE inst );

	/** Called after main loop. */
	void		deinit();

	/** Called when the window loses input focus. */
	void		focusLost();

	/** 
	 * Called between flushing messages in primary main loop. 
	 * @return false if quit requested.
	 */
	bool		update( float dt );

	/** Called after each update(). */
	void		render();

	LRESULT		handleMessage( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
	void		handleKeyDown( int key );

	/** Returns music manager. Valid only after init(). */
	music::MusicManager*	musicManager() const;

	/** Loads & initializes input driver. */
	void		initInputDriver();

	/** Deinitializes & unloads input driver. */
	void		deInitInputDriver();

private:
	lang::DynamicLinkLibrary		m_inputDrvDll;

	P(io::InputStreamArchive)		m_arch;
	P(util::ExProperties)			m_cfg;
	P(script::VM)					m_vm;
	P(sgu::SceneManager)			m_sceneMgr;
	P(ps::ParticleSystemManager)	m_particleMgr;
	P(snd::SoundManager)			m_soundMgr;
	P(music::MusicManager)			m_musicMgr;
	P(id::InputDriver)				m_inputDrv;
	P(sg::Context)					m_context;
	P(sg::Font)						m_dbgFont;
	bool							m_grabScreen;
	P(Game)							m_game;
	bool							m_quit;

	void	recompile();

	GameWindow( const GameWindow& );
	GameWindow& operator=( const GameWindow& );
};


#endif // _GAMEWINDOW_H
