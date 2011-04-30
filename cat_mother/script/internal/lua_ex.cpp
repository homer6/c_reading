#include "lua_ex.h"
#include <lang/Array.h>
#include <lang/String.h>
#include <script/Table.h>
#include <script/ScriptException.h>
#include <lua.h>
#include <string.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace script
{


void lua_pushUTF8( lua_State* lua, const String& v )
{
	Array<char,512> buf( v.getBytes(0,0,"UTF-8")+1 );
	v.getBytes( buf.begin(), buf.size(), "UTF-8" );
	lua_pushstring( lua, buf.begin() );
}

String lua_toUTF8( lua_State* lua, int index )
{
	if ( !lua_isstring(lua, index) )
		throw ScriptException( Format("Tried to get string from table but type was invalid ({1})", lua_type(lua,index)) );

	const char* sz = lua_tostring( lua, index );
	String str( (void*)sz, strlen(sz), "UTF-8" );
	return str;
}

void lua_pushTable( lua_State* lua, const Table* v )
{
	if ( v && v->lua() >= 0 )
		lua_getref( lua, v->lua() );
	else
		lua_pushnil( lua );
}


} // script
