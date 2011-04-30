#include "GameAction.h"
#include <lang/Exception.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

static const String s_actionDescription[] =
{
	"jump",
	"attack",
	"accelerate",
	"backward",
	"strafe_right",
	"strafe_left",
	"roll",
	"crouch",
	"crosshair_x_relative",
	"crosshair_y_relative",
	"crosshair_right_absolute",
	"crosshair_left_absolute",
	"crosshair_down_absolute",
	"crosshair_up_absolute",
	"attack_strike",
	"attack_kick",
	"peek_left",
	"peek_right",
	"change_clip",
	"cycle_weapon",
	"throw_empty_shell",
	"toggle_run",
	"modifier_sneak",
};

//-----------------------------------------------------------------------------

GameAction::GameAction()
{
	m_action	= ATTACK; // Default action
	m_device	= 0;
	m_eventCode	= 0;
}

GameAction::GameAction( ActionType action, int deviceIndex, int eventCode, bool configurable ) :
	m_action(action),
	m_device(deviceIndex),
	m_eventCode(eventCode),
	m_isConfigurable(configurable)
{
	assert( s_actionDescription[JUMP] == "jump" );
	assert( s_actionDescription[ATTACK] == "attack" );
	assert( s_actionDescription[BACKWARD] == "backward" );
	assert( s_actionDescription[STRAFERIGHT] == "strafe_right" );
	assert( s_actionDescription[STRAFELEFT] == "strafe_left" );	
	assert( s_actionDescription[ROLL] == "roll" );	
	assert( s_actionDescription[CROUCH] == "crouch" );
	assert( s_actionDescription[CROSSHAIR_REL_X] == "crosshair_x_relative" );
	assert( s_actionDescription[CROSSHAIR_REL_Y] == "crosshair_y_relative" );
	assert( s_actionDescription[CROSSHAIR_ABS_RIGHT] == "crosshair_right_absolute" );
	assert( s_actionDescription[CROSSHAIR_ABS_LEFT] == "crosshair_left_absolute" );
	assert( s_actionDescription[CROSSHAIR_ABS_DOWN] == "crosshair_down_absolute" );
	assert( s_actionDescription[CROSSHAIR_ABS_UP] == "crosshair_up_absolute" );
	assert( s_actionDescription[ATTACK_STRIKE] == "attack_strike" );
	assert( s_actionDescription[ATTACK_KICK] == "attack_kick" );
	assert( s_actionDescription[PEEKRIGHT] == "peek_right" );
	assert( s_actionDescription[PEEKLEFT] == "peek_left" );	
	assert( s_actionDescription[CHANGECLIP] == "change_clip" );	
	assert( s_actionDescription[CYCLEWEAPON] == "cycle_weapon" );	
	assert( s_actionDescription[THROWEMPTYSHELL] == "throw_empty_shell" );	
	assert( s_actionDescription[TOGGLE_RUN] == "toggle_run" );
	assert( s_actionDescription[MODIFIER_SNEAK] == "modifier_sneak" );
	assert( sizeof(s_actionDescription)/sizeof(s_actionDescription[0]) == ACTION_COUNT );
}

GameAction::~GameAction()
{
}

void GameAction::setAction( ActionType action )
{
	m_action = action;
}


void GameAction::setDevice( int deviceIndex )
{
	m_device = deviceIndex;
}


void GameAction::setEventCode( int eventCode )
{
	m_eventCode = eventCode;
}

GameAction::ActionType GameAction::action() const
{
	return m_action;
}

int	GameAction::device() const
{
	return m_device;
}

int GameAction::eventCode() const
{
	return m_eventCode;
}	

bool GameAction::isConfigurable() const
{
	return m_isConfigurable;
}

bool GameAction::matches( int devIndex, int eventCode ) const
{
	if ( m_device == devIndex && m_eventCode == eventCode )
		return true;
	else
		return false;
}

const String& GameAction::toString( ActionType action )
{
	return s_actionDescription[action];
}

GameAction::ActionType	GameAction::toAction( const String& str )
{
	for ( int i = 0; i < ACTION_COUNT; ++i )
		if ( str == s_actionDescription[i] )
			return (ActionType)i;
	
	throw Exception( Format("Unknown controller action: {0}", str) );
	return ATTACK;
}

int GameAction::possibleActions()
{
	return ACTION_COUNT;
}
