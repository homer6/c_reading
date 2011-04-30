#include "ControlSet.h"
#include <util/ExProperties.h>
#include <id/InputDriver.h>
#include <id/InputDevice.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace id;
using namespace util;
using namespace lang;

//-----------------------------------------------------------------------------

ControlSet::ControlSet( id::InputDriver* inputDriver, ExProperties* cfg ) :
	m_inputDriver(inputDriver),
	m_cfg(cfg),
	m_actions( Allocator<GameAction>(__FILE__) )
{
	m_actions.setSize( GameAction::ACTION_COUNT );
}

ControlSet::ControlSet() :
	m_inputDriver ( 0 ),
	m_cfg( 0 ),
	m_actions( Allocator<GameAction>(__FILE__) )
{
	m_actions.setSize( GameAction::ACTION_COUNT );
}

ControlSet::~ControlSet() 
{
}

void ControlSet::initializeByDevice( int index )
{
	InputDevice* dev = m_inputDriver->getAttachedInputDevice( index );

	/* Find keyboard controller for mouse. */
	int keyboard = -1;
	for ( int i = 0; i < m_inputDriver->attachedInputDevices(); ++i )
		if ( m_inputDriver->getAttachedInputDevice(i)->deviceType() == InputDevice::TYPE_KEYBOARD )
			keyboard = i;

	if ( keyboard == -1 )
	{
		Debug::printlnError( Format("Unable to find keyboard!").format() );
		keyboard = 0;
	}

	/* Set Name */
	m_name = dev->name();
	
	/* Create GameActions */		
	m_actions[ GameAction::ATTACK ]				= GameAction( GameAction::ATTACK, index, 0, true );
	m_actions[ GameAction::ACCELERATE ]			= GameAction( GameAction::ACCELERATE, index, 0, true );
	m_actions[ GameAction::BACKWARD ]			= GameAction( GameAction::BACKWARD, index, 0, true );
	m_actions[ GameAction::STRAFERIGHT ]		= GameAction( GameAction::STRAFERIGHT, index, 0, true );
	m_actions[ GameAction::STRAFELEFT ]			= GameAction( GameAction::STRAFELEFT, index, 0, true );
	m_actions[ GameAction::ROLL ]				= GameAction( GameAction::ROLL, index, 0, true );
	m_actions[ GameAction::CROUCH ]				= GameAction( GameAction::CROUCH, index, 0, true );
	m_actions[ GameAction::CROSSHAIR_REL_X]		= GameAction( GameAction::CROSSHAIR_REL_X, index, 0, true );
	m_actions[ GameAction::CROSSHAIR_REL_Y]		= GameAction( GameAction::CROSSHAIR_REL_Y, index, 0, true );
	m_actions[ GameAction::CROSSHAIR_ABS_RIGHT]	= GameAction( GameAction::CROSSHAIR_ABS_RIGHT, index, 0, true );
	m_actions[ GameAction::CROSSHAIR_ABS_LEFT]	= GameAction( GameAction::CROSSHAIR_ABS_LEFT, index, 0, true );
	m_actions[ GameAction::CROSSHAIR_ABS_DOWN]	= GameAction( GameAction::CROSSHAIR_ABS_DOWN, index, 0, true );
	m_actions[ GameAction::CROSSHAIR_ABS_UP]	= GameAction( GameAction::CROSSHAIR_ABS_UP, index, 0, true );
	m_actions[ GameAction::ATTACK_STRIKE]		= GameAction( GameAction::ATTACK_STRIKE, index, 0, true );
	m_actions[ GameAction::ATTACK_KICK]			= GameAction( GameAction::ATTACK_KICK, index, 0, true );
	m_actions[ GameAction::PEEKRIGHT ]			= GameAction( GameAction::PEEKRIGHT, index, 0, true );
	m_actions[ GameAction::PEEKLEFT ]			= GameAction( GameAction::PEEKLEFT, index, 0, true );
	m_actions[ GameAction::CHANGECLIP ]			= GameAction( GameAction::CHANGECLIP, index, 0, true );
	m_actions[ GameAction::CYCLEWEAPON ]		= GameAction( GameAction::CYCLEWEAPON, index, 0, true );
	m_actions[ GameAction::THROWEMPTYSHELL ]	= GameAction( GameAction::THROWEMPTYSHELL, index, 0, true );
	m_actions[ GameAction::TOGGLE_RUN ]			= GameAction( GameAction::TOGGLE_RUN, index, 0, true );
	m_actions[ GameAction::MODIFIER_SNEAK ]		= GameAction( GameAction::MODIFIER_SNEAK, index, 0, true );

	/* Initialize default controls */
	InputDevice::DeviceType type = dev->deviceType();

	switch ( type )
	{
	case InputDevice::TYPE_UNSUPPORTED:
		break;
	case InputDevice::TYPE_MOUSE:
		m_actions[ GameAction::ACCELERATE ].setDevice( keyboard );
		m_actions[ GameAction::BACKWARD ].setDevice( keyboard );
		m_actions[ GameAction::STRAFERIGHT ].setDevice( keyboard );
		m_actions[ GameAction::STRAFELEFT ].setDevice( keyboard );
		m_actions[ GameAction::ROLL ].setDevice( keyboard );
		m_actions[ GameAction::CROUCH ].setDevice( keyboard );
		m_actions[ GameAction::CROSSHAIR_ABS_RIGHT ].setDevice( keyboard );
		m_actions[ GameAction::CROSSHAIR_ABS_LEFT ].setDevice( keyboard );
		m_actions[ GameAction::CROSSHAIR_ABS_DOWN ].setDevice( keyboard );
		m_actions[ GameAction::CROSSHAIR_ABS_UP ].setDevice( keyboard );
		m_actions[ GameAction::PEEKRIGHT ].setDevice( keyboard );
		m_actions[ GameAction::PEEKLEFT ].setDevice( keyboard );
		m_actions[ GameAction::CHANGECLIP ].setDevice( keyboard );
		m_actions[ GameAction::CYCLEWEAPON ].setDevice( keyboard );
		m_actions[ GameAction::THROWEMPTYSHELL ].setDevice( keyboard );
		m_actions[ GameAction::ATTACK_KICK ].setDevice( keyboard );
		m_actions[ GameAction::TOGGLE_RUN ].setDevice( keyboard );
		m_actions[ GameAction::MODIFIER_SNEAK ].setDevice( keyboard );

		m_actions[ GameAction::ACCELERATE ].setEventCode		( 0x110000 );	// W
		m_actions[ GameAction::BACKWARD ].setEventCode			( 0x1F0000 );	// S
		m_actions[ GameAction::STRAFERIGHT ].setEventCode		( 0x200000 );	// D
		m_actions[ GameAction::STRAFELEFT ].setEventCode		( 0x1E0000 );	// A
		m_actions[ GameAction::ROLL ].setEventCode				( 0x2D0000 );	// X
		m_actions[ GameAction::CROUCH ].setEventCode			( 0x2E0000 );	// C			(47)
		m_actions[ GameAction::CROSSHAIR_ABS_RIGHT ].setEventCode	( 0 );
		m_actions[ GameAction::CROSSHAIR_ABS_LEFT ].setEventCode	( 0 );
		m_actions[ GameAction::CROSSHAIR_ABS_DOWN ].setEventCode	( 0 );
		m_actions[ GameAction::CROSSHAIR_ABS_UP ].setEventCode		( 0 );
		m_actions[ GameAction::PEEKRIGHT ].setEventCode			( 0x120000 );	// E
		m_actions[ GameAction::PEEKLEFT ].setEventCode			( 0x100000 );	// Q
		m_actions[ GameAction::CHANGECLIP ].setEventCode		( 0x130000 );	// R
		m_actions[ GameAction::CYCLEWEAPON ].setEventCode		( 0x0F0000 );	// Tab
		m_actions[ GameAction::THROWEMPTYSHELL ].setEventCode	( 0x2C0000 );	// Z
		m_actions[ GameAction::TOGGLE_RUN ].setEventCode		( 0x3A0000 );	// Caps Lock
		m_actions[ GameAction::MODIFIER_SNEAK ].setEventCode	( 0x2A0000 );	// Left Shift

		m_actions[ GameAction::ATTACK ].setEventCode			( 0x0400 );		// Left Mouse Button
		m_actions[ GameAction::ATTACK_STRIKE ].setEventCode		( 0x0500 );		// Middle Mouse Button
		m_actions[ GameAction::ATTACK_KICK ].setEventCode		( 0x1D0000 );	// Left Control
		m_actions[ GameAction::CROSSHAIR_REL_X ].setEventCode	( 0x10000000 );	// Relative Axis 1	( X Axis )
		m_actions[ GameAction::CROSSHAIR_REL_Y ].setEventCode	( 0x20000000 );	// Relative Axis 2	( Y Axis )
		break;
	case InputDevice::TYPE_JOYSTICK:
		m_actions[ GameAction::ATTACK ].setEventCode				( 0x0600 );		// R2
		m_actions[ GameAction::ATTACK_STRIKE ].setEventCode			( 0x0400 );		// Square
		m_actions[ GameAction::ATTACK_KICK ].setEventCode			( 0x0300 );		// X
		m_actions[ GameAction::ACCELERATE ].setEventCode			( 0x05 );		// Left Joystick
		m_actions[ GameAction::BACKWARD ].setEventCode				( 0x06 );	
		m_actions[ GameAction::STRAFERIGHT ].setEventCode			( 0x08 );	
		m_actions[ GameAction::STRAFELEFT ].setEventCode			( 0x07 );	
		m_actions[ GameAction::ROLL ].setEventCode					( 0x0700 );		// L1
		m_actions[ GameAction::CROUCH ].setEventCode				( 0x0500 );		// L2
		m_actions[ GameAction::CROSSHAIR_ABS_RIGHT ].setEventCode	( 0x04 );		// Right Joystick
		m_actions[ GameAction::CROSSHAIR_ABS_LEFT ].setEventCode	( 0x03 );		
		m_actions[ GameAction::CROSSHAIR_ABS_DOWN ].setEventCode	( 0x02 );	
		m_actions[ GameAction::CROSSHAIR_ABS_UP ].setEventCode		( 0x01 );	
		m_actions[ GameAction::PEEKRIGHT ].setEventCode				( 0x0e00 );		// Digital Right
		m_actions[ GameAction::PEEKLEFT ].setEventCode				( 0x1000 );		// Digital Left
		m_actions[ GameAction::CHANGECLIP ].setEventCode			( 0x0100 );		// Triangle
		m_actions[ GameAction::CYCLEWEAPON ].setEventCode			( 0x0200 );		// Ball
		m_actions[ GameAction::THROWEMPTYSHELL ].setEventCode		( 0x0800 );		// R1
		break;
	case InputDevice::TYPE_KEYBOARD:  // NOT SUPPORTED
	case InputDevice::TYPE_GAMEPAD:
	case InputDevice::TYPE_FLIGHT:
		break;
	}
}

int ControlSet::actions() const
{
	return m_actions.size();
}

int	ControlSet::configurableActions() const
{
	int c = 0;

	for ( int i = 0; i < m_actions.size(); i++ )
	{
		if ( m_actions[i].isConfigurable() ) 
			c++;
	}
	return c;
}

int	ControlSet::configurableActionIndexIs( int index ) const
{
	int c = 0;

	for ( int i = 0; i < m_actions.size(); i++ )
	{
		if ( m_actions[i].isConfigurable() ) 
		{
			if ( c == index ) return i;
			c++;
		}
	}
	throw Exception( Format("Action index out of range") );
}

const GameAction& ControlSet::getAction( int index ) const
{
	if ( index > m_actions.size() )
		throw Exception( Format("Action index out of range") );

	return m_actions[index];
}

GameAction& ControlSet::getAction( int index ) 
{
	if ( index > m_actions.size() )
		throw Exception( Format("Action index out of range") );

	return m_actions[index];
}

GameAction& ControlSet::getConfigurableAction( int index )
{
	int c = 0;

	for ( int i = 0; i < m_actions.size(); i++ )
	{
		if ( m_actions[i].isConfigurable() ) 
		{
			if ( c == index ) return m_actions[i];
			c++;
		}
	}

	throw Exception( Format("Action index out of range") );
}

void ControlSet::addAction( const GameAction& val )
{
	m_actions.add( val );
}

void ControlSet::setName( const lang::String& name ) 
{
	m_name = name;
}

const String& ControlSet::name() const
{
	return m_name;
}

const bool ControlSet::hasJoystickControls() const 
{
	bool has = false;
	
	for ( int i = 0; i < m_actions.size(); ++i )
	{
		if ( m_inputDriver->getAttachedInputDevice( m_actions[i].device() )->deviceType() == InputDevice::TYPE_JOYSTICK )
			has = true;
	}

	return has;
}

