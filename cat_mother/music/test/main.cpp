#include <win/FrameWindow.h>
#include <lang/System.h>
#include <lang/Exception.h>
#include <music/MusicManager.h>
#include <stdio.h>
#include <stdlib.h>
#include <config_msvc.h>

//-----------------------------------------------------------------------------

using namespace win;
using namespace lang;
using namespace music;

//-----------------------------------------------------------------------------

class MainWindow :
	public FrameWindow
{
public:
	MainWindow( MusicManager* musicMgr ) :
		m_musicMgr( musicMgr )
	{
	}

	void handleKeyDown( int key )
	{
		switch ( key )
		{
		case '1':
			m_musicMgr->fadeIn( "/tmp/sunburn.mp3", 3.f );
			break;
		case '2':
			m_musicMgr->fadeIn( "/tmp/dontbelight.mp3", 3.f );
			break;
		}
	}

	void deinit()
	{
		m_musicMgr = 0;
	}

private:
	P(MusicManager)		m_musicMgr;
};

//-----------------------------------------------------------------------------

int WINAPI WinMain( HINSTANCE inst, HINSTANCE, LPSTR /*cmdLine*/, int /*cmdShow*/ )
{
	char			wndTitle[256]		= "mp3play";
	P(MainWindow)	wnd					= 0;

	try
	{
		P(MusicManager)	musicMgr = new MusicManager;

		// create main window
		wnd = new MainWindow( musicMgr );
		wnd->create( "mp3playclass", wndTitle, 640, 480, false, inst );

		// primary main loop
		long prevTime = System::currentTimeMillis();
		while ( Window::flushWindowMessages() )
		{
			// time elapsed from last update
			long curTime = System::currentTimeMillis();
			long deltaTime = curTime - prevTime;
			prevTime = curTime;
			float dt = (float)deltaTime * 1e-3f;

			musicMgr->pause( !wnd->active() );
			if ( wnd->active() )
			{
				musicMgr->update( dt );
			}
		}
	}
	catch ( Throwable& e )
	{
		wnd->deinit();

		// minimize window so that it doesn't overlap message box
		HWND hwnd = (wnd ? wnd->handle() : 0);
		ShowCursor( TRUE );
		if ( hwnd )
			MoveWindow( hwnd, 0, 0, 4, 4, TRUE );

		// show error message
		char msgText[256];
		char msgTitle[256];
		sprintf( msgTitle, "%s - Error", wndTitle );
		e.getMessage().format().getBytes( msgText, sizeof(msgText), "ASCII-7" );
		MessageBox( hwnd, msgText, msgTitle, MB_OK|MB_ICONERROR );
	}
	return 0;
}
