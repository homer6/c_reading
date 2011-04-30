// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <lang/Debug.h>
#include <fsm/StateMachine.h>
#include <script/VM.h>

using namespace fsm;
using namespace lang;
using namespace script;

int main(int argc, char* argv[])
{
	printf("Testing Finite State Machine library. (c) Cat Mother Ltd. 2003\n");

	P(VM) vm = new VM;
	StateMachine statemachine( vm );
	statemachine.compileFile( "test.lua" );
	statemachine.pushMethod( "init" );
	statemachine.call(0,0);

	printf("Calling update with dt = 0.01\n");
	bool quit = false;
	while ( !quit )
	{
		statemachine.update(0.01f);

		if ( _kbhit() )
		{
			int c = _getch();
			printf( "Key %d ", c ); 
			switch( c )
			{
			case 27 : // escape
				quit = true;
				break;
			case 113 : // q
				printf( "Setting interrupt state enter condition" );
				statemachine.setNumber( "enterFlag", 1 );
				break;
			case 119 : // w
				printf( "Disabling interrupt state enter condition" );
				statemachine.setNumber( "enterFlag", 0 );
				break;
			case 97 : // q
				printf( "Setting base state exit condition" );
				statemachine.setNumber( "baseExitFlag", 1 );
				break;
			case 115 : // w
				printf( "Disabling base state exit condition" );
				statemachine.setNumber( "baseExitFlag", 0 );
				break;
			case 105 : // i 
				printf( "Re-Initializing state machine" );
				statemachine.pushMethod( "init" );
				statemachine.call(0,0);
				break;
			default:;
			}
		}
	}

	return 0;
}

