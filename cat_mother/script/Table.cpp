#include "Table.h"
#include "VM.h"
#include "lua_ex.h"
#include "RestoreStack.h"
#include <lang/Array.h>
#include <script/ScriptException.h>
#include <lua.h>
#include <string.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace script
{


Table::Table() :
	m_lua( 0 ),
	m_ref( -1 )
{
}

Table::Table( VM* vm ) :
	m_lua( 0 ),
	m_ref( -1 )
{
	if ( vm )
	{
		assert( vm );
		m_lua = vm->lua();
		lua_newtable( m_lua );
		m_ref = lua_ref( m_lua, true );
	}
}

Table::Table( const Table& other ) :
	m_lua( other.m_lua ),
	m_ref( -1 )
{
	if ( other.m_lua && other.m_ref >= 0 )
	{
		lua_getref( m_lua, other.m_ref );
		m_ref = lua_ref( m_lua, true );
	}
}

Table::~Table()
{
	if ( m_ref >= 0 )
		lua_unref( m_lua, m_ref );
}

Table& Table::operator=( const Table& other )
{
	int oldref = m_ref;
	lua_State* oldlua = m_lua;

	if ( other.m_lua && other.m_ref >= 0 )
	{
		lua_getref( m_lua, other.m_ref );
		m_lua = other.m_lua;
		m_ref = lua_ref( other.m_lua, true );
	}
	else
	{
		m_lua = 0;
		m_ref = -1;
	}

	if ( oldlua && oldref >= 0 )
		lua_unref( oldlua, oldref );

	return *this;
}

void Table::remove( int index )
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushnil( m_lua );
	lua_rawseti( m_lua, -2, index );
}

void Table::remove( const lang::String& name )
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_pushnil( m_lua );
	lua_rawset( m_lua, -3 );
}

void Table::setString( int index, const String& v )
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, v );
	lua_rawseti( m_lua, -2, index );
}

void Table::setNumber( int index, float v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushnumber( m_lua, v );
	lua_rawseti( m_lua, -2, index );
}

void Table::setTable( int index, const Table& v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushTable( m_lua, &v );
	lua_rawseti( m_lua, -2, index );
}

void Table::setTable( int index, const Table* v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushTable( m_lua, v );
	lua_rawseti( m_lua, -2, index );
}

void Table::setBoolean( int index, bool v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	if ( v )
		lua_pushnumber( m_lua, 1.f );
	else
		lua_pushnil( m_lua );
	lua_rawseti( m_lua, -2, index );
}

void Table::setString( const String& name, const String& v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_pushUTF8( m_lua, v );
	lua_rawset( m_lua, -3 );
}

void Table::setNumber( const String& name, float v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_pushnumber( m_lua, v );
	lua_rawset( m_lua, -3 );
}

void Table::setTable( const String& name, const Table& v ) 
{
	assert( m_ref >= 0 );
	assert( v.m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_pushTable( m_lua, &v );
	lua_rawset( m_lua, -3 );
}

void Table::setTable( const String& name, const Table* v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_pushTable( m_lua, v );
	lua_rawset( m_lua, -3 );
}

void Table::setBoolean( const String& name, bool v ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	if ( v )
		lua_pushnumber( m_lua, 1.f );
	else
		lua_pushnil( m_lua );
	lua_rawset( m_lua, -3 );
}

String Table::getString( const String& name ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_rawget( m_lua, -2 );
	return lua_toUTF8( m_lua, -1 );
}

float Table::getNumber( const String& name ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_rawget( m_lua, -2 );
	if ( lua_type(m_lua,-1) != LUA_TNUMBER )
		throw ScriptException( Format("Tried to get number {0} from table but type was invalid ({1})", name, lua_type(m_lua,-1)) );
	return lua_tonumber( m_lua, -1 );
}

Table Table::getTable( const String& name ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_rawget( m_lua, -2 );
	if ( !lua_istable(m_lua,-1) )
		throw ScriptException( Format("Tried to get table {0} from table but type was invalid ({1})", name, lua_type(m_lua,-1)) );

	Table tab;
	tab.m_lua = m_lua;
	tab.m_ref = lua_ref( m_lua, true );
	return tab;
}

String Table::getString( int index ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_rawgeti( m_lua, -1, index );
	return lua_toUTF8( m_lua, -1 );
}

float Table::getNumber( int index ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_rawgeti( m_lua, -1, index );
	if ( lua_type(m_lua,-1) != LUA_TNUMBER )
		throw ScriptException( Format("Tried to get number from table but type was invalid ({1})", lua_type(m_lua,-1)) );
	return lua_tonumber( m_lua, -1 );
}

Table Table::getTable( int index ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_rawgeti( m_lua, -1, index );
	if ( !lua_istable(m_lua,-1) )
		throw ScriptException( Format("Tried to get table from table but type was invalid ({1})", lua_type(m_lua,-1)) );

	Table tab;
	tab.m_lua = m_lua;
	tab.m_ref = lua_ref( m_lua, true );
	return tab;
}

bool Table::getBoolean( const String& name ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_rawget( m_lua, -2 );
	if ( lua_type(m_lua,-1) != LUA_TNUMBER && !lua_isnil(m_lua,-1) )
		throw ScriptException( Format("Tried to get boolean {0} from table but type was invalid ({1})", name, lua_type(m_lua,-1)) );
	return !lua_isnil( m_lua, -1 );
}

void Table::setUserData( int index, void* userData, int tag ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushusertag( m_lua, userData, tag );
	lua_rawseti( m_lua, -2, index );
}

void Table::setUserData( const String& name, void* userData, int tag )
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_pushusertag( m_lua, userData, tag );
	lua_rawset( m_lua, -3 );
}

void* Table::getUserData( int index ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_rawgeti( m_lua, -1, index );
	if ( !lua_isuserdata(m_lua,-1) )
		throw ScriptException( Format("Tried to use script object type({0}) as user data", lua_type(m_lua,-1)) );
	return lua_touserdata( m_lua, -1 );
}

void* Table::getUserData( const String& name ) 
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_rawget( m_lua, -2 );
	if ( !lua_isuserdata(m_lua,-1) )
		throw ScriptException( Format("Tried to use script object {0} type({1}) as user data", name, lua_type(m_lua,-1)) );
	return lua_touserdata( m_lua, -1 );
}

bool Table::isNil( const lang::String& name )
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_rawget( m_lua, -2 );
	bool isnil = 0 != lua_isnil( m_lua, -1 );
	return isnil;
}

bool Table::isNil( int index )
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	lua_rawgeti( m_lua, -1, index );
	bool isnil = 0 != lua_isnil( m_lua, -1 );
	return isnil;
}

void Table::pushMember( int index ) const
{
	assert( m_ref >= 0 );

	lua_getref( m_lua, m_ref );
	lua_rawgeti( m_lua, -1, index );
	lua_remove( m_lua, -2 );
}

void Table::pushMember( const lang::String& name ) const
{
	assert( m_ref >= 0 );

	lua_getref( m_lua, m_ref );
	lua_pushUTF8( m_lua, name );
	lua_rawget( m_lua, -2 );
	lua_remove( m_lua, -2 );
}

int Table::lua() const
{
	return m_ref;
}

bool Table::initialized() const
{
	return m_ref >= 0;
}

int Table::size() const
{
	assert( m_ref >= 0 );

	RestoreStack rs( m_lua );
	lua_getref( m_lua, m_ref );
	return lua_getn( m_lua, -1 );
}


} // script
