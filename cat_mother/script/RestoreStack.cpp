#include "RestoreStack.h"
#include <script/VM.h>
#include <lua.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace script
{


RestoreStack::RestoreStack( VM* vm ) :
	m_lua( vm->lua() ),
	m_top( lua_gettop(m_lua) )
{
}

RestoreStack::RestoreStack( lua_State* lua ) :
	m_lua( lua ),
	m_top( lua_gettop(lua) )
{
}

RestoreStack::~RestoreStack()
{
	lua_settop( m_lua, m_top );
}


} // script
