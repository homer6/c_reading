#include "Scriptable.h"
#include "StackCheck.h"
#include "RestoreStack.h"
#include "This.h"
#include <lang/Debug.h>
#include <lang/String.h>
#include <lang/Throwable.h>
#include <util/Vector.h>
#include <script/VM.h>
#include <script/ScriptException.h>
#include <lua.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace script
{


/** Member function call dispatcher. */
static int callMethod( lua_State* lua )
{
	assert( lua_isnumber(lua,-1) );		// function closure (methodIndex)

	try
	{
		// check that 'this' table is passed to the function
		int type = lua_type( lua, 1 );
		if ( type != VM::TYPE_TABLE )
			throw ScriptException( Format("Missing 'this' table when calling C++ function from Lua") );

		// get C++ this ptr from Lua 'this' table index 0
		lua_rawgeti( lua, 1, 0 );
		type = lua_type( lua, -1 );
		if ( type != VM::TYPE_USERDATA )
			throw ScriptException( Format("Invalid 'this' table when calling C++ function from Lua") );
		Scriptable* scriptable = (Scriptable*)lua_touserdata( lua, -1 );

		// get index of called method
		int methodIndex = (int)lua_tonumber( lua, -2 );
		assert( methodIndex >= 0 && methodIndex < scriptable->methods() );

		// remove 'this' table,
		lua_remove( lua, 1 );
		lua_pop( lua, 2 );

		// call the function, only arguments in the Lua stack
		return scriptable->methodCall( scriptable->vm(), methodIndex );
	}
	catch ( Throwable& e )
	{
		char msg[512];
		e.getMessage().format().getBytes( msg, sizeof(msg), "UTF-8" );
		lua_error( lua, msg );
		return 0;
	}
}

/** 
 * Gets scriptable C++ functions of a table.
 * @param tableIndex Index of the table in VM stack.
 */
static void getFunctions( VM* vm, Vector<String>& funcNames, int tableIndex )
{
	vm->pushNil();

	while ( vm->next(tableIndex) )
	{
		if ( vm->isFunction(-1) && vm->isCFunction(-1) )
		{
			// ignore 'init'
			if ( String("init") != vm->toString(-2) )
			{
				String funcName = vm->toString(-2);
				funcNames.add( funcName );
			}
		}

		vm->pop();
	}
}

/** Stores error to global '_err' variable. */
static int storeError( lua_State* lua )
{
	assert( lua_isstring(lua,1) );

	lua_setglobal( lua, "_err" );
	return 0;
}

//-----------------------------------------------------------------------------

Scriptable::Scriptable() :
	m_vm(0),
	m_tag(0), 
	m_methods(0)
{
}

Scriptable::Scriptable( VM* vm, int tag ) :
	Table(vm),
	m_vm(vm),
	m_tag(tag), 
	m_methods(0)
{
	if ( vm )
	{
		assert( vm );
		assert( vm->top() >= 0 );
		assert( tag > 0 );

		// set C++ this ptr to index 0
		setUserData( 0, this, tag );
	}
}

Scriptable::~Scriptable()
{
	if ( initialized() )
		remove( 0 );
}

void Scriptable::compileFile( const String& name )
{
	assert( m_vm );

	This th( m_vm, *this );
	m_vm->compileFile( name );
	m_source = name;
}

void Scriptable::compileBuffer( const void* buffer, int size, const String& name )
{
	assert( m_vm );

	This th( m_vm, *this );
	m_vm->compileBuffer( buffer, size, name );
	m_source = name;
}

int Scriptable::addMethod( const String& name )
{
	assert( m_vm );

	// add function closure (this,methodIndex) to table
	RestoreStack rs( m_vm );
	const int methodIndex = m_methods++;
	m_vm->pushTable( this );
	m_vm->pushString( name );
	m_vm->pushNumber( (float)methodIndex );
	m_vm->pushCClosure( callMethod, 1 );
	m_vm->setTable( -3 );
	return methodIndex;
}

void Scriptable::pushMethod( const String& name )
{
	assert( m_vm );
	StackCheck stackCheck( m_vm, 2, __FILE__, StackCheck::EXPLICIT_CHECK );

	// get method to stack top
	pushMember( name );
	m_vm->pushTable( *this );

	if ( !m_vm->isFunction(-2) )
	{
		m_vm->pop(2);
		throw ScriptException( Format("Function {0} not exist in script {1}", name, m_source) );
	}

	stackCheck.check();
}

bool Scriptable::hasMethod( const String& name ) const
{
	if ( !m_vm )
		return false;

	assert( m_vm );

	RestoreStack rs( m_vm );
	pushMember( name );
	return m_vm->isFunction(-1);
}

bool Scriptable::hasParams( const int* tags, int n, int opt ) const
{
	int params = m_vm->top();
	if ( params < n-opt )
		return false;

	int count = n;
	if ( count > params && count >= n-opt )
		count = params;

	for ( int i = 0 ; i < count ; ++i )
	{
		if ( m_vm->getTag(i+1) != tags[i] )
			return false;
	}

	return true;
}

void Scriptable::call( int nargs, int nresults )
{
	assert( m_vm );
	StackCheck stackCheck( m_vm, nresults-(nargs+2), __FILE__, StackCheck::EXPLICIT_CHECK );

	m_vm->call( 1+nargs, nresults );	// this + args

	stackCheck.check();
}

VM* Scriptable::vm() const
{
	assert( m_vm );
	return m_vm;
}

int Scriptable::methods() const
{
	return m_methods;
}

const String& Scriptable::source() const
{
	return m_source;
}

int Scriptable::methodCall( VM*, int )
{
	return 0;
}

void Scriptable::printFunctions( const String& objectType ) const
{
	assert( m_vm );

	// set temporary error handler
	m_vm->getGlobal( "_ERRORMESSAGE" );
	int olderr = m_vm->ref( true );
	m_vm->pushCFunction( storeError );
	m_vm->setGlobal( "_ERRORMESSAGE" );

	// list functions
	Vector<String> funcNames( Allocator<String>(__FILE__) );
	m_vm->pushTable( *this );
	getFunctions( m_vm, funcNames, m_vm->top() );
	m_vm->pop();
	std::sort( funcNames.begin(), funcNames.end() );

	// dummy parameter for functions
	int userDataTag = m_vm->newTag();

	Debug::println( "" );
	Debug::println( "Scriptable (C++) functions of {0}:", objectType );
	Debug::println( "" );
	for ( int i = 0 ; i < funcNames.size() ; ++i )
	{
		// print function name
		String funcName = funcNames[i];
		Debug::println( "  {0}", funcName );

		// set _err = nil
		m_vm->pushNil();
		m_vm->setGlobal( "_err" );

		// call function
		try
		{
			pushMember( funcName );
			m_vm->pushTable( *this );
			m_vm->pushUserTag( 0, userDataTag );
			m_vm->call( 2, 0 );
		}
		catch ( Throwable& )
		{
		}

		// check _err
		m_vm->getGlobal( "_err" );
		if ( m_vm->isString(-1) )
			Debug::println( "    Description: {0}", m_vm->toString(-1) );
		Debug::println( "" );

		m_vm->pop();
	}
	Debug::println( "-----------------------------------------------------------------------------" );
	Debug::println( "" );

	// restore error handler
	m_vm->getRef( olderr );
	m_vm->setGlobal( "_ERRORMESSAGE" );
	m_vm->unref( olderr );
}

Scriptable* Scriptable::getThisPtr( VM* vm, int stackIndex )
{
	if ( vm->isNil(stackIndex) )
		return 0;

	RestoreStack rs( vm );
	if ( !vm->isTable(stackIndex) )
		throw ScriptException( Format("C++ this ptr can be only in Lua table") );
	vm->getTableRawI( stackIndex, 0 );
	if ( !vm->isUserData(-1) )
		throw ScriptException( Format("C++ this ptr not in Lua table") );
	Scriptable* obj = (Scriptable*)vm->toUserData( -1 );
	return obj;
}


} // script
