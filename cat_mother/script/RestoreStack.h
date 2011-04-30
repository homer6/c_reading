#ifndef _SCRIPT_RESTORESTACK_H
#define _SCRIPT_RESTORESTACK_H


struct lua_State;


namespace script
{


class VM;


/** 
 * Restores script VM stack top at the end of the scope. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class RestoreStack
{
public:
	RestoreStack( VM* vm );
	RestoreStack( lua_State* lua );
	~RestoreStack();

private:
	lua_State*	m_lua;
	int			m_top;

	RestoreStack( const RestoreStack& );
	RestoreStack& operator=( const RestoreStack& );
};


} // script


#endif // _SCRIPT_RESTORESTACK_H
