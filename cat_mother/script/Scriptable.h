#ifndef _SCRIPT_SCRIPTABLE_H
#define _SCRIPT_SCRIPTABLE_H


#include <lang/Object.h>
#include <lang/String.h>
#include <script/Table.h>


namespace script
{


class VM;
	

/** 
 * Base class for C++ objects which can interact with scripts. 
 * For each C++ Scriptable object there is a Lua entity, table.
 * Both Lua and C++ methods can be added to the table so that
 * C++ methods can call Lua methods and vice versa.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Scriptable :
	public Table
{
public:
	/** Initializes the object with scripting disabled. */
	Scriptable();

	/** 
	 * Initializes scriptable object in specified environment. 
	 * @param vm Script execution environment.
	 * @param classTag Unique class identifier created with VM::newTag().
	 */
	explicit Scriptable( VM* vm, int classTag );

	///
	virtual ~Scriptable();

	/** 
	 * Compiles script from ASCII-7 text file to executable code.
	 * @exception ScriptException
	 */
	void			compileFile( const lang::String& name );

	/** 
	 * Compiles script from ASCII-7 text buffer to executable code. 
	 * @exception ScriptException
	 */
	void			compileBuffer( const void* buffer, int size, const lang::String& name );

	/** 
	 * Adds a (C++) member function to be usable from scripts. 
	 * When the function is called from a script, virtual methodCall()
	 * is executed and unique function identifier is passed as parameter.
	 * @return Unique function identifier (index).
	 * @see methodCall
	 */
	int				addMethod( const lang::String& name );

	/** 
	 * Pushes a script object member function followed by script object table to the script stack. 
	 * To call a script function, first push the function
	 * with this (pushMethod), then push arguments (pushParam)
	 * and finally call(nargs,nresults) method.
	 * @exception ScriptException
	 * @see call
	 * @see pushParam
	 */
	void			pushMethod( const lang::String& name );

	/** 
	 * Calls a script function using this object as global environment. 
	 * Method and parameters must be pushed to script stack before the call.
	 * Specified number of results is left to the stack after execution.
	 * @exception ScriptException
	 * @see pushMethod
	 * @see pushParam
	 */
	void			call( int nargs, int nresults );

	/**
	 * When a (C++) member function is called from a script, this function
	 * is executed and unique function identifier is passed as parameter.
	 * Derived classes must override this if they add new scriptable functions.
	 * @param vm Script virtual machine executing the method.
	 * @param i Unique function identifier (index).
	 * @return Number of arguments returned in the script stack.
	 * @exception ScriptException
	 * @see addMethod
	 */
	virtual int		methodCall( VM* vm, int i );

	/** Returns true if the object has a script function with specified name. */
	bool			hasMethod( const lang::String& name ) const;

	/**
	 * Checks from script stack that the tags of the parameters passed 
	 * to the (C++) method match specified sequence of tags.
	 * @param tags Array of type tags to check.
	 * @param n Maximum number of parameters.
	 * @param opt Number of optional parameters (at the end).
	 * @return true if the parameter tags match, false otherwise.
	 */
	bool			hasParams( const int* tags, int n, int opt=0 ) const;

	/** Returns script execution environment of the object. */
	VM*				vm() const;

	/** Returns number of (C++) member functions there is in the script object. */
	int				methods() const;

	/** Returns name of the script source. */
	const lang::String&		source() const;

	/** Prints debug info about functions in the object table. */
	void			printFunctions( const lang::String& objectType ) const;

	/** 
	 * Returns C++ 'this' ptr from Lua table. 
	 * @exception ScriptException
	 */
	static Scriptable*		getThisPtr( VM* vm, int stackIndex );

private:
	P(VM)			m_vm;
	int				m_tag;
	int				m_methods;
	lang::String	m_source;

	Scriptable( const Scriptable& );
	Scriptable& operator=( const Scriptable& );
};


} // script


#endif // _SCRIPT_SCRIPTABLE_H
