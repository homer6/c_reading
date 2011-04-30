#include "StateMachine.h"
#include "ScriptUtil.h"
#include <lang/Debug.h>
#include <script/ScriptException.h>
#include "config.h"

//---------------------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace script;

//---------------------------------------------------------------------------------------

namespace fsm
{


ScriptMethod<StateMachine> StateMachine::sm_methods[] =
{
	ScriptMethod<StateMachine>( "addState", script_addState ),
	ScriptMethod<StateMachine>( "getState", script_getState ),
	ScriptMethod<StateMachine>( "removeStates", script_removeStates ),
};

//-----------------------------------------------------------------------------

StateMachine::StateMachine( script::VM* vm ) :
	Scriptable( vm, vm->newTag() ),
	m_vm( vm ),
	m_methodBase( -1 ),
	m_time( 0 ),
	m_updateTime( 0 ),
	m_state( 0 ),
	m_stateStack( Allocator<int>(__FILE__) ),
	m_machineReset( true )
{
	m_methodBase = ScriptUtil<StateMachine,Scriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}
	
StateMachine::~StateMachine()
{
	for ( int i = 0; i < m_stateStack.size(); ++i )
	{
		vm()->unref( m_stateStack[i] );
	}
	
	m_stateStack.setSize(0);
	m_state = 0;
}

void StateMachine::addState( int ref ) 
{
	m_stateStack.add( ref );
}

void StateMachine::update( float dt )
{
	assert( m_stateStack.size() > 0 );
	if ( 0 == m_stateStack.size() )
		return;

	if ( m_machineReset )
	{
		if ( !table( m_stateStack[ m_state ] ).isNil( "enter" ) )
		{
			pushMemberMethod( table( m_stateStack[ m_state ]  ), "enter" );
			m_vm->call(1,0);
		}
		m_machineReset = false;
	}

	m_time += dt;
	m_updateTime += dt;

	if ( m_updateTime > table( m_stateStack[ m_state ] ).getNumber( "updateInterval" ) )
	{
		// evaluate trough stack until enter condition is not satisfied, active state "deepest" succesful enter condition
		int newState = -1;
		
		if ( m_stateStack.size() - 1 > m_state && callBool( table( m_stateStack[ m_state + 1 ] ), "enterCondition" ) )
		{
			newState = m_state + 1;
		}

		if ( newState == -1 &&  callBool( table( m_stateStack[ m_state ] ), "exitCondition" ) )
		{
			newState = m_state - 1;
		}

		// switch to new state
		if ( newState != -1 && newState != m_state )
		{
			if ( m_state != -1 && !table( m_stateStack[ m_state] ).isNil( "exit" ) && newState < m_state )
			{
				pushMemberMethod( table( m_stateStack[ m_state ] ), "exit" );
				m_vm->call(1,0);
			}

			if ( !table( m_stateStack[ newState ] ).isNil( "enter" ) && newState > m_state )
			{
				pushMemberMethod( table( m_stateStack[ newState ]  ), "enter" );
				m_vm->call(1,0);
			}
			
			m_state = newState;
		}
	
		if ( !table( m_stateStack[ m_state] ).isNil( "update" ) )
		{
			pushMemberMethod( table( m_stateStack[ m_state ] ), "update" );
			m_vm->call(1,0);
		}

		m_updateTime = 0.f;
	}
}

Table StateMachine::table( int tablref ) 
{
	m_vm->getRef( tablref );
	Table tab = m_vm->toTable(-1);
	m_vm->pop();

	return tab;
}

void StateMachine::pushMemberMethod( script::Table tab, const lang::String& name ) 
{
	tab.pushMember( name );
	m_vm->pushTable( tab );
}

bool StateMachine::callBool( script::Table tab, const lang::String& name ) 
{
	pushMemberMethod( tab, name );
	m_vm->call( 1, 1 );

	if ( m_vm->top() != 1 )
		throw Exception( Format("{0} should return a boolean", name) );

	bool isTrue = !m_vm->isNil(1);
	m_vm->pop();

	return isTrue;
}

String StateMachine::state()
{
	assert( m_state >= 0 );
	//assert( m_state < m_stateStack.size() );

	if ( m_state < 0 || m_state >= m_stateStack.size() )
		return "";
	return table( m_stateStack[ m_state ] ).getString( "name" );
}

script::Table	StateMachine::stateTable() 
{
	assert( m_state >= 0 );
	assert( m_state < m_stateStack.size() );

	if ( m_state < 0 || m_state >= m_stateStack.size() )
		throw ScriptException( Format("StateMachine::stateTable cannot be called without defining state stack first") );
	return table( m_stateStack[ m_state ] );
}

int	StateMachine::states() const
{
	return m_stateStack.size();
}

bool StateMachine::nextState() const 
{
	assert( m_state >= 0 );

	if ( m_stateStack.size() - 1 > m_state )
		return true;
	else
		return false;
}

script::Table	StateMachine::nextStateTable() 
{
	assert( m_state >= 0 );
	assert( m_state + 1 < m_stateStack.size() );

	return table( m_stateStack[ m_state + 1 ] );	
}

int StateMachine::methodCall( script::VM* vm, int i ) 
{
	return ScriptUtil<StateMachine,Scriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );	
}

int StateMachine::script_removeStates( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format( "{0} clears state machine, expects no parameters.", funcName ) );

	for ( int i = 0; i < m_stateStack.size(); ++i )
	{
		vm->unref( m_stateStack[i] );
	}
	
	m_stateStack.setSize(0);
	m_state = 0;
	m_machineReset = true;
	m_updateTime = 0;
	return 0;
}

int StateMachine::script_addState( script::VM* vm, const char* funcName ) 
{
	if ( vm->top() != 1 )
		throw ScriptException( Format("{0} adds a new state to stack, expects table", funcName ) );

	// Check state validity
	Table state = vm->toTable(1);
	if ( state.isNil( "updateInterval" ) || state.isNil( "enterCondition" ) || state.isNil( "exitCondition" ) )
		throw ScriptException( Format("{0} does not accept states without valid \"updateInterval\", \"enterCondition\" or \"exitCondition\" members", funcName ) );
	
	int ref = vm->ref( true );
	addState( ref );
	return 0;
}

int StateMachine::script_getState( script::VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_STRING};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects name of state", funcName) );

	String name = vm->toString(1);
	int match = -1;

	for ( int i = 0; i < m_stateStack.size(); ++i )
	{
		if ( name == table( m_stateStack[i] ).getString( "name" ) )
		{
			match = i;
		}
	}

	if ( match == -1 )
		throw ScriptException( Format("{0} can not find state {1}", funcName, name ) );
	
	vm->pushTable( table( m_stateStack[match] ) );
	return 1;
}


} // fsm
