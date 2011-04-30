#ifndef _OVERLAYDISPLAY_H
#define _OVERLAYDISPLAY_H


#include "GameScriptable.h"
#include "OverlayFont.h"
#include "OverlayBitmap.h"
#include <util/Vector.h>


namespace sg {
	class TriangleList;
	class Camera;
	class Mesh;}

namespace snd {
	class SoundManager; }

class GameController;


/** 
 * On-screen info contains 2D bitmap sprites and text.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class OverlayDisplay :
	public GameScriptable
{
public:
	OverlayDisplay( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, GameController* gamecontroller );

	~OverlayDisplay();

	/** 
	 * Update overlays. 
	 * @exception Exception
	 * @exception ScriptException
	 */
	void		update( float dt );

	/** Renders overlay items. */
	void		render();

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
	// scripting
	int									m_methodBase;
	static ScriptMethod<OverlayDisplay>	sm_methods[];

	// script independent variables
	P(io::InputStreamArchive)		m_arch;
	P(GameController)				m_gameController;

	// script dependent variables
	bool							m_enabled;
	bool							m_paused;
	float							m_time;
	float							m_dt;
	util::Vector<P(OverlayFont)>	m_fonts;
	util::Vector<P(OverlayBitmap)>	m_bitmaps;
	P(sg::TriangleList)				m_tri;

	int		script_clear( script::VM* vm, const char* funcName );
	int		script_createBitmap( script::VM* vm, const char* funcName );
	int		script_createFont( script::VM* vm, const char* funcName );
	int		script_enabled( script::VM* vm, const char* funcName );
	int		script_format( script::VM* vm, const char* funcName );
	int		script_height( script::VM* vm, const char* funcName );
	int		script_pause( script::VM* vm, const char* funcName );
	int		script_setEnabled( script::VM* vm, const char* funcName );
	int		script_setPause( script::VM* vm, const char* funcName );
	int		script_setTime( script::VM* vm, const char* funcName );
	int		script_time( script::VM* vm, const char* funcName );
	int		script_width( script::VM* vm, const char* funcName );
	int		script_crosshairPos( script::VM* vm, const char* funcName );

	OverlayDisplay( const OverlayDisplay& );
	OverlayDisplay& operator=( const OverlayDisplay& );
};


#endif // _OVERLAYDISPLAY_H
