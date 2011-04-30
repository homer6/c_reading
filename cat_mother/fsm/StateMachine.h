#ifndef _FSM_STATEMACHINE_H
#define _FSM_STATEMACHINE_H


#include "ScriptMethod.h"
#include <util/Vector.h>
#include <script/VM.h>
#include <script/Table.h>
#include <script/Scriptable.h>
#include <lang/Object.h>


namespace fsm
{


/** 
 * Finite state machine.
 * State machine consists of vector of states.
 *
 * Each state is a Lua table with following properties:
 * state = {}
 * state.enterCondition = function(this) ... end
 * state.enter = function(this) ... end
 * state.updateInterval = <seconds>
 * state.update = function(this) ... end
 * state.exitCondition = function(this) ... end
 * state.exit = function(this) ... end
 *
 * On each update enterCondition of next state on the vector
 * is checked. If we assume that state number 3 is active:
 * If enterCondition of state number 4 becomes true
 * then state 4 is activated. If exitCondition
 * of state 3 becomes valid then highest state below
 * is selected which has enterCondition true.
 *
 * Example AI script init:
 *
 * statemachine:removeStates()
 * statemachine:addState( this:patrolRoute( "PATH_Guard1" ) )
 * statemachine:addState( this:checkIfNoticed( hero ) )
 * statemachine:addState( this:turnTo( hero ) )
 * statemachine:addState( this:crouchAndShoot( hero ) )
 * statemachine:addState( this:lowOnHealth() )
 * statemachine:addState( this:duckAndCover() )
 *
 * Example script execution sequence:
 * AI patrols until checkIfNoticed enterCondition is true.
 * turnTo has enter condition that target is in known position
 * so it becomes valid too. crouchAndShoot has enter condition
 * that checks that target is in appropriate direction,
 * so turnTo remains active state until AI is facing towards
 * player. Then crouchAndShoot becomes active; AI crouches
 * and then starts shooting. Then AI loses sight on player;
 * crouchAndShoot exitCondition becomes true.
 * Now highest valid state is searched; turnTo enterCondition
 * is false because hero is not in known position. 
 * Same thing with checkIfNoticed. patrolRoute has no enterCondition
 * so AI returns to patrolling state.
 *
 * Optional sequence of events:
 *
 * Everything happens as before AI loses player. 
 * When AI loses player and crouchAndShoot exitCondition becomes true,
 * crouchAndShoot exit function sees that hero is still alive and
 * decides to follow the player at all cost: it removes all states 
 * from state machine ands adds following states:
 *
 * statemachine:removeStates()
 * statemachine:addState( this:standAlert() )
 * statemachine:addState( this:runToLastKnownPosition( hero ) )
 * statemachine:addState( this:checkIfVisible( hero ) )
 * statemachine:addState( this:shootAndRunTowards( hero ) ) 
 *
 * Action States:
 *
 * Action states are executed only in forward transitions and block until their 
 * exitCondition is true. After exitCondition==true the state is removed from the stack.
 *
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class StateMachine :
	public script::Scriptable
{
public:
	explicit StateMachine( script::VM* vm );
	
	~StateMachine();

	/** Push new state to state stack. */
	void addState( int ref );

	/** Updates the machine. */
	void update( float dt );

	/** Returns current state name. */
	lang::String	state();

	/** Returns current state table. */
	script::Table	stateTable();

	/** Returns true if there is a next state. */
	bool	nextState() const;

	/** Returns number of states. */
	int		states() const;

	/** Returns next state table. */
	script::Table	nextStateTable();

private:
	int									m_methodBase;
	static	ScriptMethod<StateMachine>	sm_methods[];
	P(script::VM)						m_vm;

	// Internal state
	float				m_time;
	float				m_updateTime;
	int					m_state;
	util::Vector<int>	m_stateStack;
	bool				m_machineReset;
	
	// Helper functions
	script::Table		table( int );
	void				pushMemberMethod( script::Table, const lang::String& name );
	bool				callBool( script::Table, const lang::String& name );

	// Script methods
	int		methodCall( script::VM* vm, int i );
	int		script_addState( script::VM* vm, const char* funcName );
	int		script_getState( script::VM* vm, const char* funcName );
	int		script_removeStates( script::VM* vm, const char* funcName );

	StateMachine();
	StateMachine( const StateMachine& other );
	StateMachine& operator=( const StateMachine& other );
};


} // fsm


#endif
