#include "VM.h"
#include "ScriptException.h"
#include <dev/Profile.h>
#include <dev/TimeStamp.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include <lualib.h>
#include <luadebug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <lua.h>
#include "config.h"

//#define LUA_DEBUG

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace script
{


#ifdef LUA_DEBUG
static FILE* s_log = 0;

static void linehook( lua_State*, lua_Debug* ar )
{
	if ( !s_log )
		s_log = fopen( "/tmp/lualog.txt", "wt" );
	if ( s_log )
		fprintf( s_log, "%i\n", ar->currentline );
}

static void callhook( lua_State* lua, lua_Debug* ar )
{
	lua_getinfo( lua, "S", ar );
	if ( !s_log )
		s_log = fopen( "/tmp/lualog.txt", "wt" );
	if ( s_log )
		fprintf( s_log, "%s %s\n", ar->event, ar->source );
}
#endif

//-----------------------------------------------------------------------------

VM::VM( int stackSize )
{
	assert( TYPE_USERDATA == LUA_TUSERDATA );
	assert( TYPE_NIL == LUA_TNIL );
	assert( TYPE_NUMBER == LUA_TNUMBER );
	assert( TYPE_STRING == LUA_TSTRING );
	assert( TYPE_TABLE == LUA_TTABLE );
	assert( TYPE_FUNCTION == LUA_TFUNCTION );
	
	m_lua = lua_open( stackSize );
	if ( !m_lua )
		throw ScriptException( Format("Failed to initialize Lua virtual machine") );

	// set '_ERRORMESSAGE' handler
	lua_pushusertag( m_lua, this, lua_newtag(m_lua) );
	lua_pushcclosure( m_lua, printError, 1 );
	lua_setglobal( m_lua, "_ERRORMESSAGE" );

	// set 'trace' handler
	lua_pushcclosure( m_lua, printMessage, 0 );
	lua_setglobal( m_lua, "trace" );

	// create 'string' table
	lua_newtable( m_lua );
	lua_setglobal( m_lua, "string" );
	lua_getglobals( m_lua );
	int oldGlobals = lua_ref( m_lua, true );
	lua_getglobal( m_lua, "string" );
	lua_setglobals( m_lua );
	lua_strlibopen( m_lua );
	lua_getref( m_lua, oldGlobals );
	lua_setglobals( m_lua );
	lua_unref( m_lua, oldGlobals );

	// create 'math' table
	lua_newtable( m_lua );
	lua_setglobal( m_lua, "math" );
	lua_getglobals( m_lua );
	oldGlobals = lua_ref( m_lua, true );
	lua_getglobal( m_lua, "math" );
	lua_setglobals( m_lua );
	lua_mathlibopen( m_lua );
	lua_getref( m_lua, oldGlobals );
	lua_setglobals( m_lua );
	lua_unref( m_lua, oldGlobals );

	// set line hook
	#ifdef LUA_DEBUG
	lua_setlinehook( m_lua, linehook );
	lua_setcallhook( m_lua, callhook );
	#endif
}

VM::~VM()
{
	if ( m_lua )
	{
		lua_settop( m_lua, 0 );
		lua_close( m_lua );
		m_lua = 0;
	}
}

lua_State* VM::lua()
{
	return m_lua;
}

void VM::call( int args, int results )
{
	assert( lua_isfunction(m_lua,-args-1) );
	if ( !lua_isfunction(m_lua,-args-1) )
		throw ScriptException( Format("Called invalid script function\n") );

	int err = lua_call( m_lua, args, results );
	if ( err )
	{
		if ( m_err.startsWith("Script error: ") )
			throw ScriptException( Format("{0}", m_err) );
		else
			throw ScriptException( Format("Script error: {0}\n", m_err) );
	}
}

void VM::compileFile( const String& name ) 
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	int err = lua_dofile( m_lua, buf.begin() );
	if ( err )
		throw ScriptException( Format("Failed to compile script file: {0}\n{1}", name, m_err) );
}

void VM::compileString( const String& str ) 
{
	Array<char,512> buf( str.length()+1 );
	str.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	int err = lua_dostring( m_lua, buf.begin() );
	if ( err )
		throw ScriptException( Format("Failed to compile string: {0}\n{1}", str, m_err) );
}

void VM::compileBuffer( const void* buffer, int size, const String& name ) 
{
	Array<char,512> buf( name.length()+1 );
	name.getBytes( buf.begin(), buf.size(), "ASCII-7" );

	int err = lua_dobuffer( m_lua, (char*)buffer, size, buf.begin() );
	if ( err )
		throw ScriptException( Format("Failed to compile buffer: {0}\n{1}", name, m_err) );
}

void VM::getGlobal( const String& name ) 
{
	Array<char,512> buf( name.getBytes(0,0,"UTF-8")+1 );
	name.getBytes( buf.begin(), buf.size(), "UTF-8" );

	lua_getglobal( m_lua, buf.begin() );
}

void VM::getGlobals() 
{
	lua_getglobals( m_lua );
}

void VM::getTable( int index ) 
{
	assert( lua_istable(m_lua,index) );
	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	lua_gettable( m_lua, index );
}

int VM::getRef( int ref ) 
{
	return lua_getref( m_lua, ref );
}

void VM::getTableRaw( int index ) 
{
	assert( lua_istable(m_lua,index) );
	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	lua_rawget( m_lua, index );
}

void VM::getTableRawI( int index, int n ) 
{
	assert( lua_istable(m_lua,index) );
	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	lua_rawgeti( m_lua, index, n );
}

int VM::newTag() 
{
	return lua_newtag( m_lua );
}

bool VM::next( int index ) 
{
	assert( lua_istable(m_lua,index) );

	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	
	return 0 != lua_next( m_lua, index );
}

void VM::pop( int n )
{
	lua_pop( m_lua, n );
}

void VM::pushNil() 
{
	lua_pushnil( m_lua );
}

void VM::pushNumber( float x ) 
{
	lua_pushnumber( m_lua, x );
}

void VM::pushString( const String& str ) 
{
	Array<char,512> buf( str.getBytes(0,0,"UTF-8")+1 );
	str.getBytes( buf.begin(), buf.size(), "UTF-8" );

	lua_pushstring( m_lua, buf.begin() );
}

void VM::pushTable( const Table& tab ) 
{
	assert( m_lua == tab.m_lua );

	if ( tab.m_ref >= 0 )
		lua_getref( m_lua, tab.m_ref );
	else
		lua_pushnil( m_lua );
}

void VM::pushTable( const Table* tab ) 
{
	if ( tab && tab->m_ref >= 0 )
		lua_getref( m_lua, tab->m_ref );
	else
		lua_pushnil( m_lua );
}

void VM::pushCFunction( CFunction f ) 
{
	lua_pushcclosure( m_lua, f, 0 );
}

void VM::pushCClosure( CFunction f, int n ) 
{
	lua_pushcclosure( m_lua, f, n );
}

void VM::pushUserTag( void* u, int tag ) 
{
	lua_pushusertag( m_lua, u, tag );
}

void VM::pushValue( int index ) 
{
	lua_pushvalue( m_lua, index );
}

void VM::pushBoolean( bool x )
{
	if ( x )
		lua_pushnumber( m_lua, 1.f );
	else
		lua_pushnil( m_lua );
}

void VM::setGlobal( const String& name ) 
{
	Array<char,512> buf( name.getBytes(0,0,"UTF-8")+1 );
	name.getBytes( buf.begin(), buf.size(), "UTF-8" );

	lua_setglobal( m_lua, buf.begin() );
}

void VM::setGlobals() 
{
	lua_setglobals( m_lua );
}

void VM::setTable( int index ) 
{
	assert( lua_istable(m_lua,index) );
	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	lua_settable( m_lua, index );
}

void VM::setTableRaw( int index ) 
{
	assert( lua_istable(m_lua,index) );
	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	lua_rawset( m_lua, index );
}

void VM::setTableRawI( int index, int n ) 
{
	assert( lua_istable(m_lua,index) );
	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	lua_rawseti( m_lua, index, n );
}

void VM::setTag( int tag ) 
{
	lua_settag( m_lua, tag );
}

void VM::setTop( int index ) 
{
	lua_settop( m_lua, index );
}

int VM::ref( bool lock ) 
{
	return lua_ref( m_lua, lock );
}

void VM::remove( int index ) 
{
	lua_remove( m_lua, index );
}

void VM::unref( int ref ) 
{
	lua_unref( m_lua, ref );
}

void VM::gc()
{
	dev::TimeStamp t0;
	lua_gc( m_lua );
	dev::TimeStamp t1;
	Debug::println( "Garbage collection took {0} seconds", (float)(t1-t0).seconds() );
}

int VM::getTag( int index ) const 
{
	return lua_tag( m_lua, index );
}

int VM::getType( int index ) const 
{
	return lua_type( m_lua, index );
}

String VM::getTypeName( int type ) const 
{
	return lua_typename( m_lua, type );
}

bool VM::isCFunction( int index ) const 
{
	return 0 != lua_iscfunction( m_lua, index );
}

bool VM::isFunction( int index ) const 
{
	return lua_type( m_lua, index ) == LUA_TFUNCTION;
}

bool VM::isNil( int index ) const
{
	return lua_type( m_lua, index ) == LUA_TNIL;
}

bool VM::isNumber( int index ) const 
{
	return lua_type( m_lua, index ) == LUA_TNUMBER;
}

bool VM::isString( int index ) const 
{
	return lua_type( m_lua, index ) == LUA_TSTRING;
}

bool VM::isTable( int index ) const 
{
	return lua_type( m_lua, index ) == LUA_TTABLE;
}

bool VM::isUserData( int index ) const 
{
	return lua_type( m_lua, index ) == LUA_TUSERDATA;
}

bool VM::isBoolean( int index ) const
{
	return isNil(index) || ( isNumber(index) && 1.f == toNumber(index) );
}

bool VM::isEqual( int index1, int index2 ) const 
{
	return 0 != lua_equal( m_lua, index1, index2 );
}

bool VM::isLess( int index1, int index2 ) const 
{
	return 0 != lua_lessthan( m_lua, index1, index2 );
}

int VM::stackSpace() const 
{
	return lua_stackspace( m_lua );
}

float VM::toNumber( int index ) const 
{
	assert( lua_type( m_lua, index ) == LUA_TNUMBER );
	if ( lua_type( m_lua, index ) != LUA_TNUMBER )
		throw ScriptException( Format("Tried to use script object type({0}) as number", lua_type(m_lua,index)) );
	return lua_tonumber( m_lua, index );
}

String VM::toString( int index ) const 
{
	assert( lua_isstring(m_lua,index) );
	if ( !lua_isstring(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as string", lua_type(m_lua,index)) );
	return lua_tostring( m_lua, index );
}

Table VM::toTable( int index ) const
{
	assert( lua_istable(m_lua,index) );
	
	if ( !lua_istable(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as table", lua_type(m_lua,index)) );
	lua_pushvalue( m_lua, index );
	
	Table tab;
	tab.m_lua = m_lua;
	tab.m_ref = lua_ref( m_lua, true );
	return tab;
}

VM::CFunction VM::toCFunction( int index ) const 
{
	assert( lua_iscfunction(m_lua,index) );
	if ( !lua_iscfunction(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as C-function", lua_type(m_lua,index)) );
	return lua_tocfunction( m_lua, index );
}

void* VM::toUserData( int index ) const 
{
	if ( !lua_isuserdata(m_lua,index) )
		throw ScriptException( Format("Tried to use script object type({0}) as user data", lua_type(m_lua,index)) );
	return lua_touserdata( m_lua, index );
}

int VM::top() const
{
	return lua_gettop( m_lua );
}

String VM::getStackTrace( int level ) const
{
	lua_Debug ar;
	memset( &ar, 0, sizeof(ar) );
	if ( !lua_getstack( m_lua, 1+level, &ar ) )
		return "";
	if ( !lua_getinfo( m_lua, "Snl", &ar ) )
		return "";

	char str[512];
	sprintf( str, "at %s(%s:%i)", ar.name, ar.source, ar.currentline );
	return str;
}

int VM::printError( lua_State* lua )
{
	assert( lua_isstring(lua,1) );
	assert( lua_isuserdata(lua,-1) );

	String msg = lua_tostring( lua, 1 );
	VM* vm = (VM*)lua_touserdata( lua, -1 );
	vm->m_err = msg;

	// get stack trace
	const int MAX_LEVEL = 20;
	String trace[MAX_LEVEL];
	int level = 0;
	for ( ; level < MAX_LEVEL ; ++level )
	{
		String str = vm->getStackTrace( level );
		if ( str == "" )
			break;
		trace[level] = str;

		if ( 0 == level )
			vm->m_err = vm->m_err + "\nStack trace:";
		vm->m_err = vm->m_err + "\n  " + str;
	}

	// debug output
	Debug::printlnError( "script: {0}", msg );
	if ( level > 0 )
	{
		Debug::printlnError( "Stack trace:" );
		for ( int i = 0 ; i < level ; ++i )
			Debug::printlnError( "  {0}", trace[i] );
	}
	return 0;
}

int VM::printMessage( lua_State* lua )
{
	assert( lua_isstring(lua,1) );

	String msg = lua_tostring( lua, 1 );

	// get caller
	lua_Debug ar;
	memset( &ar, 0, sizeof(ar) );
	lua_getstack( lua, 1, &ar );
	lua_getinfo( lua, "Snl", &ar );

	// debug output
	String str = ar.source;
	Debug::println( "script: {0} -- at {1}({2})", msg, str, ar.currentline );
	return 0;
}


} // script
