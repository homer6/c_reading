#ifndef _SCRIPT_LUA_EX_H
#define _SCRIPT_LUA_EX_H


#include <lang/String.h>


struct lua_State;


namespace script
{


class Table;


void			lua_pushUTF8( lua_State* lua, const lang::String& v );
lang::String	lua_toUTF8( lua_State* lua, int index );
void			lua_pushTable( lua_State* lua, const Table* v );


} // script


#endif // _SCRIPT_LUA_EX_H
