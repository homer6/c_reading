#ifndef _GAMEACTION_H
#define _GAMEACTION_H


#include <lang/String.h>


/**
 * @author Toni Aittoniemi
 */
class GameAction
{
public:
	enum ActionType
	{
		JUMP,
		ATTACK,
		ACCELERATE,
		BACKWARD,
		STRAFERIGHT,
		STRAFELEFT,
		ROLL,
		CROUCH,
		CROSSHAIR_REL_X,
		CROSSHAIR_REL_Y,
		CROSSHAIR_ABS_RIGHT,
		CROSSHAIR_ABS_LEFT,
		CROSSHAIR_ABS_DOWN,
		CROSSHAIR_ABS_UP,
		ATTACK_STRIKE,
		ATTACK_KICK,
		PEEKLEFT,
		PEEKRIGHT,
		CHANGECLIP,
		CYCLEWEAPON,
		THROWEMPTYSHELL,
		TOGGLE_RUN,
		MODIFIER_SNEAK,
		ACTION_COUNT
	};

	GameAction( ActionType action, int deviceIndex, int eventCode, bool configurable );
	GameAction();
	~GameAction();
	
	void		setAction( ActionType action );
	void		setDevice( int deviceIndex );
	void		setEventCode( int eventCode );

	ActionType	action() const;
	int			device() const;
	int			eventCode() const;
	bool		isConfigurable() const;
	bool		matches( int devIndex, int eventCode ) const;

	// Script methods :
	//	Get controller sets
	//  
	
	static const lang::String&	toString( ActionType action );
	static ActionType			toAction( const lang::String& str );
	static int					possibleActions();

private:
	ActionType	m_action;
	int			m_device;
	int			m_eventCode;
	bool		m_isConfigurable;
};


#endif // _GAMEACTION_H
