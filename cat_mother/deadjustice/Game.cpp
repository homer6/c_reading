#include "Game.h"
#include "GameCamera.h"
#include "GameCell.h"
#include "CollisionInfo.h"
#include "GameCutScene.h"
#include "GamePointObject.h"
#include "GameRenderPass.h"
#include "GameCharacter.h"
#include "GameController.h"
#include "GameNoiseManager.h"
#include "GameSurface.h"
#include "UserControl.h"
#include "GameBSPTree.h"
#include "GameLevel.h"
#include "GamePortal.h"
#include "GameProjectile.h"
#include "GameWeapon.h"
#include "ComputerControl.h"
#include "Timer.h"
#include "OverlayDisplay.h"
#include "ProjectileManager.h"
#include <gd/GraphicsDevice.h>
#include <id/InputDriver.h>
#include <io/InputStream.h>
#include <io/InputStreamArchive.h>
#include <sg/Font.h>
#include <sg/Mesh.h>
#include <sg/SpotLight.h>
#include <sg/PointLight.h>
#include <sg/DirectLight.h>
#include <sg/Scene.h>
#include <sg/Camera.h>
#include <sg/Sprite.h>
#include <sg/Context.h>
#include <sg/Texture.h>
#include <sg/LineList.h>
#include <sg/VertexLock.h>
#include <sg/VertexAndIndexLock.h>
#include <sg/ShadowShader.h>
#include <sgu/NodeUtil.h>
#include <sgu/ShadowUtil.h>
#include <bsp/BSPNode.h>
#include <bsp/BSPCollisionUtil.h>
#include <fsm/StateMachine.h>
#include <mem/Group.h>
#include <snd/Sound.h>
#include <snd/SoundManager.h>
#include <sgu/LineListUtil.h>
#include <sgu/ContextUtil.h>
#include <sgu/CameraUtil.h>
#include <sgu/SceneManager.h>
#include <ps/ParticleSystemManager.h>
#include <dev/Profile.h>
#include <pix/Color.h>
#include <win/Window.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <lang/Exception.h>
#include <util/ExProperties.h>
#include <util/Random.h>
#include <math/Vector3.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include <music/MusicManager.h>
#include <assert.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

#define TRACE() /*Debug::println( "{0}({1})", __FILE__, __LINE__ )*/

//-----------------------------------------------------------------------------

using namespace id;
using namespace io;
using namespace sg;
using namespace ps;
using namespace bsp;
using namespace sgu;
using namespace snd;
using namespace dev;
using namespace pix;
using namespace sgu;
using namespace win;
using namespace lang;
using namespace util;
using namespace math;
using namespace script;
using namespace music;

//-----------------------------------------------------------------------------

class Game::Impl :
	public lang::Object
{
public:
	Impl( VM* vm, InputStreamArchive* arch, Context* context, Font* dbgFont, ExProperties* cfg,
		SoundManager* soundMgr, ParticleSystemManager* particleMgr, MusicManager* musicMgr, 
		SceneManager* sceneMgr, InputDriver* inputDriver ) :
		m_soundMgr( soundMgr ),
		m_particleMgr( particleMgr ),
		m_musicMgr( musicMgr ),
		m_sceneMgr( sceneMgr ),
		m_noiseMgr( new GameNoiseManager(vm, arch) ),
		m_projectileMgr( new ProjectileManager( vm, arch, soundMgr, particleMgr, sceneMgr, m_noiseMgr ) ),
		m_gameCtrl( new GameController( vm, arch, inputDriver, cfg ) ),
		m_arch( arch ),
		m_context( context ),
		m_dbgFont( dbgFont ),
		m_cfg( cfg ),
		m_vm( vm ),
		m_endScreen( 0 ),
		m_noticeScreen( 0 ),
		m_noticeScreenEnabled( false ),
		m_noticeTime( 0.f ),
		m_timer( 20e-3f ),
		m_level(0),
		m_cameras( Allocator<P(GameCamera)>(__FILE__) ),
		m_onscreen( 0 ),
		m_hero( 0 ),
		m_scene( new Scene ),
		m_soundListener( new Node ),
		m_dbgLineMesh( new Mesh ),
		m_dbgLines3D( new LineList(5000,LineList::LINES_3D) ),
		m_dbgLines2D( new LineList(1000,LineList::LINES_2D) ),
		m_initStart( System::currentTimeMillis() ),
		m_fps( 0.f ),
		m_maxNoise( 0 ),
		m_timeProfiled( 0.f ),
		m_arcBallCameraEnabled( false ),
		m_shadowFiller( new Mesh ),
		m_shadowShader( new ShadowShader ),
		m_inputDriver( inputDriver ),
		m_wasActiveCutScene( false ),
		m_profiles( Allocator<String>(__FILE__) )
	{
		m_cfg->setBoolean( "Game.Pause", false );
		m_cfg->setBoolean( "Game.FlyCamera", false );
		m_cfg->setBoolean( "Debug.ManualFrameAdvance", false );
		dev::Profile::setEnabled( m_cfg->getBoolean("Debug.Profiling") );

		// set controller global
		m_vm->pushTable( m_gameCtrl );
		m_vm->setGlobal( "controller" );

		// render loading splash
		m_context->clear();
		m_context->beginScene();
		renderScreen( m_cfg->get("Game.LoadingScreen"), 0 );
		m_context->endScene();
		m_context->present();	

		// Set sound manager scene
		m_soundMgr->setScene( m_scene );		

		// Set particle system manager scene 
		m_particleMgr->setScene( m_scene );	

		loadGame();
	}

	~Impl()
	{
		m_noiseMgr->removeNoises();
	}

	void loadGame()
	{
		m_level = new GameLevel( m_vm, m_arch, m_soundMgr, m_particleMgr, m_sceneMgr, m_musicMgr, m_projectileMgr, m_noiseMgr, m_cfg->getInteger("Debug.BSPBuildPolySkip") );
		m_level->compile( m_cfg->get("Game.Level") );
		
		m_hero = m_level->mainCharacter();
		if ( !m_hero )
			throw Exception( Format("Main character not set in level {0}", m_level->name()) );

		// HACK: re-parent "Dummy_moon" to CELL background
		P(GameCell) cell = m_level->getCell("CELL");
		P(Node) cellNode = cell->getRenderObject(0);
		if ( !cellNode )
			throw Exception( Format("Moon hack failed: Cell CELL not found") );
		P(Node) moon = NodeUtil::findNodeByName(cellNode,"Dummy_moon");
		P(Node) bg = cell->background();
		if ( !bg )
			throw Exception( Format("Moon hack failed: CELL has no background") );
		if ( !moon )
			throw Exception( Format("Moon hack failed: CELL has no Dummy_moon object") );
		Matrix4x4 moontm = moon->worldTransform();
		moon->linkTo( bg );
		moon->setTransform( bg->worldTransform().inverse() * moontm );

		initGameObjects();
		load();
		m_gameCtrl->flushDevices();
	}

	P(Sprite) renderScreen( const String& fname, P(Sprite) sprite )
	{
		if ( !sprite )
		{
			P(InputStream) in = m_arch->getInputStream( fname );
			sprite = new Sprite( new Texture( in ), Sprite::createMaterial(), Sprite::createTriangleList() );
			in->close();
		}

		float splashX = (float)(m_context->width()-sprite->width())/2;
		float splashY = (float)(m_context->height()-sprite->height())/2;
		sprite->setPosition( Vector2(splashX, splashY) );
		sprite->draw();
		return sprite;
	}

	GameLevel* level() const
	{
		return m_level;
	}

	/** Loads game objects to rendering device. */
	void load()
	{
		if ( m_level )
		{
			for ( int i = 0 ; i < m_level->cells() ; ++i )
			{
				GameCell* cell = m_level->getCell(i);
				Node* root = cell->getRenderObject(0);
				for ( Node* node = root ; node ; node = node->nextInHierarchy() )
				{
					Mesh* mesh = dynamic_cast<Mesh*>( node );
					if ( mesh )
					{
						for ( int k = 0 ; k < mesh->primitives() ; ++k )
						{
							Primitive* prim = mesh->getPrimitive(k);
							prim->load();
						}
					}
				}
			}
		}
	}

	void update( float dt )
	{
		m_fps = (dt >= Float::MIN_VALUE ? 1.f / dt : 0.f);

		// notice still active?
		m_noticeTime += dt;
		m_noticeScreenEnabled = ( m_noticeTime < m_cfg->getFloat("Game.NoticeTime") );
		if ( m_noticeScreenEnabled )
			return;
		else
			m_noticeScreen = 0;

		// advance update manually
		if ( m_cfg->getBoolean("Debug.ManualFrameAdvance") )
		{
			if ( GetKeyState('O') < 0 )
				dt = 1.f/100.f;
			else
				dt = 0.f;
		}

		//m_gameCtrl->update( dt );
		updateGame( dt );
		m_timeProfiled += dt;
	}

	void updateGame( float dt )
	{
		// level ended?
		if ( m_level->ended() )
			return;

		// check active camera
		if ( !m_activeCamera )
			throw Exception( Format("Active camera not found.") );
		if ( !m_activeCamera->inCell() )
			m_activeCamera->setPosition( m_hero->cell(), m_hero->position() );

		// arcball camera input
		if ( m_arcBallCameraEnabled )
		{
			m_gameCtrl->update( dt );
			m_gameCtrl->controlArcBallCamera( m_hero, m_activeCamera, dt, m_cfg->getFloat("ArcBallCamera.TargetHeight"), Math::toRadians(m_cfg->getFloat("ArcBallCamera.RotationSpeed")), m_cfg->getFloat("ArcBallCamera.DollySpeed"), m_cfg->getFloat("ArcBallCamera.DollyMin"), m_cfg->getFloat("ArcBallCamera.DollyMax"), Math::toRadians(m_cfg->getFloat("ArcBallCamera.TiltMax")) );
		}

		// time scale
		float timeScale = m_cfg->getFloat("Game.TimeScale");
		if ( m_cfg->getBoolean("Game.SlowMotion") )
			timeScale = m_cfg->getFloat("Debug.SlowMotion");
		if ( m_cfg->getBoolean("Game.Pause") )
			timeScale = 0.f;
		if ( m_arcBallCameraEnabled )
			timeScale = 0.f;
		float dt0 = dt;
		dt *= timeScale;
	
		// game update
		float totalDt = 0.f;
		int updatesInThisFrame = 0;
		for ( m_timer.beginUpdate(dt) ; m_timer.update() ; ++updatesInThisFrame )
		{
			float dt = m_timer.dt();
			totalDt += dt;

			if ( dt > Float::MIN_VALUE )
			{
				// controller input
				if ( !m_cfg->getBoolean("Game.FlyCamera") && !m_arcBallCameraEnabled )
				{
					m_gameCtrl->update( dt );
					m_activeCamera->setTarget( m_hero );
					if ( m_level->isActiveCutScene() )
					{
						m_hero->setControlSource( GameCharacter::CONTROL_COMPUTER );
						m_gameCtrl->controlCutScene( m_activeCamera, dt );
					}
					else
					{
						m_hero->setControlSource( GameCharacter::CONTROL_USER );
						m_gameCtrl->controlCharacter( m_hero, m_activeCamera, dt );
					}
				}

				// update level & level objects
				{dev::Profile pr( "update.level" );
				m_level->update( dt );}
				if ( m_wasActiveCutScene && !m_level->isActiveCutScene() )
					focusLost();
				m_wasActiveCutScene = m_level->isActiveCutScene();

				for ( int i = 0; i < m_level->characters(); ++i )
					m_level->getCharacter(i)->update( dt, updatesInThisFrame==0 );

				{dev::Profile pr( "update.weapons" );
				for ( int i = 0; i < m_level->weapons(); ++i )
					m_level->getWeapon(i)->update( dt );}

				{dev::Profile pr( "update.triggers" );
				for ( int i = 0; i < m_level->triggers(); ++i )
					m_level->getTrigger(i)->update( dt );}

				{dev::Profile pr( "update.dynamicObjects" );
				for ( int i = 0; i < m_level->dynamicObjects(); ++i )
					m_level->getDynamicObject(i)->update( dt );}

				{dev::Profile pr( "update.noises" );
				m_noiseMgr->update( totalDt );}
 
				// update camera
				{dev::Profile pr( "update.camera" );
				if ( m_activeCamera && !m_cfg->getBoolean("Game.FlyCamera") && !m_arcBallCameraEnabled )
					m_activeCamera->update( dt );}

				// update projectiles
				{dev::Profile pr( "update.projectiles" );
				m_projectileMgr->update( dt );}

				// store hero noise level
				m_maxNoise = 0;
				for ( int i = 0 ; i < m_noiseMgr->noises() ; ++i )
				{
					GameNoise* noise = m_noiseMgr->getNoise(i);
					if ( noise->noiseSource() == m_hero )
					{
						float level = noise->getLevelAt( m_hero->position() );
						if ( !m_maxNoise || level > m_maxNoise->noiseLevel() )
							m_maxNoise = noise;
					}
				}
			}
		}

		if ( totalDt > Float::MIN_VALUE )
		{
			//Profile pr("Onscreen & Manager update"); 

			// update onscreen display.		
			m_onscreen->update( totalDt );
			
			// update sounds
			if ( m_cfg->getBoolean("Game.FlyCamera") || m_level->isActiveCutScene() || m_arcBallCameraEnabled )
			{
				m_soundListener->setTransform( m_activeCamera->getRenderCamera()->worldTransform() );
			}
			else
			{
				Matrix4x4 tm = m_hero->headWorldTransform();
				tm.setRotation( m_hero->rotation() );
				m_soundListener->setTransform( tm );
			}
			m_soundMgr->update( m_soundListener, totalDt, timeScale );
			
			// update particles
			{dev::Profile pr( "update.particles" );
			m_particleMgr->update( totalDt );}
		}

		// update flares
		for ( int i = 0 ; i < m_level->flareSets() ; ++i )
			m_level->getFlareSet(i)->update( dt0 );

		// update fly camera
		if ( m_activeCamera && m_cfg->getBoolean("Game.FlyCamera") && !m_arcBallCameraEnabled )
			flyCamera( m_activeCamera, dt0 );
	}

	void render()
	{
		// still loading?
		if (!m_level)
			return;

		// render end screen
		if ( m_level->ended() )
		{
			m_endScreen = renderScreen( m_cfg->get("Game.EndScreen"), m_endScreen );
			return;
		}

		// render notice screen
		if ( m_noticeScreenEnabled )
		{
			// render one frame after notice time is ended so we don't get one non-updated frame of the game
			m_noticeScreen = renderScreen( m_cfg->get("Game.NoticeScreen"), m_noticeScreen );
			return;
		}

		// wait for first update
		if ( m_timer.time() < 0.10f )
			return;

		// ensure active camera
		if ( !m_activeCamera )
			throw Exception( Format("Active camera not found.") );

		// prepare shadow filler
		m_shadowFiller->linkTo( m_scene );

		// render cells and objects
		m_activeCamera->render( m_context, m_scene );

		// unlink shadow filler
		m_shadowFiller->unlink();

		// add debug lines
		renderDebugLines( m_scene );

		// render on-screen display
		if ( !m_cfg->getBoolean( "Game.FlyCamera" ) )
			m_onscreen->render();
		
		// render debug and profiling information
		if ( m_cfg->getBoolean("Debug.Info") ) 
			renderDebugInfo();
		else
			renderGameInfo();

		// clear debug line lists
		m_dbgLines2D->removeLines();
		m_dbgLines3D->removeLines();
	}

	void renderGameInfo()
	{
		float x = 4.f;
		float y = 0.f + m_dbgFont->height() * m_cfg->getFloat( "Debug.FirstTextLine" );

		static bool showFPS = m_cfg->getBoolean("Debug.DisplayFPSAlways");
		if ( showFPS )
			m_dbgFont->drawText( x, y, Format("fps = {0,#}", m_fps).format(), 0, &y );

		if ( m_arcBallCameraEnabled )
			m_dbgFont->drawText( x, y, Format("Arc ball / pause camera. Press F8 again to return to game.").format(), 0, &y );

		if ( m_cfg->getBoolean("Game.SlowMotion") )
			m_dbgFont->drawText( x, y, Format("Slow motion. Press T again to return to normal time.").format(), 0, &y );

		if ( m_hero && m_hero->invulnerable() )
			m_dbgFont->drawText( x, y, Format("Invulnerability. Press I again to become vulnerable.").format(), 0, &y );
	}

	void refreshShadowFiller()
	{
		P(Primitive) shadowFiller = ShadowUtil::createShadowFiller( m_level->shadowColor(), (float)m_context->width(), (float)m_context->height() );
		shadowFiller->shader()->setPass( GameRenderPass::RENDERPASS_SHADOW_FILLER );
		m_shadowFiller->removePrimitives();
		m_shadowFiller->addPrimitive( shadowFiller );
		m_shadowFiller->setName( "ShadowFiller" );
	}

	void renderBSPTreeDebugLines( BSPNode* bspnode, const Matrix4x4& tm=Matrix4x4(1.f) )
	{
		for ( int i = 0 ; i < bspnode->polygons() ; ++i )
		{
			const BSPPolygon& poly = bspnode->getPolygon(i);

			Vector3 avg(0,0,0);
			for ( int k = 0 ; k < poly.vertices() ; ++k )
				avg += poly.getVertex(k) * (1.f/poly.vertices());
			avg = tm.transform(avg);

			if ( (avg-m_hero->position()).length() < 5.f ||
				(avg-m_activeCamera->position()).length() < 10.f )
			{
				int n = poly.vertices() - 1;
				for ( int k = 0 ; k < poly.vertices() ; n = k++ )
				{
					Vector3 v0 = tm.transform( poly.getVertex(n) );
					Vector3 v1 = tm.transform( poly.getVertex(k) );
					m_dbgLines3D->addLine( v0, v1, Color(0,0,128) );
					if ( m_dbgLines3D->lines() == m_dbgLines3D->maxLines() )
						return;
				}
			}
		}

		if ( bspnode->positive() )
			renderBSPTreeDebugLines( bspnode->positive() );
		if ( bspnode->negative() )
			renderBSPTreeDebugLines( bspnode->negative() );
	}

	void renderPortalDebugLines( GameCell* cell )
	{
		for ( int j = 0 ; j < cell->portals() ; ++j )
		{
			Vector3 corners[4];
			GamePortal* portal = cell->getPortal(j);
			portal->getCorners( corners );
			m_dbgLines3D->addLine(corners[0], corners[2], Color(0,255,0,128) );
			m_dbgLines3D->addLine(corners[1], corners[3], Color(0,255,0,128) );
			m_dbgLines3D->addLine(corners[0], corners[1], Color(255,255,0), Color(0,255,0) );
			m_dbgLines3D->addLine(corners[1], corners[2], Color(0,255,0) );
			m_dbgLines3D->addLine(corners[2], corners[3], Color(0,255,0) );
			m_dbgLines3D->addLine(corners[3], corners[0], Color(0,255,0) );
			Vector3 center = (corners[0]+corners[2])*.5f;
			m_dbgLines3D->addLine(center, center+portal->normal(), Color(0,0,255,128) );
		}
	}

	void renderAimVector( GameCharacter* character )
	{
		Vector3 plop(0,0,0);
		character->rotation().rotate( character->aimVector(), &plop );
		Color color(255,0,0,127);
		
		if ( m_gameCtrl->aimCollisionInfo().isCollision(CollisionInfo::COLLIDE_OBJECT) )
			color = Color(0,255,0,127);

		m_dbgLines3D->addLine( character->aimCenter(), character->aimCenter() + plop, color );
	}

	void renderProjectileTracers( const String& script )
	{
		Vector<P(GameProjectile)> projectiles(Allocator<P(GameProjectile)>(__FILE__,__LINE__));
		projectiles.setSize(m_projectileMgr->numProjectiles(script));
		
		m_projectileMgr->getProjectiles( script, projectiles );

		for ( int i = 0; i < projectiles.size(); ++i )
		{
			if ( projectiles[i] )
			{
				m_dbgLines3D->addLine( projectiles[i]->position(), projectiles[i]->position() - projectiles[i]->velocity() * 0.25f, 
									Color(255, 255, 255, 255 ), Color(255, 255, 255, 0 ) );
			}
		}
	}

	void drawDebugLineBox( const Matrix4x4& tm, const Vector3& boxMin, const Vector3& boxMax, Color color=Color(0,255,0) )
	{
		Vector3 c = tm.translation();
		Vector3 xn = tm.rotation().getColumn(0) * boxMin.x;
		Vector3 yn = tm.rotation().getColumn(1) * boxMin.y;
		Vector3 zn = tm.rotation().getColumn(2) * boxMin.z;
		Vector3 xp = tm.rotation().getColumn(0) * boxMax.x;
		Vector3 yp = tm.rotation().getColumn(1) * boxMax.y;
		Vector3 zp = tm.rotation().getColumn(2) * boxMax.z;

		Vector3 lines[] =
		{
			c+xn+yp+zn, c+xp+yp+zn,
			c+xp+yp+zn, c+xp+yn+zn,
			c+xp+yn+zn, c+xn+yn+zn,
			c+xn+yn+zn, c+xn+yp+zn,
			c+xn+yp+zp, c+xp+yp+zp,
			c+xp+yp+zp, c+xp+yn+zp,
			c+xp+yn+zp, c+xn+yn+zp,
			c+xn+yn+zp, c+xn+yp+zp,
			c+xn+yp+zn, c+xn+yp+zp, 
			c+xp+yp+zn, c+xp+yp+zp, 
			c+xp+yn+zn, c+xp+yn+zp, 
			c+xn+yn+zn, c+xn+yn+zp, 
		};

		const int LINES = sizeof(lines) / sizeof(lines[0]) / 2;
		for ( int i = 0 ; i < LINES ; ++i )
			m_dbgLines3D->addLine( lines[i*2], lines[i*2+1], color );
	}

	void renderBoneCollisionBoxes( GameCharacter* ch )
	{
		const BoneCollisionBox& box = ch->rootCollisionBox();
		drawDebugLineBox( box.bone()->worldTransform(), box.boxMin(), box.boxMax() );

		for ( int i = 0 ; i < ch->boneCollisionBoxes() ; ++i )
		{
			const BoneCollisionBox& box = ch->getBoneCollisionBox( i );
			drawDebugLineBox( box.bone()->worldTransform(), box.boxMin(), box.boxMax() );
		}
	}

	void renderDebugLineTriggers()
	{
		for ( int i = 0 ; i < m_level->triggers() ; ++i )
		{
			GameBoxTrigger* trigger = m_level->getTrigger(i);
			drawDebugLineBox( trigger->transform(), -trigger->dimensions(), trigger->dimensions() );
		}
	}

	void renderBoneAxisDebugLines( GameCharacter* character, const String& boneName )
	{
		Node* node = NodeUtil::findNodeByName( character->getRenderObject(0), boneName );
		if ( !node )
			throw Exception( Format("Bone {0} not found from {1}", boneName, character->name()) );
		Matrix4x4 tm = character->getBoneWorldTransform( node );
		//Matrix4x4 tm = character->headWorldTransform();
		m_dbgLines3D->addLine( tm.translation(), tm.translation()+tm.rotation().getColumn(0), Color(255,0,0) );
		m_dbgLines3D->addLine( tm.translation(), tm.translation()+tm.rotation().getColumn(1), Color(0,255,0) );
		m_dbgLines3D->addLine( tm.translation(), tm.translation()+tm.rotation().getColumn(2), Color(0,0,255) );
	}

	void renderDebugLinePaths()
	{
		for ( int i = 0 ; i < m_level->paths() ; ++i )
		{
			GamePath* path = m_level->getPath(i);
			for ( int k = 0 ; k < path->points() ; ++k )
			{
				Vector3 start = path->getPoint(k);
				
				Vector3 end(0,0,0);
				if ( k+1 < path->points() )
					end = path->getPoint(k+1);
				else
					end = path->getPoint(0);

				Vector3 offs = Vector3(0,0.2f,0);
				m_dbgLines3D->addLine( start+offs, end+offs, Color(255,255,255) );
			}
		}
	}

	/** Draws debug cone along Z-axis */
	void drawDebugLineCone( const Matrix4x4& tm, float length, float horzAngle, float vertAngle, Color col, GameCharacter* character=0 )
	{
		Vector3 o = tm.translation();
		float dx = Math::tan(horzAngle) * length;
		float dy = Math::tan(vertAngle) * length;

		Vector3 pts[] =
		{
			o,
			o + tm.rotate( Vector3(-dx,dy,length) ),
			o + tm.rotate( Vector3(dx,dy,length) ),
			o + tm.rotate( Vector3(dx,-dy,length) ),
			o + tm.rotate( Vector3(-dx,-dy,length) ),
		};
		const int POINTS = sizeof(pts) / sizeof(pts[0]);

		int lines[][2] =
		{
			0,1,
			0,2,
			0,3,
			0,4,
			1,2,
			2,3,
			3,4,
			4,1
		};
		int LINES = sizeof(lines) / sizeof(lines[0]);

		// ray trace collisions if requested
		if ( character )
		{
			GamePointObject* vis = character->visibilityCollisionChecker();
			for ( int i = 1 ; i < POINTS ; ++i )
			{
				CollisionInfo cinfo;
				vis->setPosition( character->cell(), o );
				vis->move( pts[i]-vis->position(), &cinfo );
				pts[i] = cinfo.position();
			}
		}

		for ( int i = 0 ; i < LINES ; ++i )
			m_dbgLines3D->addLine( pts[lines[i][0]], pts[lines[i][1]], col );
	}

	void renderDebugLineVisionCones()
	{
		for ( int i = 0 ; i < m_level->characters() ; ++i )
		{
			GameCharacter* character = m_level->getCharacter(i);
			if ( character->visible() && !character->isDead(character->primaryState()) && character != m_level->mainCharacter() )
			{
				ComputerControl* cc = character->computerControl();

				const float horzInnerCone		= Math::toRadians( cc->getNumber( "visionHorzInnerCone" ) ) * .5f;
				const float horzOuterCone		= Math::toRadians( cc->getNumber( "visionHorzOuterCone" ) ) * .5f;
				const float vertInnerCone		= Math::toRadians( cc->getNumber( "visionVertInnerCone" ) ) * .5f;
				const float vertOuterCone		= Math::toRadians( cc->getNumber( "visionVertOuterCone" ) ) * .5f;
				const float farAttenStart		= cc->getNumber( "visionFarAttenStart" );
				const float farAttenEnd			= cc->getNumber( "visionFarAttenEnd" );

				Matrix4x4 tm = character->headWorldTransform();

				drawDebugLineCone( tm, farAttenStart, horzInnerCone, vertInnerCone, Color(0,255,255), 0 );
				drawDebugLineCone( tm, farAttenEnd, horzOuterCone, vertOuterCone, Color(255,0,0), 0 );
			}
		}
	}

	void drawDebugLineAxis( const Matrix4x4& tm, float axisLen=1.f )
	{
		m_dbgLines3D->addLine( tm.translation(), tm.translation()+tm.rotation().getColumn(0)*axisLen, Color(255,0,0) );
		m_dbgLines3D->addLine( tm.translation(), tm.translation()+tm.rotation().getColumn(1)*axisLen, Color(0,255,0) );
		m_dbgLines3D->addLine( tm.translation(), tm.translation()+tm.rotation().getColumn(2)*axisLen, Color(0,0,255) );
	}

	void renderDebugLineLights()
	{
		GameCell* cell = m_activeCamera->cell();
		for ( Node* node = cell->getRenderObject(0) ; node ; node = node->nextInHierarchy() )
		{
			SpotLight* spot = dynamic_cast<SpotLight*>( node );
			if ( spot )
			{
				drawDebugLineCone( spot->worldTransform(), spot->range(), spot->innerCone(), spot->innerCone(), Color(255,255,0), 0 );
				drawDebugLineCone( spot->worldTransform(), spot->range(), spot->outerCone(), spot->outerCone(), Color(255,255,0,64), 0 );
				continue;
			}

			DirectLight* direct = dynamic_cast<DirectLight*>( node );
			if ( direct )
			{
				drawDebugLineAxis( direct->worldTransform() );
				float directLightLen = 10.f;
				drawDebugLineBox( direct->worldTransform(), -Vector3(0.5f,0.5f,0), Vector3(0.5f,0.5f,directLightLen), Color(255,255,0) );
				continue;
			}

			PointLight* point = dynamic_cast<PointLight*>( node );
			if ( point )
			{
				drawDebugLineAxis( point->worldTransform() );
				drawDebugLineBox( point->worldTransform(), -Vector3(1,1,1)*point->range(), Vector3(1,1,1)*point->range(), Color(255,255,0) );
				continue;
			}
		}
	}

	void renderDebugLines( Node* root )
	{
		// setup debug line drawing
		m_dbgLineMesh->setName( "Debug lines" );
		m_dbgLineMesh->removePrimitives();
		m_dbgLineMesh->addPrimitive( m_dbgLines2D );
		m_dbgLineMesh->addPrimitive( m_dbgLines3D );
		m_dbgLineMesh->setPosition( m_activeCamera->forward()*-0.10f );
		m_dbgLineMesh->unlink();

		bool render = m_cfg->getBoolean("Debug.Info");
		if ( render )
		{
			m_dbgLineMesh->linkTo( root );

			VertexLock<LineList> dbgLinesLock2D( m_dbgLines2D, LineList::LOCK_WRITE );
			VertexLock<LineList> dbgLinesLock3D( m_dbgLines3D, LineList::LOCK_WRITE );

			if ( m_cfg->getBoolean("Debug.DynamicObjectInfo") )
				renderDynamicObjectDebugLines();

			if ( m_cfg->getBoolean("Debug.DrawLights") )
				renderDebugLineLights();

			if ( m_cfg->getBoolean("Debug.AIDrawVisionCones") )
				renderDebugLineVisionCones();

			if ( m_cfg->getBoolean("Debug.BoneCollisionLines") )
			{
				for ( int i = 0 ; i < m_level->characters() ; ++i )
					renderBoneCollisionBoxes( m_level->getCharacter(i) );
			}

			if ( m_cfg->getBoolean("Debug.DrawTriggers") )
				renderDebugLineTriggers();

			if ( m_cfg->getBoolean("Debug.AIDrawPaths") )
				renderDebugLinePaths();

			renderAimVector( m_hero );
			renderProjectileTracers( "9mm.lua" );
			renderProjectileTracers( "5point6.lua" );
			renderProjectileTracers( "shotgunshot.lua" );
			renderCameraLimitDebugLines( m_activeCamera );
			renderPortalDebugLines( m_activeCamera->cell() );

			if ( m_cfg->getBoolean("Debug.BSPLines") )
				renderBSPTreeDebugLines( m_activeCamera->cell()->bspTree()->root() );
		}

		if ( render )
		{
			Camera* cam = m_activeCamera->getRenderCamera();
			cam->linkTo( root );
			cam->render();
			cam->unlink();
		}
	}

	void renderCameraLimitDebugLines( GameCamera* camera )
	{
		if ( !m_cfg->getBoolean( "Game.FlyCamera" ) && m_cfg->getBoolean("Debug.CameraInfo") )
		{		
			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + -camera->turnThreshold(0) ), 0.5f * m_context->height() * ( 1.f + -camera->tiltThreshold(0) ), 0),
								   Vector3( 0.5f * m_context->width() * ( 1.f + camera->turnThreshold(1) ), 0.5f * m_context->height() * ( 1.f + -camera->tiltThreshold(0) ), 0), Color(0,255,0,127) );
			
			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + camera->turnThreshold(1) ), 0.5f * m_context->height() * ( 1.f + -camera->tiltThreshold(0) ), 0), 
								   Vector3( 0.5f * m_context->width() * ( 1.f + camera->turnThreshold(1) ), 0.5f * m_context->height() * ( 1.f + camera->tiltThreshold(1) ), 0), Color(0,255,0,127) );
			
			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + camera->turnThreshold(1) ), 0.5f * m_context->height() * ( 1.f + camera->tiltThreshold(1) ), 0), 
								   Vector3( 0.5f * m_context->width() * ( 1.f + -camera->turnThreshold(0) ), 0.5f * m_context->height() * ( 1.f + camera->tiltThreshold(1) ), 0), Color(0,255,0,127) );
			
			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + -camera->turnThreshold(0) ), 0.5f * m_context->height() * ( 1.f + camera->tiltThreshold(1) ), 0), 
								   Vector3( 0.5f * m_context->width() * ( 1.f + -camera->turnThreshold(0) ), 0.5f * m_context->height() * ( 1.f + -camera->tiltThreshold(0) ), 0), Color(0,255,0,127) );

			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + -camera->horizontalLimit(0) ), 0.5f * m_context->height() * ( 1.f + -camera->verticalLimit(0) ), 0),
								   Vector3( 0.5f * m_context->width() * ( 1.f + camera->horizontalLimit(1) ), 0.5f * m_context->height() * ( 1.f + -camera->verticalLimit(0) ), 0), Color(255,75,75,127) );
			
			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + camera->horizontalLimit(1) ), 0.5f * m_context->height() * ( 1.f + -camera->verticalLimit(0) ), 0), 
								   Vector3( 0.5f * m_context->width() * ( 1.f + camera->horizontalLimit(1) ), 0.5f * m_context->height() * ( 1.f + camera->verticalLimit(1) ), 0), Color(255,75,75,127) );
			
			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + camera->horizontalLimit(1) ), 0.5f * m_context->height() * ( 1.f + camera->verticalLimit(1) ), 0), 
								   Vector3( 0.5f * m_context->width() * ( 1.f + -camera->horizontalLimit(0) ), 0.5f * m_context->height() * ( 1.f + camera->verticalLimit(1) ), 0), Color(255,75,75,127) );
			
			m_dbgLines2D->addLine( Vector3( 0.5f * m_context->width() * ( 1.f + -camera->horizontalLimit(0) ), 0.5f * m_context->height() * ( 1.f + camera->verticalLimit(1) ), 0), 
								   Vector3( 0.5f * m_context->width() * ( 1.f + -camera->horizontalLimit(0) ), 0.5f * m_context->height() * ( 1.f + -camera->verticalLimit(0) ), 0), Color(255,75,75,127) );
		}		
	}

	int updateCount( int& count, int newCount )
	{
		int diff = newCount - count;
		count = newCount;
		return diff;
	}

	void renderAIDebugInfo()
	{
		for ( int i = 0 ; i < m_level->characters() ; ++i )
		{
			GameCharacter* character = m_level->getCharacter(i);

			if ( character->visible() && character != m_level->mainCharacter() )
			{
				Vector3 pt = m_activeCamera->getScreenPoint( character->position() + Vector3(0,2,0) );
				pt.y -= m_dbgFont->height() * 6;

				if ( character->isDead( character->primaryState() ) )
				{
					m_dbgFont->drawText( pt.x, pt.y, Format("{0} is dead", character->name()).format(), 0, &pt.y );
				}
				else
				{
					m_dbgFont->drawText( pt.x, pt.y, Format("{0} ({1,#}%)", character->name(), character->health()).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("Morph = {0}", character->morphStateString() ).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("FSM = {0}", character->computerControl()->stateMachine()->state()).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("speed = {0} m/s", character->velocity().length()).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("state = {0}", character->stateString()).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("anim1: {0}", character->animationPrimaryStateString() ).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("anim2: {0}", character->animationSecondaryStateString() ).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("distance = {0,#.00}", (character->position() - m_hero->position()).length() ).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("canSee(hero) = {0,#.00} (visionLimit={1,#.00})", character->canSeeProbability(m_hero), character->computerControl()->getNumber("visionLimit") ).format(), 0, &pt.y );
					m_dbgFont->drawText( pt.x, pt.y, Format("canHear(hero) = {0,#.00} (hearingLimit={1,#.00})", (m_maxNoise ? m_maxNoise->getLevelAt(character->position()) : 0.f), character->computerControl()->getNumber("hearingLimit") ).format(), 0, &pt.y );
				}
			}
		}
	}

	void renderDynamicObjectDebugInfo()
	{
		for ( int i = 0 ; i < m_level->dynamicObjects() ; ++i )
		{
			GameDynamicObject* obj = m_level->getDynamicObject(i);

			if ( obj->visible() )
			{
				Vector3 pt = m_activeCamera->getScreenPoint( obj->position() );

				pt.y -= m_dbgFont->height() * 1;
				m_dbgFont->drawText( pt.x, pt.y, Format("{0}", obj->name()).format(), 0, &pt.y );
			}
		}
	}

	void renderDynamicObjectDebugLines()
	{
		for ( int i = 0 ; i < m_level->dynamicObjects() ; ++i )
		{
			GameDynamicObject* obj = m_level->getDynamicObject(i);

			if ( obj->visible() )
			{
				Vector3 pt = m_activeCamera->getScreenPoint( obj->position() );

				m_dbgLines2D->addLine( pt+Vector3(-5,-5,0), pt+Vector3(5,5,0), Color(255,255,255) );
				m_dbgLines2D->addLine( pt+Vector3(5,-5,0), pt+Vector3(-5,5,0), Color(255,255,255) );

				drawDebugLineAxis( obj->transform(), obj->bspTree()->boundSphere() );

				if ( m_cfg->getBoolean("Debug.RenderDynamicObjectBSPs") )
					renderBSPTreeDebugLines( obj->bspTree()->root(), obj->transform() );
			}
		}
	}

	void renderDebugInfo()
	{
		float x = 4.f;
		float y = 0.f + m_dbgFont->height() * m_cfg->getFloat( "Debug.FirstTextLine" );
		int lockedVertices = Context::device()->lockedVertices();
		int lockedIndices = Context::device()->lockedIndices();
		int renderedTriangles = Context::device()->renderedTriangles();
		int renderedPrimitives = Context::device()->renderedPrimitives();

		m_dbgFont->drawText( x, y, Format("fps = {0,#}", m_fps).format(), 0, &y );
		m_dbgFont->drawText( x, y, Format("time = {0,#.00} s", m_timer.time()).format(), 0, &y );

		if ( m_cfg->getBoolean("Debug.ObjectStats") )
		{
			int characters = 0;
			int dynamicObjects = 0;
			GameCell* cell = m_activeCamera->cell();
			for ( GameObjectListItem* it = cell->objectsInCell() ; it ; it = it->next() )
			{
				GameObject* obj = it->object();
				if ( dynamic_cast<GameCharacter*>(obj) )
					++characters;
				else if ( dynamic_cast<GameDynamicObject*>(obj) )
					++dynamicObjects;
			}
			m_dbgFont->drawText( x, y, Format("DOs in cell = {0}", dynamicObjects).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("characters in cell = {0}", characters).format(), 0, &y );
		}

		if ( m_activeCamera && m_level->isActiveCutScene() )
			m_dbgFont->drawText( x, y, Format("Cut scene time = {0} (frame {1,#})", m_level->activeCutScene()->time(), m_level->activeCutScene()->time()*30.f).format(), 0, &y );

		if ( m_cfg->getBoolean("Debug.AIInfo") && m_activeCamera )
			renderAIDebugInfo();

		if ( m_cfg->getBoolean("Debug.DynamicObjectInfo") && m_activeCamera )
			renderDynamicObjectDebugInfo();

		if ( m_cfg->getBoolean("Debug.CollisionInfo") )
		{
			m_dbgFont->drawText( x, y, Format("sphere-polygon checks = {0}", BSPCollisionUtil::statistics().movingSpherePolygonTests).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("line-polygon checks = {0}", BSPCollisionUtil::statistics().linePolygonTests).format(), 0, &y );
			BSPCollisionUtil::statistics().clear();
		}

		if ( m_cfg->getBoolean("Debug.RenderInfo") )
		{
			m_dbgFont->drawText( x, y, Format("pri / tri = {0} / {1}", renderedPrimitives, renderedTriangles).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("locked verts / indices = {0} / {1}", lockedVertices, lockedIndices).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("particles = {0}", m_particleMgr->particles()).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("texmem = {0} MB", Texture::totalTextureMemoryUsed()/(1024.f*1024.f)).format(), 0, &y );
		}

		if ( m_activeCamera && m_cfg->getBoolean("Debug.CameraInfo") )
		{
			Camera* cam = m_activeCamera->getRenderCamera();
			m_dbgFont->drawText( x, y, Format("Camera ({0}):", m_activeCamera->name()).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("  Position = ({0,#.#} {1,#.#} {2,#.#}) (cell {3}, objs {4}, col {5})", m_activeCamera->position().x, m_activeCamera->position().y, m_activeCamera->position().z, m_activeCamera->cell()->name(), m_activeCamera->cell()->objects(), m_activeCamera->cell()->collidableObjects()).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("  Forward = ({0,#.000} {1,#.000} {2,#.000})", m_activeCamera->forward().x, m_activeCamera->forward().y, m_activeCamera->forward().z).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("  FOV (horz) = {0,#}", Math::toDegrees(cam->horizontalFov()) ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format("  front/back = {0} / {1}", cam->front(), cam->back() ).format(), 0, &y );

			Vector3 fwd = m_activeCamera->forward();
			fwd.y = 0.f;
			if ( fwd.length() > Float::MIN_VALUE )
				fwd = fwd.normalize();
			float tilt = Math::toDegrees( Math::acos( m_activeCamera->forward().dot(fwd) ) );
			m_dbgFont->drawText( x, y, Format("  Tilt angle = {0,#.###}", tilt).format(), 0, &y );
		}

		if ( m_hero && m_cfg->getBoolean("Debug.CharacterInfo") )
		{
			m_dbgFont->drawText( x, y, Format( "Character ({0})", m_hero->name() ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  Position = ({0,#.##}, {1,#.##}, {2,#.##}) (cell {3})", m_hero->position().x, m_hero->position().y, m_hero->position().z, m_hero->cell()->name() ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  Velocity = ({0,#.##}, {1,#.##}, {2,#.##}) (speed={3})", m_hero->velocity().x, m_hero->velocity().y, m_hero->velocity().z, m_hero->velocity().length() ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  State = {0}", m_hero->stateString() ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  Morph = {0}", m_hero->morphStateString() ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  Animation 1st = {0}", m_hero->animationPrimaryStateString() ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  Animation 2nd = {0}", m_hero->animationSecondaryStateString() ).format(), 0, &y );
			if ( m_hero->invulnerable() ) 
				m_dbgFont->drawText( x, y, Format( "  Health = Invulnerable", m_hero->health() ).format(), 0, &y );			
			else
				m_dbgFont->drawText( x, y, Format( "  Health = {0,###}%", m_hero->health() ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  Ground brightness = {0,#}%", m_hero->groundLightmapColor().brightness()*100 ).format(), 0, &y );
			m_dbgFont->drawText( x, y, Format( "  Ground type = {0,#}", m_hero->groundMaterial()->getString("typeName") ).format(), 0, &y );
			Light* lt = m_hero->keylight();
			m_dbgFont->drawText( x, y, Format( "  Key light = {0}", lt ? lt->name() : "none" ).format(), 0, &y );

			// movement control vector
			Vector3 vec = m_hero->userControl()->movementControlVector();
			float ang = Math::toDegrees(Math::atan2(vec.z,vec.x));
			if ( ang < 0.f )
				ang += 360.f;
			m_dbgFont->drawText( x, y, Format( "  Control vec = {0,#.000} {1,#.000} (len={2,#.000}, deg={3,#})", vec.x, vec.z, vec.length(), ang ).format(), 0, &y );

			// noise level
			m_dbgFont->drawText( x, y, Format( "  Noise level = {0,#.00}", m_maxNoise?m_maxNoise->noiseLevel():0.f ).format(), 0, &y );

			if ( m_hero->hasWeapon() )
			{
				GameWeapon* weapon = m_hero->weapon();
				m_dbgFont->drawText( x, y, Format( "  Weapon: {0}; Shells {1}, Recoil Error {2}", weapon->name(), weapon->shellsRemaining(), Math::toDegrees(weapon->accumulatedRecoilError()) ).format(), 0, &y );
			}
		}

		if ( m_projectileMgr && m_cfg->getBoolean("Debug.ProjectileInfo" ) )
		{
		}

		if ( m_cfg->getBoolean("Debug.SoundInfo") )
		{
				m_dbgFont->drawText( x, y, Format("active sounds = {0}", m_soundMgr->activeSounds()).format(), 0, &y );
				for ( int i = 0 ; i < m_soundMgr->activeSounds() ; ++i )
					m_dbgFont->drawText( x, y, Format("sound{0} = {1}", i, m_soundMgr->getActiveSound(i)->stateString()).format(), 0, &y );
		}

		// profiling
		if ( m_cfg->getBoolean("Debug.Profiling") )
		{
			if ( m_timeProfiled > 1.f )
			{
				m_profiles.clear();
				for ( int i = 0 ; i < Profile::count() ; ++i ) 
				{ 
					Profile::BlockInfo* block = Profile::get(i);
					float time = (float)block->time() * 100.f;
					if ( time > 1.f )
						m_profiles.add( Format("\"{0}\" ({2,#}x) CPU usage {1,#}%", block->name(), time, block->count()/m_fps).format() );
				}
				std::sort( m_profiles.begin(), m_profiles.end() );
				Profile::reset();
				m_timeProfiled = 0;
			}

			for ( int i = 0 ; i < m_profiles.size() ; ++i ) 
				m_dbgFont->drawText( x, y, m_profiles[i], 0, &y );
		}

		#ifdef _DEBUG
		m_dbgFont->drawText( x, y, Format("Memory in use {0} kB in {1} blocks", mem_bytesInUse()/1024, mem_blocksInUse()).format(), 0, &y );
		#endif // _DEBUG

		Context::device()->resetStatistics();
	}

	void initGameObjects()
	{
		assert( m_hero );

		// create game cameras
		const int cameraCount = m_cfg->getInteger("Game.CameraCount");
		if ( cameraCount < 1 )
			throw Exception( Format("Camera count must be > 0") );

		// create on-screen info display
		m_onscreen = new OverlayDisplay( m_vm, m_arch, m_soundMgr, m_gameCtrl );
		m_onscreen->compile( "onscreen.lua" );

		refreshShadowFiller();
		refreshListenerParam();

		// print init time
		long initTime = System::currentTimeMillis() - m_initStart;
		Debug::println( "Game initialization took {0} seconds", (float)initTime/1e3f );

		// create cameras
		String activeCameraName = m_gameCtrl->activeCameraName();
		for ( int i = 0 ; i < cameraCount ; ++i )
		{
			P(GameCamera) cam = new GameCamera( m_vm, m_arch, m_sceneMgr, m_soundMgr, m_particleMgr );
			m_cameras.add( cam );
			cam->compile(  Format("camera_{0,#}.lua",i+1).format() );
			
			cam->setTarget( m_hero );
			
			if ( m_level && cam->name() == activeCameraName )
				selectActiveCamera( i );
		}
		if ( !m_activeCamera )
			throw Exception( Format("Default camera ({0}) not found!", activeCameraName) );

		// remove enemies if requested
		if ( m_cfg->getBoolean("Debug.RemoveNPCs") )
			m_level->removeNonMainCharacters();

		// print functions
		#ifdef _DEBUG
		if ( m_cfg->getBoolean("Debug.PrintFunctions") )
		{
			m_cameras[0]->printFunctions( "Camera" );
			m_level->getCell(0)->printFunctions( "Cell" );
			m_hero->printFunctions( "Character" );
			if ( m_level->dynamicObjects() > 0 )
				m_level->getDynamicObject(0)->printFunctions( "Dynamic Object" );
			m_level->printFunctions( "Level" );
			m_hero->weapon()->printFunctions( "Weapon" );
			if ( m_level->paths() > 0 )
				m_level->getPath(0)->printFunctions( "Path" );
			if ( m_level->triggers() > 0 )
				m_level->getTrigger(0)->printFunctions( "Trigger" );
		}
		#endif
	}

	void refreshListenerParam()
	{
		float distanceFactor = m_cfg->getFloat("Listener.DistanceFactor");
		float dopplerFactor = m_cfg->getFloat("Listener.DopplerFactor");
		float rolloffFactor = m_cfg->getFloat("Listener.RolloffFactor");
		m_soundMgr->setListenerParam( distanceFactor, dopplerFactor, rolloffFactor );
	}

	void flyCamera( GameCamera* camera, float dt )
	{
		static String flyCamCountStr	= "FlyCamera.Count";
		static String flyCamSelectedStr	= "FlyCamera.Selected";
		static String flyCamMovementStr	= "";
		static String flyCamRotationStr	= "";

		int count = m_cfg->getInteger(flyCamCountStr);
		int selected = m_cfg->getInteger(flyCamSelectedStr);
		static int oldSelected = -1;
		if ( selected != oldSelected )
		{
			flyCamMovementStr = Format("FlyCamera.MovementSpeed{0,#}",selected).format();
			flyCamRotationStr = Format("FlyCamera.RotationSpeed{0,#}",selected).format();
			oldSelected = selected;
		}
		float flySpeed = m_cfg->getFloat(flyCamMovementStr);
		float flySpeedRot = m_cfg->getFloat(flyCamRotationStr);
		float deltaPos = flySpeed / 3.6f * dt;
		float deltaRot = Math::toRadians(flySpeedRot) * dt;
		Vector3 pos = camera->transform().translation();
		Vector3 dx = camera->transform().rotation().getColumn(0) * deltaPos;
		Vector3 dy = camera->transform().rotation().getColumn(1) * deltaPos;
		Vector3 dz = camera->transform().rotation().getColumn(2) * deltaPos;
		Matrix3x3 rot = camera->transform().rotation();
		bool shiftDown = (GetKeyState(VK_SHIFT) < 0);

		for ( int i = 1 ; i <= count ; ++i )
			if ( GetKeyState('1'+i-1) < 0 )
				m_cfg->setInteger( flyCamSelectedStr, i );

		if ( GetKeyState('W') < 0 )
			pos += dz;
		if ( GetKeyState('S') < 0 )
			pos -= dz;
		if ( GetKeyState('A') < 0 || 
			(GetKeyState(VK_LEFT) < 0 && shiftDown) )
			pos -= dx;
		if ( GetKeyState('D') < 0 ||
			(GetKeyState(VK_RIGHT) < 0 && shiftDown) )
			pos += dx;
		if ( GetKeyState(VK_UP) < 0 && shiftDown )
			pos += dy;
		if ( GetKeyState(VK_DOWN) < 0 && shiftDown )
			pos -= dy;
		if ( GetKeyState(VK_LEFT) < 0 && !shiftDown )
			rot *= Matrix3x3(Vector3(0,1,0), -deltaRot);
		if ( GetKeyState(VK_RIGHT) < 0 && !shiftDown )
			rot *= Matrix3x3(Vector3(0,1,0), deltaRot);
		if ( GetKeyState(VK_UP) < 0 && !shiftDown )
			rot *= Matrix3x3(Vector3(1,0,0), deltaRot);
		if ( GetKeyState(VK_DOWN) < 0 && !shiftDown )
			rot *= Matrix3x3(Vector3(1,0,0), -deltaRot);

		// eliminate banking
		rot.setColumn( 0, rot.getColumn(0) - Vector3(0,1,0)*rot.getColumn(0).dot(Vector3(0,1,0)) );
		camera->moveWithoutColliding( pos - camera->position() );
		camera->setRotation( rot.orthonormalize() );
	}

	void start()
	{
	}

	void focusLost()
	{
		Debug::println( "Game.focusLost" );

		resetInputState();

		Debug::println( "Flushing Input Devices" );
		for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
			m_inputDriver->getAttachedInputDevice(i)->flushEvents();
	}

	int	createSound()
	{
		return 0;
	}

	void selectActiveCamera( int n )
	{
		assert( n < m_cameras.size() );
		assert( m_hero );

		P(GameCamera) oldcam = m_activeCamera;
		if ( !oldcam || !m_level->isActiveCutScene() )
		{
			// update new camera for a second
			m_activeCamera = m_cameras[n];

			if ( oldcam )
				m_activeCamera->setTransform( oldcam->cell(), oldcam->transform() );
			else
				m_activeCamera->setTransform( m_hero->cell(), m_hero->transform() );

			const float dt = 20e-3f;
			for ( float t = 0.f ; t < 1.f ; t += dt )
				m_activeCamera->update( dt );

			m_gameCtrl->resetCrosshairPos();

			m_vm->pushTable( m_activeCamera );
			m_vm->setGlobal( "camera" );
		}
	}

	void resetInputState()
	{
		if ( m_hero )
			m_hero->resetInputState();
	}

	GameCamera*	activeCamera() const
	{
		return m_activeCamera;
	}

	void setInvulnerable()
	{
		m_hero->setInvulnerable( !m_hero->invulnerable() );
	}

	void skipNoticeScreen()
	{
		m_noticeTime = m_cfg->getFloat("Game.NoticeTime");
	}

	void setArcBallCameraEnabled( bool enabled )
	{
		m_arcBallCameraEnabled = enabled;
	}

	bool arcBallCameraEnabled() const
	{
		return m_arcBallCameraEnabled;
	}

private:
	P(snd::SoundManager)			m_soundMgr;
	P(ps::ParticleSystemManager)	m_particleMgr;
	P(music::MusicManager)			m_musicMgr;
	P(sgu::SceneManager)			m_sceneMgr;
	P(GameNoiseManager)				m_noiseMgr;
	P(ProjectileManager)			m_projectileMgr;
	P(GameController)				m_gameCtrl;

	P(InputStreamArchive)			m_arch;
	P(Context)						m_context;
	P(Font)							m_dbgFont;
	P(ExProperties)					m_cfg;
		
	P(VM)							m_vm;
	P(Sprite)						m_endScreen;
	P(Sprite)						m_noticeScreen;
	bool							m_noticeScreenEnabled;
	float							m_noticeTime;
	Timer							m_timer;
	Vector<P(GameCamera)>			m_cameras;
	P(OverlayDisplay)				m_onscreen;
	P(GameCharacter)				m_hero;
	P(GameLevel)					m_level;
	P(GameCamera)					m_activeCamera;
	P(Scene)						m_scene;
	P(Node)							m_soundListener;

	P(Mesh)							m_dbgLineMesh;
	P(LineList)						m_dbgLines2D;
	P(LineList)						m_dbgLines3D;
	long							m_initStart;
	float							m_fps;
	P(GameNoise)					m_maxNoise;
	float							m_timeProfiled;
	bool							m_arcBallCameraEnabled;

	P(Mesh)							m_shadowFiller;
	P(ShadowShader)					m_shadowShader;

	P(InputDriver)					m_inputDriver;
	bool							m_wasActiveCutScene;

	Vector<String>					m_profiles;

	Impl( const Impl& );
	Impl& operator=( const Impl& );
};

//-----------------------------------------------------------------------------

Game::Game( VM* vm, InputStreamArchive* arch, Context* context, Font* dbgFont, ExProperties* cfg, 
	SoundManager* soundMgr, ParticleSystemManager* particleMgr, MusicManager* musicMgr, 
	SceneManager* sceneMgr, InputDriver* inputDriver )
{
	m_this = new Impl( vm, arch, context, dbgFont, cfg, soundMgr, particleMgr, musicMgr, sceneMgr, inputDriver );
}

Game::~Game()
{
}

void Game::update( float dt )
{
	m_this->update( dt );
}

void Game::render()
{
	m_this->render();
}

void Game::destroy()
{
	m_this = 0;
}

void Game::focusLost()
{
	m_this->focusLost();
}

void Game::selectActiveCamera( int n )
{
	m_this->selectActiveCamera(n);
}

void Game::resetInputState() 
{
	m_this->resetInputState();
}

void Game::setInvulnerable() 
{
	m_this->setInvulnerable();
}

GameCamera*	Game::activeCamera() const
{
	return m_this->activeCamera();
}

GameLevel* Game::level() const
{
	return m_this->level();
}

void Game::skipNoticeScreen()
{
	m_this->skipNoticeScreen();
}

void Game::setArcBallCameraEnabled( bool enabled )
{
	m_this->setArcBallCameraEnabled( enabled );
}

bool Game::arcBallCameraEnabled() const
{
	return m_this->arcBallCameraEnabled();
}

