#ifndef _SCRIPTMETHOD_H
#define _SCRIPTMETHOD_H


#include <script/VM.h>


/** 
 * Default method type of scriptable object.
 * For example usage see class GamePlayer.
 */
template <class T> class ScriptMethod
{
public:
	/** Method prototype. */
	typedef int (T::*FuncType)( script::VM* vm, const char* funcName );

	/** 
	 * Creates method with specified name and function address. 
	 * Note that the name is <em>not</em> copied so it needs to be string literal.
	 */
	ScriptMethod( const char* name, FuncType func )									: m_name(name), m_func(func) {}

	/** Returns name of the method. */
	const char*		name() const													{return m_name;}

	/** Returns function address of the method. */
	FuncType		func() const													{return m_func;}

private:
	const char*		m_name;
	FuncType		m_func;
};


#endif // _SCRIPTMETHOD_H
