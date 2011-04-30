#include "GameWindow.h"
#include <util/ExProperties.h>
#include "build.h"
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <io/DirectoryInputStreamArchive.h>
#include <sg/Context.h>
#include <mem/Group.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <lang/Exception.h>
#include <music/MusicManager.h>
#include <crypt/DecryptDirectoryInputStreamArchive.h>
#include <stdio.h>
#include "config.h"

//-----------------------------------------------------------------------------

static char			PROP_FILE_NAME[]	= "deadjustice.prop";

static const float	FIRST_UPDATE_DT		= 1.f / 10.f;

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace win;
using namespace lang;
using namespace crypt;

//-----------------------------------------------------------------------------

static void run( HINSTANCE inst )
{
	char							wndTitle[256]	= "";
	P(GameWindow)					wnd				= 0;

	try
	{
		// game window title
		#ifdef _DEBUG
		sprintf( wndTitle, "Dead Justice DEBUG Build %i", BUILD_NUMBER );
		#else
		sprintf( wndTitle, "Dead Justice Build %i", BUILD_NUMBER );
		#endif

		// load properties
		P(util::ExProperties) cfg = new util::ExProperties;
		FileInputStream propIns( PROP_FILE_NAME );
		cfg->load( &propIns );
		propIns.close();

		// open file archive
		P(InputStreamArchive) arch = 0;
		const char* paths[] = 
		{ 
			"cameras",
			"characters", 
			"particles", 
			"levels",
			"misc", 
			"fonts", 
			"sounds",
			"music",
			"onscreen",
			"projectiles",
			"weapons",
			0	
		};
		if ( cfg->getBoolean("Decrypt.Enabled") )
		{
			String dataPath = "data";
			P(DecryptDirectoryInputStreamArchive) dirArchive = new DecryptDirectoryInputStreamArchive;
			dirArchive->addPath( dataPath );
			for ( int i = 0 ; paths[i] ; ++i )
				dirArchive->addPath( File(dataPath,paths[i]).getPath() );
			arch = dirArchive.ptr();
		}
		else
		{
			String dataPath = "data";
			P(DirectoryInputStreamArchive) dirArchive = new DirectoryInputStreamArchive;
			dirArchive->addPath( dataPath );
			for ( int i = 0 ; paths[i] ; ++i )
				dirArchive->addPath( File(dataPath,paths[i]).getPath() );
			arch = dirArchive.ptr();
		}

		if ( cfg->getBoolean("Game.ControllerWarning") )
			MessageBox( 0, "This demo has been designed to work with\n- mouse and keyboard \n- Playstation 2 controller with EMS2 adapter\n  (controller name: 4 axis 16 button joystick)\n\nPlease remove any other attached joysticks, thank you.", "Note About Dead Justice Demo Controller Support", MB_OK );

		// create main window
		wnd = new GameWindow( arch, cfg );
		wnd->init( wndTitle, inst );

		// primary main loop
		bool firstUpdate = true;
		bool wasActive = true;
		long prevTime = System::currentTimeMillis();
		while ( Window::flushWindowMessages() )
		{
			// time elapsed from last update
			long curTime = System::currentTimeMillis();
			long deltaTime = curTime - prevTime;
			prevTime = curTime;
			float dt = (float)deltaTime * 1e-3f;
			if ( dt > 1.f/5.f )
				dt = 1.f/5.f;

			// update game window
			if ( wnd->active() )
			{
				if ( !wasActive )
					wnd->focusLost();

				// constant update interval on first update
				if ( firstUpdate )
				{
					dt = FIRST_UPDATE_DT;
					firstUpdate = false;
				}

				if ( !wnd->update(dt) )
					break;

				wnd->render();
			}
			else if ( wasActive )
			{
			}

			// update background music
			/*music::MusicManager* musicMgr = wnd->musicManager();
			if ( musicMgr )
			{
				musicMgr->pause( !wnd->active() || !cfg->getBoolean("Music.Enabled") );
				musicMgr->update( dt );
			}*/

			wasActive = wnd->active();
		}
		FileOutputStream out( PROP_FILE_NAME );
		cfg->store( &out, "Dead Justice game options." );
		out.close();	

		// deinit rendering context, threads, etc.
		wnd->deinit();
	}
	catch ( Throwable& e )
	{
		// deinit rendering context, threads, etc.
		if ( wnd )
			wnd->deinit();

		// minimize window so that it doesn't overlap message box
		HWND hwnd = (wnd ? wnd->handle() : 0);
		ShowCursor( TRUE );
		if ( hwnd )
			MoveWindow( hwnd, 0, 0, 4, 4, TRUE );

		// show error message
		char msgText[2560];
		char msgTitle[256];
		sprintf( msgTitle, "%s - Error", wndTitle );
		e.getMessage().format().getBytes( msgText, sizeof(msgText), "ASCII-7" );
		MessageBox( hwnd, msgText, msgTitle, MB_OK|MB_ICONERROR );
	}
}

//-----------------------------------------------------------------------------

int WINAPI WinMain( HINSTANCE inst, HINSTANCE, LPSTR /*cmdLine*/, int /*cmdShow*/ )
{
	run( inst );
	mem_printAllocatedBlocks();
	return 0;
}
