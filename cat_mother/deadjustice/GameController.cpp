#include "GameController.h"
#include "GameCharacter.h"
#include "GameCamera.h"
#include "GameCell.h"
#include "GameCutScene.h"
#include "GamePointObject.h"
#include "GameLevel.h"
#include "UserControl.h"
#include "ScriptUtil.h"
#include "CollisionInfo.h"
#include <id/InputDevice.h>
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <sgu/NodeUtil.h>
#include <bsp/BSPCollisionUtil.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <lang/String.h>
#include <lang/Integer.h>
#include <math/Vector3.h>
#include <math/Intersection.h>
#include <util/Hashtable.h>
#include <util/ExProperties.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include <sg/Camera.h>
#include <stdlib.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace bsp;
using namespace id;
using namespace io;
using namespace lang;
using namespace util;
using namespace math;
using namespace script;
using namespace sg;
using namespace sgu;

//-----------------------------------------------------------------------------

ScriptMethod<GameController> GameController::sm_methods[] =
{
	//ScriptMethod<GameController>( "funcName", script_funcName ),
	ScriptMethod<GameController>( "hasJoystickControls", script_hasJoystickControls ),
};

//-----------------------------------------------------------------------------

static const String CONTROLLER_CONFIG_NAME = "controller.prop";

static const String AXIS_STRING = "AXIS";
static const String BUTTON_STRING = "BUTTON";
static const String KEY_STRING = "KEY";
static const String HAT_STRING = "HAT";
static const String RELAXIS_STRING = "RELATIVE";
static const String NONE_STRING = "NONE";

//-----------------------------------------------------------------------------

GameController::GameController( VM* vm, InputStreamArchive* arch, InputDriver* inputDriver, ExProperties* gameconfig ) :
	GameScriptable( vm, arch, 0, 0 ),
	m_methodBase( -1 ),
	m_inputDriver( inputDriver ),
	m_cfg( gameconfig ),
	m_devicesToPoll( Allocator<int>(__FILE__) ),
	m_controllerCfg( 0 ),
	m_activeControlSet( -1 ),
	m_controlSets( Allocator<ControlSet>(__FILE__) ),
	m_configuringInput( false ),
	m_configuringAction( -1 ),
	m_controllersDirty( false ),
	m_setHasJoystickControls( false ),
	m_keyboardRunToggle( true ),
	m_needToResetInputState( true ),
	m_needToResetCrosshairPos( true ),
	m_crosshairPos( Vector3(0,0,0) ),
	m_crosshairShouldBeAtPos( Vector3(0,0,0) ),
	m_aimCollisionInfo(),
	m_crosshairAverageBuffer( Allocator<Vector2>(__FILE__) ),
	m_collisionTester( new GamePointObject(CollisionInfo::COLLIDE_GEOMETRY_SOLID+CollisionInfo::COLLIDE_OBJECT) ),
	m_keyStrokes( Allocator<id::InputDevice::Event>(__FILE__) )
{
	m_methodBase = ScriptUtil<GameController,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	// load controller config
	loadConfiguration();

	bool						actionsfound = false;
	lang::String				activecontroller;
	Array<Array<lang::String> > controlmapping( GameAction::ACTION_COUNT );

	/* Initialize input devices */
	resetDevices();

	/* Check config for GameAction mapping */
	if ( m_controllerCfg->containsKey( "ActiveControlset" ) ) actionsfound = true;
	
	if ( !actionsfound )
	{		
		activecontroller = m_controlSets[ m_activeControlSet ].name();

		/* Set control strings */
		m_controllerCfg->put( "ActiveControlset", activecontroller );

		for ( int i = 0; i < GameAction::ACTION_COUNT; ++i )
			controlmapping[i].setSize(3);

		for ( int s = 0; s < m_controlSets.size(); ++s )
		{
			for ( int i = 0; i < GameAction::ACTION_COUNT; ++i )
			{
				GameAction& gameaction = m_controlSets[s].getAction(i);

				controlmapping[i][0] = inputDriver->getAttachedInputDevice( gameaction.device() )->name();
				if ( gameaction.eventCode() & InputDevice::EVENT_AXIS_MASK )
					controlmapping[i][1] = Format("{1}{0,###}", gameaction.eventCode() >> InputDevice::EVENT_AXIS_BITOFFSET, AXIS_STRING ).format();
				else if ( gameaction.eventCode() & InputDevice::EVENT_BUTTON_MASK )
					controlmapping[i][1] = Format("{1}{0,###}", gameaction.eventCode() >> InputDevice::EVENT_BUTTON_BITOFFSET, BUTTON_STRING ).format();
				else if ( gameaction.eventCode() & InputDevice::EVENT_KEY_MASK )
					controlmapping[i][1] = Format("{1}{0,###}", gameaction.eventCode() >> InputDevice::EVENT_KEY_BITOFFSET, KEY_STRING ).format();
				else if ( gameaction.eventCode() & InputDevice::EVENT_HAT_MASK )
					controlmapping[i][1] = Format("{1}{0,###}", gameaction.eventCode() >> InputDevice::EVENT_HAT_BITOFFSET, HAT_STRING ).format();
				else if ( gameaction.eventCode() & InputDevice::EVENT_RELAXIS_MASK )
					controlmapping[i][1] = Format("{1}{0,###}", gameaction.eventCode() >> InputDevice::EVENT_RELAXIS_BITOFFSET, RELAXIS_STRING ).format();
				else 
					controlmapping[i][1] = Format("{1}{0}", String("0"), NONE_STRING ).format();

				controlmapping[i][2] = GameAction::toString( gameaction.action() );
			}
		
			for ( int i = 0; i < GameAction::ACTION_COUNT; ++i )
				m_controllerCfg->setStrings( Format("Controller.{0}.{1}", m_controlSets[s].name(), 
    								 controlmapping[i][2] ).format(),			
	     							controlmapping[i].begin(), 2 );		
		}

		/* Set deadzones for joystick types. */
		for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
		{
			InputDevice* controller = m_inputDriver->getAttachedInputDevice(i);

			switch ( controller->deviceType() )
			{
			case InputDevice::TYPE_JOYSTICK:
				{
					for ( int e = 0; e < controller->eventCodeCount(); e++ )
					{
						unsigned int code = controller->getEventCode( e );

						if ( code & InputDevice::EVENT_AXIS_MASK )
						{
							int axis = ( code & InputDevice::EVENT_AXIS_MASK ) >> InputDevice::EVENT_AXIS_BITOFFSET;
						
							if ( axis == 1 || axis == 5)
							{
								controller->setDeadZone( code, 0.20f );
								m_controllerCfg->put( Format( "DeadZone.{0}.{1}", controller->name(), code ).format(), Format("{0}",0.20f).format() );
							}
							else if ( axis == 2 || axis == 6 )
							{
								controller->setDeadZone( code, 0.15f );
								m_controllerCfg->put( Format( "DeadZone.{0}.{1}", controller->name(), code ).format(), Format("{0}",0.15f).format() );
							}
							else if ( axis == 3 || axis == 7 )
							{
								controller->setDeadZone( code, 0.20f );
								m_controllerCfg->put( Format( "DeadZone.{0}.{1}", controller->name(), code ).format(), Format("{0}",0.20f).format() );
							}
							else if ( axis == 4 || axis == 8 )
							{
								controller->setDeadZone( code, 0.15f );
								m_controllerCfg->put( Format( "DeadZone.{0}.{1}", controller->name(), code ).format(), Format("{0}",0.15f).format() );
							}
						}
					}
				}
				break;
			case InputDevice::TYPE_GAMEPAD:
				{
					for ( int e = 0; e < controller->eventCodeCount(); e++ )
					{
						unsigned int code = controller->getEventCode( e );

						if ( code & InputDevice::EVENT_AXIS_MASK )
						{						
							controller->setDeadZone( code, 0.10f );
							m_controllerCfg->put( Format( "DeadZone.{0}.{1}", controller->name(), code ).format(), Format("{0}",0.10f).format() );
						}
					}
				}				
				break;
			default:;
			}

		}

	}
	else
	{
//		m_controlSets.clear();

		// Get strings from file, loop trough them and create control sets

		HashtableIterator<lang::String, lang::String> entry = m_controllerCfg->begin();

		for ( ; entry != m_controllerCfg->end(); ++entry )
		{
			String controller("Controller.");
			if ( entry.key().regionMatches( 0, controller, 0, controller.length() ) )
			{ 
				String setAndAction = entry.key().substring( controller.length() );
				int separator		= setAndAction.lastIndexOf( "." );
				String set			= setAndAction.substring( 0, separator );
				String action		= setAndAction.substring( separator + 1 );
				int valueseparator	= entry.value().lastIndexOf( " " );
				String device		= entry.value().substring( 0, valueseparator );
				String control		= entry.value().substring( valueseparator + 1 );
				unsigned int code	= 0;

				if ( control.indexOf( AXIS_STRING ) != -1 )
				{
					code = Integer::parseInt( control.substring( AXIS_STRING.length() ) );
					code <<= InputDevice::EVENT_AXIS_BITOFFSET;
				}
				else if ( control.indexOf( BUTTON_STRING ) != -1 )
				{
					code = Integer::parseInt( control.substring( BUTTON_STRING.length() ) );
					code <<= InputDevice::EVENT_BUTTON_BITOFFSET;
				}
				else if ( control.indexOf( KEY_STRING ) != -1 )
				{
					code = Integer::parseInt( control.substring( KEY_STRING.length() ) );
					code <<= InputDevice::EVENT_KEY_BITOFFSET;
				}
				else if ( control.indexOf( HAT_STRING ) != -1 )
				{
					code = Integer::parseInt( control.substring( HAT_STRING.length() ) );
					code <<= InputDevice::EVENT_HAT_BITOFFSET;
				}
				else if ( control.indexOf( RELAXIS_STRING ) != -1 )
				{
					code = Integer::parseInt( control.substring( RELAXIS_STRING.length() ) );
					code <<= InputDevice::EVENT_RELAXIS_BITOFFSET;
				}
				else
				{
					code = 0;
				}

				addSetAndAction( set, action, device, code );
			}

			String deadzone("DeadZone.");
			if ( entry.key().regionMatches( 0, deadzone, 0, deadzone.length() ) )
			{
				String deviceAndEvent	= entry.key().substring( deadzone.length() );
				int separator			= deviceAndEvent.lastIndexOf( "." );
				String device			= deviceAndEvent.substring( 0, separator );
				String event			= deviceAndEvent.substring( separator + 1 );
				int code				= Integer::parseInt( event );
				float value				= Float::parseFloat( entry.value() );

				InputDevice* controller = m_inputDriver->getAttachedInputDevice( getInputDevice( device ) );
				if ( controller )
				{
					controller->setDeadZone( code, value );
				}
				else
				{
					Debug::printlnError( Format("Controller.prop entry {0} = {1} does not point to an existing input device", entry.key(), entry.value()).format() );
				}
			}
		}

		/* Get active controlset */
		activecontroller = m_controllerCfg->get( "ActiveControlset" );

		/* Assing controlset */
		for ( int i = 0; i < m_controlSets.size(); ++i )
			if ( activecontroller == m_controlSets[i].name() )
			{
				selectActiveControlSet( i );
			}
	}
	/* Store cfg file */
	saveConfiguration();

	/* Set Poll on to all devices */
	m_devicesToPoll.clear();

	for (int i = 0; i < m_inputDriver->attachedInputDevices(); ++i)
	{
		/* Add to poll list */
		m_devicesToPoll.add( i );
		/* Flush input buffer */
		m_inputDriver->getAttachedInputDevice( i )->flushEvents();
	}
}

GameController::~GameController()
{
	saveConfiguration();
}

int GameController::methodCall( script::VM* vm, int i )
{
	return ScriptUtil<GameController,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

void GameController::resetCrosshairPos() 
{
	m_needToResetCrosshairPos = true;
}

void GameController::update( float dt )
{
	GameScriptable::update( dt );

	/* Poll device input */
	for ( int i = 0; i < m_devicesToPoll.size(); ++i ) 
	{
		InputDevice* controller = m_inputDriver->getAttachedInputDevice( m_devicesToPoll[i] );
		if ( controller == 0 ) 
			continue;
		
		controller->poll();
		
		int events = controller->events();	
		if ( events > 0 ) 
			for ( int j = 0 ; j < events ; ++j )
			{
				InputDevice::Event event;
				controller->getEvent( j, &event );

			// Configure currently selected controlset action with input value
				if ( m_configuringInput && event.value > 0.5f )
				{					
					GameAction& action = m_controlSets[m_activeControlSet].getAction( m_configuringAction );
					action.setDevice( m_devicesToPoll[i] );
					action.setEventCode( event.code );
					
					m_configuringInput = false;
				}
			// Forward keystroke to specific buffer
				if ( event.code & InputDevice::EVENT_KEY_MASK )
				{
					m_keyStrokes.add(event);
				}				
			}
	}
}

void GameController::flushDevices()
{
	InputDevice* dev;

	for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
	{
		dev = m_inputDriver->getAttachedInputDevice( i );
		dev->flushEvents();
	}
}

void GameController::resetDevices()
{
	m_controlSets.setSize( 0 );
	m_activeControlSet = -1;
	m_inputDriver->refreshAttachedInputDevices();

	for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
	{
		InputDevice* dev = m_inputDriver->getAttachedInputDevice( i );
		InputDevice::DeviceType type = dev->deviceType();
			
		if ( type != InputDevice::TYPE_UNSUPPORTED && type != InputDevice::TYPE_KEYBOARD && type != InputDevice::TYPE_GAMEPAD && type != InputDevice::TYPE_FLIGHT )
		{
			m_controlSets.add( ControlSet( m_inputDriver, m_controllerCfg ) );
			m_controlSets[m_controlSets.size()-1].initializeByDevice( i );
		}

		/* Use first joystick type device if found */
		if ( m_activeControlSet == -1 && type == InputDevice::TYPE_JOYSTICK &&
			dev->buttons() == 16 && dev->axes() == 4 )
		{
			Debug::println( "Selected joystick controller: {0}", dev->name() );
			selectActiveControlSet ( m_controlSets.size()-1 );
		}
	}	

	if ( m_controlSets.size() == 0 )
		throw Exception( Format( "Dead Justice requires that a Mouse+Keyboard or a (PS2) Joystick is connected to the computer" ) ); 

	if ( m_activeControlSet == -1 )
	/* Use Mouse */
		for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
		{
			InputDevice* dev = m_inputDriver->getAttachedInputDevice( i );
			InputDevice::DeviceType type = dev->deviceType();
				
			if ( type == InputDevice::TYPE_MOUSE )
				selectActiveControlSet( i );
		}
}

void GameController::focusLost()
{
	m_needToResetInputState = true;	

	if ( m_inputDriver )
		m_inputDriver->focusLost();
}

ControlSet&	GameController::getControlSet( int index )
{
	if ( index > m_controlSets.size() )
		throw Exception( Format( "Control set index out of range!" ) );

	return m_controlSets[index];
}

void GameController::selectActiveControlSet( int index )
{
	if ( index > m_controlSets.size() )
		throw Exception( Format( "Control set index out of range!" ) );

	m_activeControlSet = index;
	m_setHasJoystickControls = m_controlSets[ m_activeControlSet ].hasJoystickControls();

	setDevicesToPoll();
}

void GameController::setDevicesToPoll()
{
	m_devicesToPoll.clear();

	for (int i = 0; i < m_inputDriver->attachedInputDevices(); ++i)
		m_devicesToPoll.add( i );
}

bool GameController::configuringInput() const 
{
	return m_configuringInput;
}

void GameController::configureAction( int index )
{
	if ( index > m_controlSets[m_activeControlSet].configurableActions() )
		throw Exception( Format( "Action index out of range!" ) );

	m_configuringInput = true;	
	m_configuringAction = m_controlSets[m_activeControlSet].configurableActionIndexIs( index );

	/* Set Poll on to all devices for duration of configuring */
	m_devicesToPoll.clear();

	for (int i = 0; i < m_inputDriver->attachedInputDevices(); ++i)
	{
		/* Add to poll list */
		m_devicesToPoll.add( i );
		/* Flush input buffer */
		m_inputDriver->getAttachedInputDevice( i )->flushEvents();
	}
}

void GameController::loadConfiguration() 
{
	if ( m_cfg->getBoolean("Debug.LoadAndSaveControllerConfig") )
	{
		m_controllerCfg = new ExProperties();

		if ( !File(CONTROLLER_CONFIG_NAME).exists() )
		{
			FileOutputStream cfgout( CONTROLLER_CONFIG_NAME );
			m_controllerCfg->setInteger( "Version", VERSION );
			m_controllerCfg->store( &cfgout, "# controller config load/saved by deadjustice.exe" );
			cfgout.close();
		}

		FileInputStream cfgin( CONTROLLER_CONFIG_NAME );
		m_controllerCfg->load( &cfgin );
		cfgin.close();

		int version = m_controllerCfg->getInteger( "Version" );
		if ( version != VERSION )
			throw Exception( Format( "Controller.prop is wrong version." ) );
	}
	else
	{
		m_controllerCfg = new ExProperties();
	}
}

void GameController::saveConfiguration() 
{
	if ( m_cfg->getBoolean("Debug.LoadAndSaveControllerConfig") )
	{
		m_controllerCfg->setInteger( "Version", VERSION );
		FileOutputStream out( CONTROLLER_CONFIG_NAME );
		m_controllerCfg->store( &out, "Controller settings. Editing this file is discouraged" );
		out.close();
	}
}

void GameController::controlArcBallCamera( GameCharacter* character, GameCamera* camera, float dt, 
	float targetHeight, float rotationSpeed, float dollySpeed, float dollyMin, float dollyMax, float tiltMax )
{
	character->resetInputState();

	// HACK: arc ball camera state
	static float s_angleX = 0.f;
	static float s_angleY = 0.f;
	static float s_distance = 2.f;
	static float s_dollySpeedF = 0.f;
	static float s_dollySpeedB = 0.f;

	// Process controller events
	ControlSet& set = m_controlSets[ m_activeControlSet ];
	GameAction& accelerateAction	= set.getAction((int)GameAction::ACCELERATE);
	GameAction& backwardAction		= set.getAction((int)GameAction::BACKWARD);

	for ( int i = 0; i < m_devicesToPoll.size(); ++i ) 
	{
		int devindex = m_devicesToPoll[i];
		InputDevice* controller = m_inputDriver->getAttachedInputDevice( devindex );
		if ( controller == 0 ) 
			continue;

		int events = controller->events();
		for ( int j = 0 ; j < events ; ++j )
		{
			InputDevice::Event event;
			controller->getEvent( j, &event );

			if ( accelerateAction.matches( devindex, event.code ) )
				s_dollySpeedF = event.value * dollySpeed;
			if ( backwardAction.matches( devindex, event.code ) )
				s_dollySpeedB = event.value * dollySpeed;

			processCrosshairInputEvents( camera, devindex, event );
		}
	}

	updateCrosshairPos( camera );

	// apply turning to rotation about Y-axis
	float turnLeft, turnRight, pitch;
	getTurningStrengthFromCrosshairPos( camera, &turnLeft, &turnRight, &pitch, 0 );
	s_angleY += turnLeft*dt*rotationSpeed;
	s_angleY -= turnRight*dt*rotationSpeed;
	s_angleX = pitch * tiltMax;
	
	// apply forward/backward moving to distance
	s_distance -= (s_dollySpeedF-s_dollySpeedB) * dt;
	if ( s_distance < dollyMin )
		s_distance = dollyMin;
	if ( s_distance > dollyMax )
		s_distance = dollyMax;

	// spine world position
	String boneName = "Bip01 Pelvis";
	Node* spine = NodeUtil::findNodeByName( character->getRenderObject(0), boneName );
	if ( !spine )
		throw Exception( Format("Bone {0} not found in {1}", boneName, character->name()) );
	Vector3 worldPelvis = spine->worldTransform().translation();
	worldPelvis.y += targetHeight;

	// update camera
	Matrix3x3 rot = Matrix3x3(Vector3(0,1,0),s_angleY) * Matrix3x3(Vector3(1,0,0),s_angleX);
	Vector3 delta = rot * Vector3(0,0,s_distance);
	camera->setPosition( character->cell(), character->position() );
	camera->moveWithoutColliding( worldPelvis - camera->position() );
	camera->move( delta, 0 );
	camera->lookAt( worldPelvis );

	//Debug::println( "angleX={0}, angleY={3}, turnLeft={1}, turnRight={2}, s_dollySpeed={4}, dollySpeed={5}, dt={6}", s_angleX, turnLeft, turnRight, s_angleY, s_dollySpeed, dollySpeed, dt );
}

void GameController::controlCutScene( GameCamera* camera, float /*dt*/ )
{
	assert( camera );
	
	// Process controller events
	ControlSet& set = m_controlSets[ m_activeControlSet ];
	GameAction& skipCutSceneAction	= set.getAction((int)GameAction::ATTACK_KICK);

	for ( int i = 0; i < m_devicesToPoll.size(); ++i ) 
	{
		int devindex = m_devicesToPoll[i];
		InputDevice* controller = m_inputDriver->getAttachedInputDevice( devindex );
		if ( controller == 0 ) 
			continue;

		int events = controller->events();
		if ( events > 0 ) 
		{	
			for ( int j = 0 ; j < events ; ++j )
			{
				InputDevice::Event event;
				controller->getEvent( j, &event );

				if ( skipCutSceneAction.matches( devindex, event.code ) && event.value > 0.5f )
					camera->level()->skipCutScene();;
			}
		}
	}
}

void GameController::controlCharacter( GameCharacter* character, GameCamera* camera, float dt ) 
{
	GameCharacter* activePlayer = character;

	assert( activePlayer );
	if ( m_needToResetInputState )
	{
		resetInputState( activePlayer );
		m_needToResetInputState = false;
	}

	if ( m_needToResetCrosshairPos )
	{
		m_crosshairPos.x = camera->crosshairCenter().x;
		m_crosshairPos.y = camera->crosshairCenter().y;
		m_crosshairShouldBeAtPos.x = camera->crosshairCenter().x;
		m_crosshairShouldBeAtPos.y = camera->crosshairCenter().y;
		m_needToResetCrosshairPos = false;
	}

	// Process controller events
	ControlSet& set = m_controlSets[ m_activeControlSet ];
	GameAction& attackAction		= set.getAction((int)GameAction::ATTACK);
	GameAction& attackStrikeAction	= set.getAction((int)GameAction::ATTACK_STRIKE);
	GameAction& attackKickAction	= set.getAction((int)GameAction::ATTACK_KICK);
	GameAction& accelerateAction	= set.getAction((int)GameAction::ACCELERATE);
	GameAction& backwardAction		= set.getAction((int)GameAction::BACKWARD);
	GameAction& strafeRightAction	= set.getAction((int)GameAction::STRAFERIGHT);
	GameAction& strafeLeftAction	= set.getAction((int)GameAction::STRAFELEFT);
	GameAction& rollAction			= set.getAction((int)GameAction::ROLL);
	GameAction& crouchAction		= set.getAction((int)GameAction::CROUCH);
	GameAction& peekRightAction		= set.getAction((int)GameAction::PEEKRIGHT);
	GameAction& peekLeftAction		= set.getAction((int)GameAction::PEEKLEFT);
	GameAction& changeClipAction	= set.getAction((int)GameAction::CHANGECLIP);
	GameAction& cycleWeaponAction	= set.getAction((int)GameAction::CYCLEWEAPON);
	GameAction& throwShellAction	= set.getAction((int)GameAction::THROWEMPTYSHELL);
	GameAction& runModifierAction	= set.getAction((int)GameAction::TOGGLE_RUN);
	GameAction& sneakModifierAction	= set.getAction((int)GameAction::MODIFIER_SNEAK);
	
	Vector3 oldCrosshairPos = m_crosshairShouldBeAtPos;
	for ( int i = 0; i < m_devicesToPoll.size(); ++i ) 
	{
		int devindex = m_devicesToPoll[i];
		InputDevice* controller = m_inputDriver->getAttachedInputDevice( devindex );
		if ( controller == 0 ) 
			continue;

		UserControl* userCtrl = activePlayer->userControl();
		if ( m_setHasJoystickControls )
		{
			userCtrl->runModifier( true );
			userCtrl->sneakModifier( false );
		}
		else
		{
			userCtrl->runModifier( m_keyboardRunToggle );
		}

		int events = controller->events();
		for ( int j = 0 ; j < events ; ++j )
		{
			InputDevice::Event event;
			controller->getEvent( j, &event );

			if ( attackAction.matches( devindex, event.code ) )
				activePlayer->userControl()->attack( event.value > 0.5f );
			
			if ( attackStrikeAction.matches( devindex, event.code ) )
				activePlayer->userControl()->physicalAttackStrike( event.value > 0.5f );

			if ( attackKickAction.matches( devindex, event.code ) )
				activePlayer->userControl()->physicalAttackKick( event.value > 0.5f );

			if ( accelerateAction.matches( devindex, event.code ) )
				activePlayer->userControl()->accelerate( event.value );
			
			if ( backwardAction.matches( devindex, event.code ) )
				activePlayer->userControl()->accelerateBackwards( event.value );
			
			if ( strafeRightAction.matches( devindex, event.code ) )
				activePlayer->userControl()->strafeRight( event.value );
			
			if ( strafeLeftAction.matches( devindex, event.code ) )
				activePlayer->userControl()->strafeLeft( event.value );
			
			if ( rollAction.matches( devindex, event.code ) )
				activePlayer->userControl()->roll( event.value );

			if ( crouchAction.matches( devindex, event.code ) )
				activePlayer->userControl()->crouch( event.value > 0.5f);

			if ( peekLeftAction.matches( devindex, event.code ) )
				activePlayer->userControl()->peekLeft( event.value );

			if ( peekRightAction.matches( devindex, event.code ) )
				activePlayer->userControl()->peekRight( event.value );

			if ( changeClipAction.matches( devindex, event.code ) )
				activePlayer->userControl()->changeClip( event.value > 0.5f );
			
			if ( cycleWeaponAction.matches( devindex, event.code ) )
				activePlayer->userControl()->cycleWeapon( event.value > 0.5f );

			if ( throwShellAction.matches( devindex, event.code ) )
				activePlayer->userControl()->throwEmptyShell( event.value > 0.5f );

			if ( runModifierAction.matches( devindex, event.code ) && event.value > 0.5f )
				m_keyboardRunToggle = !m_keyboardRunToggle;

			if ( sneakModifierAction.matches( devindex, event.code ) )
				activePlayer->userControl()->sneakModifier( event.value > 0.5f );

			processCrosshairInputEvents( camera, devindex, event );
		}
	}

	updateCrosshairPos( camera );
	applyCrosshairPos( activePlayer, camera, m_crosshairShouldBeAtPos-oldCrosshairPos, dt ); 
}

void GameController::resetInputState( GameCharacter* player ) 
{
	player->resetInputState();
	m_crosshairPos = Vector3(0,0,0);
	m_needToResetInputState = false;
}

void GameController::updateCrosshairPos( GameCamera* camera )
{
	// Clamp crosshair to limits
	if ( m_crosshairShouldBeAtPos.x > camera->horizontalLimit(1) )
		m_crosshairShouldBeAtPos.x = camera->horizontalLimit(1);
	if ( m_crosshairShouldBeAtPos.x < -camera->horizontalLimit(0) )
		m_crosshairShouldBeAtPos.x = -camera->horizontalLimit(0);
	if ( m_crosshairShouldBeAtPos.y > camera->verticalLimit(1) )
		m_crosshairShouldBeAtPos.y = camera->verticalLimit(1);
	if ( m_crosshairShouldBeAtPos.y < -camera->verticalLimit(0) )
		m_crosshairShouldBeAtPos.y = -camera->verticalLimit(0);

	addCrosshairAveragePos( Vector2( m_crosshairShouldBeAtPos.x, m_crosshairShouldBeAtPos.y ) );
	Vector2 cp = getAverageCrosshairPos();
	m_crosshairPos.x = cp.x;
	m_crosshairPos.y = cp.y;
	m_crosshairPos.z = 0;
}

void GameController::processCrosshairInputEvents( GameCamera* camera, int devindex, const id::InputDevice::Event& event )
{
	ControlSet& set = m_controlSets[ m_activeControlSet ];

	if ( m_setHasJoystickControls )
	{
		GameAction& crosshair_abs_right = set.getAction((int)GameAction::CROSSHAIR_ABS_RIGHT);
		GameAction& crosshair_abs_left	= set.getAction((int)GameAction::CROSSHAIR_ABS_LEFT);
		GameAction& crosshair_abs_down	= set.getAction((int)GameAction::CROSSHAIR_ABS_DOWN);
		GameAction& crosshair_abs_up	= set.getAction((int)GameAction::CROSSHAIR_ABS_UP);

		float crosshairExp = m_cfg->getFloat( "Control.PS2.Crosshair.AnalogStickExponentialFuncWindowSize" );

		if ( crosshair_abs_right.matches( devindex, event.code ) && m_crosshairShouldBeAtPos.x >= camera->crosshairCenter().x )
			m_crosshairShouldBeAtPos.x = exponentialize(event.value, crosshairExp) * (camera->horizontalLimit(1) - camera->crosshairCenter().x) 
				+ camera->crosshairCenter().x;

		if ( crosshair_abs_left.matches( devindex, event.code ) && m_crosshairShouldBeAtPos.x <= camera->crosshairCenter().x )
			m_crosshairShouldBeAtPos.x = -exponentialize(event.value, crosshairExp) * (camera->horizontalLimit(0) - camera->crosshairCenter().x) 
				+ camera->crosshairCenter().x;

		if ( crosshair_abs_down.matches( devindex, event.code ) && m_crosshairShouldBeAtPos.y >= camera->crosshairCenter().y )
			m_crosshairShouldBeAtPos.y = exponentialize(event.value, crosshairExp) * (camera->verticalLimit(1) - camera->crosshairCenter().y) 
				+ camera->crosshairCenter().y;

		if ( crosshair_abs_up.matches( devindex, event.code ) && m_crosshairShouldBeAtPos.y <= camera->crosshairCenter().y )
			m_crosshairShouldBeAtPos.y = -exponentialize(event.value, crosshairExp) * (camera->verticalLimit(0) 
				+ camera->crosshairCenter().y) + camera->crosshairCenter().y;
	}
	else
	{
		GameAction& crosshair_rel_x		= set.getAction((int)GameAction::CROSSHAIR_REL_X);
		GameAction& crosshair_rel_y		= set.getAction((int)GameAction::CROSSHAIR_REL_Y);

		float relativeAxisSensitivity = m_cfg->getFloat( "Control.PC.MouseSensitivity" );

		if ( crosshair_rel_x.matches( devindex, event.code ) ) 
			m_crosshairShouldBeAtPos.x += relativeAxisSensitivity * event.value;

		if ( crosshair_rel_y.matches( devindex, event.code ) ) 
			m_crosshairShouldBeAtPos.y += relativeAxisSensitivity * event.value;
	}
}

void GameController::getTurningStrengthFromCrosshairPos( GameCamera* camera, float* turnLeft, float* turnRight, float* pitch, Vector2* normalizedCrosshair )
{
	// translate position to turning and tilting controls
	float turnStrengthLeft = camera->turnStrength(0);
	float turnStrengthRight = camera->turnStrength(1);
	float turnThresholdLeft = camera->turnThreshold(0);
	float turnThresholdRight = camera->turnThreshold(1);
	float tiltThresholdUp = camera->tiltThreshold(0);
	float tiltThresholdDown = camera->tiltThreshold(1);

	if ( camera->horizontalLimit(0) - turnThresholdLeft == 0 || camera->verticalLimit(0) - tiltThresholdUp == 0 ||
		 camera->horizontalLimit(1) - turnThresholdRight == 0 || camera->verticalLimit(1) - tiltThresholdDown == 0 )
		throw Exception( Format("Targeting zone limit & threshold can not be equal to eachother, it will cause a divide by zero. Use a small difference. (0.000001 is enough)") );

	float turnScaleLeft = ( 1.f / ( camera->horizontalLimit(0) - turnThresholdLeft ) ) * turnStrengthLeft;
	float tiltScaleUp	= ( 1.f / ( camera->verticalLimit(0) - tiltThresholdUp ) );
	float turnScaleRight= ( 1.f / ( camera->horizontalLimit(1) - turnThresholdRight ) ) * turnStrengthRight;
	float tiltScaleDown = ( 1.f / ( camera->verticalLimit(1) - tiltThresholdDown ) );

	// set normalized & clamped crosshair pos
	Vector2 chNormalized( m_crosshairPos.x, m_crosshairPos.y );

	// Down / Up
	if ( m_crosshairPos.y > tiltThresholdDown )
	{
		float down = ( m_crosshairPos.y - tiltThresholdDown ) * tiltScaleDown;
		*pitch = down;
	}
	else if ( m_crosshairPos.y < -tiltThresholdUp )
	{
		float up = -( -m_crosshairPos.y - tiltThresholdUp ) * tiltScaleUp;
		*pitch = up;
	}
	else
	{
		*pitch = 0.f;
	}

	if ( m_crosshairPos.y < 0.f )
	{
		chNormalized.y /= camera->verticalLimit(0);
	}
	else
	{
		chNormalized.y /= camera->verticalLimit(1);
	}

	// turning exponentiality
	float turnExp = 1.f;
	turnExp = m_cfg->getFloat( "Control.PS2.Turning.OneOverXExponentialFuncWindowSize" );

	// Right / Left
	*turnLeft = *turnRight = 0.f;
	if ( m_crosshairPos.x > turnThresholdRight )
	{
		*turnRight = exponentializeOneOverX( ( m_crosshairPos.x - turnThresholdRight ) * turnScaleRight, -turnExp );
		*turnLeft = 0.f;
		chNormalized.x = 1.f;
	}
	else if ( m_crosshairPos.x < -turnThresholdLeft )
	{
		*turnRight = 0.f;
		*turnLeft = exponentializeOneOverX( ( -m_crosshairPos.x - turnThresholdLeft ) * turnScaleLeft, -turnExp );
		chNormalized.x = -1.f;
	}
	else
	{
		*turnRight = 0.f;
		*turnLeft = 0.f;

		if ( m_crosshairPos.x < 0.f )
			chNormalized.x /= turnThresholdLeft;
		else
			chNormalized.x /= turnThresholdRight;
	}

	if ( normalizedCrosshair )
		*normalizedCrosshair = chNormalized;
}

void GameController::applyCrosshairPos( GameCharacter* character, GameCamera* camera, const Vector3& /*crosshairPosDelta*/, float dt ) 
{
	if ( m_setHasJoystickControls )
	{
		// get turning strength from crosshair position
		float turnRight, turnLeft, pitch;
		Vector2 normalizedCrosshair;
		getTurningStrengthFromCrosshairPos( camera, &turnLeft, &turnRight, &pitch, &normalizedCrosshair );
		character->userControl()->turnLeft( turnLeft );
		character->userControl()->turnRight( turnRight ); 
		character->setCrosshair( normalizedCrosshair );
		camera->setPitch( pitch );
	}
	else
	{
		if ( character->primaryState() != GameCharacter::PRIMARY_STATE_PEEKING_LEFT &&
			character->primaryState() != GameCharacter::PRIMARY_STATE_PEEKING_RIGHT )
		{
			Vector3 turnDelta = m_crosshairShouldBeAtPos;
			for ( int i = 0 ; i < 3 ; ++i )
				turnDelta[i] = Math::max( -1.f, Math::min(1.f, turnDelta[i]) );

			float turnLeft=0.f, turnRight=0.f;
			if ( turnDelta.x < 0.f )
				turnLeft = -turnDelta.x * m_cfg->getFloat("Control.PC.MouseTurnSensitivity");
			if ( turnDelta.x > 0.f )
				turnRight = turnDelta.x * m_cfg->getFloat("Control.PC.MouseTurnSensitivity");
			float maxTurnRate = m_cfg->getFloat("Control.PC.MouseMaxCharacterTurnRate");
			if ( turnLeft > maxTurnRate )
				turnLeft = maxTurnRate;
			if ( turnRight > maxTurnRate )
				turnRight = maxTurnRate;
			character->userControl()->turnLeft( turnLeft );
			character->userControl()->turnRight( turnRight ); 

			float pitch = camera->pitch();
			pitch += turnDelta.y * m_cfg->getFloat("Control.PC.MousePitchSensitivity");
			pitch = Math::min( Math::max(-1.f,pitch), 1.f );
			camera->setPitch( pitch );

			m_crosshairPos = m_crosshairShouldBeAtPos = Vector3(0,0,0);
		}
		else
		{
			// get turning strength from crosshair position
			float turnRight, turnLeft, pitch;
			Vector2 normalizedCrosshair;
			getTurningStrengthFromCrosshairPos( camera, &turnLeft, &turnRight, &pitch, &normalizedCrosshair );
			float newPitch = pitch;
			float pitchSpeed = m_cfg->getFloat("Control.PC.MousePeekPitchSpeed"); //2.f;
			if ( newPitch > camera->pitch() )
			{
				camera->setPitch( camera->pitch() + dt * pitchSpeed );
				if ( !(newPitch > camera->pitch()) )
					camera->setPitch( pitch );
			}
			else if ( newPitch < camera->pitch() )
			{
				camera->setPitch( camera->pitch() - dt * pitchSpeed );
				if ( !(newPitch < camera->pitch()) )
					camera->setPitch( pitch );
			}
		}
	}

	// Compute crosshair trace delta
	Camera* cam = camera->getRenderCamera();

	float dx = ( Math::tan( cam->horizontalFov() / 2.f ) * cam->front() * m_crosshairPos.x );
	float dy = ( Math::tan( cam->verticalFov() / 2.f ) * cam->front() * -m_crosshairPos.y );

	Matrix4x4 camtm = cam->worldTransform();
	Matrix3x3 camrot = camtm.rotation();
	Vector3 traceDelta = (camrot.getColumn(0) * dx +
						  camrot.getColumn(1) * dy +
						  camrot.getColumn(2) * cam->front() );
	
	const float TRACE_DISTANCE = 75.f; // Trace x meters
	if ( traceDelta.length() > Float::MIN_VALUE )
		traceDelta = traceDelta.normalize() * TRACE_DISTANCE;
	else
		traceDelta = character->forward() * TRACE_DISTANCE;

	// Compute crosshair trace start
	// (make sure we don't aim behind character)
	Vector3 traceStart = camera->position();
	Vector3 n = -character->forward();
	Vector3 p0 = character->position() + character->up()*1.5f - n*2.f;
	Vector4 plane( n.x, n.y, n.z, -p0.dot(n) );
	float t;
	if ( Intersection::findRayPlaneIntersection( traceStart, traceDelta, plane, &t ) )
		traceStart += traceDelta * t;

	// Collide camera with scene, retrieve end point
	m_collisionTester->setPosition( camera->cell(), camera->position() );
	m_collisionTester->moveWithoutColliding( traceStart - m_collisionTester->position() );
	m_collisionTester->move( traceDelta, &m_aimCollisionInfo );
	character->aimAt( m_collisionTester->position() );
	character->lookTo( m_collisionTester->position() );
}


void GameController::addSetAndAction( const lang::String& set, const lang::String& action, const lang::String& device, unsigned int code ) 
{
	// Get device index for device name
	int deviceindex = -1;

	for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
	{
		id::InputDevice* dev = m_inputDriver->getAttachedInputDevice( i );
		String name = dev->name();
		if (device == name)
			deviceindex = i;
	}

	if (deviceindex == -1)
	{
		Debug::printlnError("Unrecognized device in controller properties");
		return;
	}

	// Check for control set already exists (See TODO in GameController.h, needs to be hash table)
	int ctrlset = -1;

	for ( int i = 0; i < m_controlSets.size(); ++i )
		if ( m_controlSets[i].name() == set )
			ctrlset = i;

	if ( ctrlset == -1 )
	{
		m_controlSets.add( ControlSet( m_inputDriver, m_controllerCfg ) );
		ctrlset = m_controlSets.size() - 1;
		m_controlSets[ctrlset].setName( set );
		m_controlSets[ctrlset].initializeByDevice( deviceindex );
	}

	// Translate action to GameAction
	GameAction::ActionType actiontype = GameAction::toAction( action );	
	
	// Get pre-set action
	GameAction& gameaction = m_controlSets[ctrlset].getAction((int)actiontype);
		
	// Set configured parameters
	gameaction.setDevice( deviceindex );
	gameaction.setEventCode( code );

}

GameAction::ActionType GameController::getActionType( int deviceindex, int eventCode )
{
	for ( int i = 0; i < m_controlSets[m_activeControlSet].actions(); ++i )
	{
		GameAction& gameaction = m_controlSets[m_activeControlSet].getAction(i);

		if ( ( gameaction.device() == deviceindex ) && ( gameaction.eventCode() == eventCode ) )
			return gameaction.action();
	}
	return GameAction::ACTION_COUNT;
}

int	GameController::activeControlSet() const
{
	return m_activeControlSet;
}

int	GameController::controlSets() const
{
	return m_controlSets.size();
}

bool GameController::controllersDirty() const 
{
	return m_controllersDirty;
}

Vector3	GameController::crosshairPos() const 
{
	return m_crosshairPos;
}

const CollisionInfo& GameController::aimCollisionInfo() const
{
	return m_aimCollisionInfo;
}

int GameController::getInputDevice( const lang::String& device ) const 
{
	for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
	{
		if ( String( m_inputDriver->getAttachedInputDevice( i )->name() ) == device )
			return i;
	}

	return -1;
}

float GameController::exponentialize( float val, float exponentiality ) const
{
	if ( exponentiality > 0.f )
	{
		float maxval = -1.f + Math::pow( 2, exponentiality );
		return (-1.f + Math::pow( 2, val * exponentiality )) / maxval;
	}
	else return 0;
}

float GameController::exponentializeOneOverX( float val, float exponentiality ) const
{
	if ( exponentiality < 0.f )
	{
		float maxval = Math::pow( 2, 1.f / (-0.11f + exponentiality) );
		return Math::pow( 2, 1.f / (-0.11f + val * exponentiality) ) / maxval;
	}
	else return 0;
}

void GameController::addCrosshairAveragePos( const Vector2& val ) 
{
	int size = 1;
	if ( m_setHasJoystickControls )
		size = m_cfg->getInteger( "Control.PS2.Crosshair.AverageBufferSize" );

	m_crosshairAverageBuffer.add( val );
	if ( m_crosshairAverageBuffer.size() > size )
		m_crosshairAverageBuffer.remove( 0 );
}

Vector2 GameController::getAverageCrosshairPos() 
{
	Vector2 accumulator(0,0);

	if ( m_crosshairAverageBuffer.size() > 0 )
	{
		for ( int i = 0; i < m_crosshairAverageBuffer.size() ; ++i )
			accumulator += m_crosshairAverageBuffer[i];

		accumulator = accumulator * ( 1.f / m_crosshairAverageBuffer.size() );
	}
	
	return accumulator;
}

String GameController::activeCameraName() const
{
	if ( m_setHasJoystickControls )
	{
		if ( !m_cfg->containsKey("Control.PS2.ActiveCamera") )
			throw Exception( Format("Prop needs Control.PS2.ActiveCamera to be set") );
		return m_cfg->get( "Control.PS2.ActiveCamera" );
	}
	else
	{
		if ( !m_cfg->containsKey("Control.PC.ActiveCamera") )
			throw Exception( Format("Prop needs Control.PC.ActiveCamera to be set") );
		return m_cfg->get( "Control.PC.ActiveCamera" );
	}
	return "";
}

int GameController::script_hasJoystickControls( script::VM* vm, const char* funcName )
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns true if controller configuration uses joystick", funcName) );

	vm->pushBoolean( m_setHasJoystickControls );
	return 1;
}
