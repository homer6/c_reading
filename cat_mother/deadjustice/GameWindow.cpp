#include "GameWindow.h"
#include "Game.h"
#include "GameCamera.h"
#include "resource.h"
#include "printMemoryState.h"
#include "GameLevel.h"
#include <util/ExProperties.h>
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <id/InputDriver.h>
#include <id/InputDevice.h>
#include <ps/ParticleSystemManager.h>
#include <sg/Font.h>
#include <sg/Texture.h>
#include <sg/Context.h>
#include <sgu/ContextUtil.h>
#include <sgu/SceneManager.h>
#include <snd/SoundManager.h>
#include <dev/Profile.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <util/ExProperties.h>
#include <music/MusicManager.h>
#include <script/VM.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace ps;
using namespace sg;
using namespace id;
using namespace sgu;
using namespace snd;
using namespace lang;
using namespace music;
using namespace script;

//-----------------------------------------------------------------------------

GameWindow::GameWindow( InputStreamArchive* arch, util::ExProperties* cfg ) :
	m_arch( arch ),
	m_cfg( cfg ),
	m_vm( new VM ),
	m_sceneMgr( new SceneManager(arch) ),
	m_particleMgr( new ParticleSystemManager(arch) ),
	m_soundMgr( 0 ),
	m_musicMgr( 0 ),
	m_inputDrvDll( "id_dx8" ),
	m_context( 0 ),
	m_dbgFont( 0 ),
	m_grabScreen( false ),
	m_game( 0 ),
	m_quit( false )
{
}

GameWindow::~GameWindow()
{
}

void GameWindow::init( const char* wndTitle, HINSTANCE inst ) 
{
	// create the window
	int		width			= m_cfg->getInteger("Display.Width");
	int		height			= m_cfg->getInteger("Display.Height");
	int		bitsPerPixel	= m_cfg->getInteger("Display.BitsPerPixel");
	int		refreshRate		= m_cfg->getInteger("Display.RefreshRate");

	create( "wc.deadjustice.catmother", wndTitle, width, height, bitsPerPixel>0, inst, IDI_ICON1 );

	// set default settings which should not be saved
	m_cfg->setBoolean( "Game.Pause", false );
	m_cfg->setBoolean( "Game.FlyCamera", false );
	m_cfg->setBoolean( "Game.SlowMotion", false );

	// init sound manager
	m_soundMgr = new SoundManager( m_arch );
	if ( m_cfg->getBoolean("Sound.Enabled") == false )
		m_soundMgr->setVolume( -100 );

	// init music manager
	/*m_musicMgr = new MusicManager;
	float musicvolume = MusicManager::interpVolume( m_cfg->getFloat( "Music.Volume" ) / 100.f, -100.f, m_cfg->getFloat( "Music.MaxVolume" ) );
	m_musicMgr->setVolume( musicvolume );
	// start playing loading music
	if ( m_cfg->getBoolean("Music.Enabled") )
		m_musicMgr->play( String("data/music/") + m_cfg->get("Music.Loading") );*/

	// initialize input driver
	initInputDriver();

	// init graphics context
	Context::RasterizerType			rasterizer		= Context::RASTERIZER_HW;
	Context::VertexProcessingType	vertexProcessor	= Context::VERTEXP_HW;
	int								surfaceFlags	= Context::SURFACE_TARGET | Context::SURFACE_DEPTH;
	Context::TextureFilterType		mipMapFilter	= ContextUtil::toTextureFilterType( (*m_cfg)["Display.MipMapFilter"] );
	float							mipMapLODBias	= m_cfg->getFloat("Display.MipMapLODBias");

	if ( m_cfg->getBoolean("Display.SoftwareVertexProcessing") )
		vertexProcessor	= Context::VERTEXP_SW;

	if ( m_cfg->getBoolean("Display.Stencil") )
		surfaceFlags |= Context::SURFACE_STENCIL;

	bool tc = m_cfg->getBoolean("Display.TextureCompression");
	Context::TextureCompressionType	textureCompression = Context::TC_NONE;
	if ( tc ) 
		textureCompression = Context::TC_COMPRESSED;

	if ( !m_cfg->containsKey("Display.RenderDriver") )
		throw Exception( Format("Display.RenderDriver must be set in properties.") );
	m_context = new Context( m_cfg->get("Display.RenderDriver") );
	m_context->open( width, height, bitsPerPixel, refreshRate, surfaceFlags, rasterizer, vertexProcessor, textureCompression );
	m_context->setMipMapFilter( mipMapFilter );
	m_context->setMipMapLODBias( mipMapLODBias );

	// texture settings
	Texture::setDownScaling( m_cfg->getBoolean("Display.HalfTextureResolution") );
	Texture::setDefaultBitDepth( m_cfg->getInteger("Display.TextureBitDepth") );

	// load debug font
	String debugFont = m_cfg->get("Debug.Font");
	P(InputStream) fontImage = m_arch->getInputStream( debugFont + ".tga" );
	P(InputStream) fontCharSet = m_arch->getInputStream( debugFont + ".txt" );
	m_dbgFont = new Font( fontImage, fontCharSet );
	fontCharSet->close();
	fontImage->close();
	
	// init game
	recompile();
}

void GameWindow::deinit()
{
	if ( m_game )
	{
		m_game->destroy();
		m_game = 0;
	}

	if ( m_context )
	{
		m_context->destroy();
		m_context = 0;
	}

	deInitInputDriver();
	
	if ( m_soundMgr )
	{
		m_soundMgr->destroy();
		m_soundMgr = 0;
	}

	Texture::flushTextures();
}

void GameWindow::recompile()
{
	// stop any running cut scene
	if ( m_game )
	{
		if ( m_game->level()->isActiveCutScene() )
			m_game->level()->skipCutScene();
		m_game = 0;
	}

	// DEBUG: print globals
	m_vm->getGlobals();
	m_vm->pushNil();
	Debug::println( "" );
	Debug::println( "Globals before recompile:" );
	Debug::println( "-------------------------" );
	while ( m_vm->next(-2) )
	{
		String name = m_vm->toString(-2);
		Debug::println( "  {0}", name );
		m_vm->pop();
	}
	m_vm->pop();

	// reset globals (note: _ERRORMESSAGE, trace, math and string must be set afterwards)
	//{Table glob( m_vm );
	//m_vm->pushTable( glob );
	//m_vm->setGlobals();}

	m_soundMgr->clear();
	m_particleMgr->clear();
	m_game = new Game( m_vm, m_arch, m_context, m_dbgFont, m_cfg, m_soundMgr, m_particleMgr, m_musicMgr, m_sceneMgr, m_inputDrv );
	m_context->flushDeviceObjects();
}

void GameWindow::focusLost()
{
	m_game->focusLost();
}

bool GameWindow::update( float dt )
{
	if ( !m_context->restore() )
		return true;

	m_game->update( dt );
	return !m_quit;
}

void GameWindow::render()
{
	if ( m_context->ready() )
	{
		// begin render
		m_context->clear();
		m_context->beginScene();

		// render menu/game
		m_game->render();

		// finish rendering
		m_context->endScene();

		// screenshot?
		if ( m_grabScreen )
		{
			ContextUtil::grabScreen( m_cfg->get("Display.ScreenshotExtension") );
			m_grabScreen = false;
		}

		// flip back buffer
		m_context->present();
	}
}

LRESULT GameWindow::handleMessage( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ) 
{
	return FrameWindow::handleMessage( hwnd, msg, wp, lp );
}

void GameWindow::handleKeyDown( int key ) 
{
	if ( m_game )
	{
		if ( m_game->level() && !m_game->level()->isActiveCutScene() )
		{
			switch ( key )
			{
			case VK_F8:			m_game->setArcBallCameraEnabled( !m_game->arcBallCameraEnabled() ); break;
			case 'I':			m_game->setInvulnerable(); break;
			}
		}

		switch ( key )
		{
		case 'T':			m_cfg->setBoolean( "Game.SlowMotion", !m_cfg->getBoolean("Game.SlowMotion") ); break;
		case VK_F11:		recompile(); m_game->skipNoticeScreen(); break;
		case VK_ESCAPE:		m_quit=true; break;
		case VK_SPACE:		if (m_game->level()) m_game->level()->skipCutScene(); break;
		}

		// developer keys
		if ( m_cfg->getBoolean("Debug.Keys") )
		{
			switch ( key )
			{
			case VK_F1:			m_game->selectActiveCamera(0); break;
			case VK_F2:			m_game->selectActiveCamera(1); break;
			case VK_F3:			m_game->selectActiveCamera(2); break;
			case VK_F4:			m_game->selectActiveCamera(3); break;
			case VK_F5:			m_game->selectActiveCamera(4); break;
			case VK_F6:			m_game->selectActiveCamera(5); break;
			case VK_F7:			if ( GetKeyState(VK_SHIFT) < 0 )
									m_cfg->setBoolean( "Debug.ManualFrameAdvance", !m_cfg->getBoolean("Debug.ManualFrameAdvance") );
								else
									if ( m_game->activeCamera() ) m_game->activeCamera()->printPrimitives(); 
								break;
			case VK_F8:			m_game->setArcBallCameraEnabled( m_game->arcBallCameraEnabled() ); break;
			case VK_F12:		m_cfg->setBoolean( "Debug.Info", !m_cfg->getBoolean("Debug.Info") ); break;
			case 'F':			m_cfg->setBoolean( "Game.FlyCamera", !m_cfg->getBoolean("Game.FlyCamera") ); if ( !m_cfg->getBoolean("Game.FlyCamera") )  m_game->resetInputState(); break;
			case VK_F9:			m_grabScreen=true; break;
			case VK_PAUSE:		m_cfg->setBoolean( "Game.Pause", !m_cfg->getBoolean("Game.Pause") ); break;
			}
		}
	}

	// debug keys
#ifdef _DEBUG
	switch ( key )
	{
		case VK_F8:				printMemoryState(); break;
	}
#endif
}


MusicManager* GameWindow::musicManager() const
{
	assert( m_musicMgr );
	return m_musicMgr;
}

void GameWindow::initInputDriver()
{
	/* Load dll */
	const char* drvname = "id_dx8";
	createInputDriverFunc createInputDriver = (createInputDriverFunc)m_inputDrvDll.getProcAddress( "createInputDriver" );
	if ( !createInputDriver )
		throw Exception( Format("Corrupted input library driver: {0}", drvname) );
		
	/* Check version */
	getInputDriverVersionFunc getInputDriverVersion = (getInputDriverVersionFunc)m_inputDrvDll.getProcAddress( "getInputDriverVersion" );
	if ( !getInputDriverVersion )
		throw Exception( Format("Old input library driver: {0}", drvname) );
	int ver = getInputDriverVersion();
	if ( ver != InputDriver::VERSION )
		throw Exception( Format("Wrong version ({1,#}) of the input library driver: {0}", drvname, ver) );

	/* Create input driver */
	m_inputDrv = createInputDriver();
	m_inputDrv->create();
}

void GameWindow::deInitInputDriver()
{
	if ( m_inputDrv )
	{
		m_inputDrv->destroy();
		m_inputDrv = 0;
	}
}
