#ifndef _SCRIPT_VM_H
#define _SCRIPT_VM_H


#include "Table.h"
#include <lang/Object.h>


struct lua_State;


namespace script
{


/** 
 * Script virtual machine. Contains script execution environment like 
 * globals, stack, etc.
 * Class member functions map practically one-to-one with Lua API,
 * see Lua documentation for details.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class VM :
	public lang::Object
{
public:
	/** Value types. */
	enum Type
	{
		/** Value is user data. */
		TYPE_USERDATA,
		/** Value is nil. */
		TYPE_NIL,
		/** Value is number. */
		TYPE_NUMBER,
		/** Value is string. */
		TYPE_STRING,
		/** Value is table. */
		TYPE_TABLE,
		/** Value is function. */
		TYPE_FUNCTION
	};

	/** C function prototype. */
	typedef int (*CFunction)( lua_State* );

	/** 
	 * Initializes virtual machine with specified stack size. (0 for default) 
	 * @exception ScriptException
	 */
	explicit VM( int stackSize=0 );

	///
	~VM();

	/** Access to the implementation. */
	lua_State*		lua();

	/** 
	 * Calls script function. 
	 * Function and params must be pushed to stack before call. 
	 * Specified number of results is left to the stack after call.
	 * @exception ScriptException
	 */
	void			call( int args, int results );

	/** 
	 * Compiles script from ASCII-text file to executable code.
	 * @exception ScriptException
	 */
	void			compileFile( const lang::String& name );

	/** 
	 * Compiles script from zero-terminated string to executable code. 
	 * @exception ScriptException
	 */
	void			compileString( const lang::String& str );

	/** 
	 * Compiles script from buffer to executable code. 
	 * @exception ScriptException
	 */
	void			compileBuffer( const void* buffer, int size, const lang::String& name );

	/** Pushes global variable value to the stack. */
	void			getGlobal( const lang::String& name );

	/** Pushes global environment table to the stack. */
	void			getGlobals();

	/** 
	 * Returns value at nth key in a table. Value is pushed to the stack. Tag method is not called. 
	 * @param index Index of the table in stack.
	 */
	void			getTableRawI( int index, int n );

	/** 
	 * Returns value at specified key in a table. Key is popped from the stack, value is pushed. 
	 * @param index Index of the table in stack.
	 */
	void			getTable( int index );

	/** Pushes referred object to the stack. Returns 0 if (unlocked) object has been recycled. */
	int				getRef( int ref );

	/** 
	 * Creates a new unique tag.
	 * @see setTag
	 * @see pushUserTag
	 */
	int				newTag();

	/** 
	 * Iterates table. Pops key and pushes key-value pair. 
	 * Use nil key to start the iteration.
	 * If no more elements then the function returns false and does not push anything. 
	 * @param index Index of the table in stack.
	 * @return If no more elements then false. 
	 */
	bool			next( int index );

	/** Pops n items from the stack. */
	void			pop( int n=1 );

	/** Pushes nil to the stack. */
	void			pushNil();

	/** Pushes number to the stack. */
	void			pushNumber( float x );

	/** Pushes string to the stack. */
	void			pushString( const lang::String& str );

	/** Pushes table to the stack. */
	void			pushTable( const Table& tab );

	/** Pushes table to the stack or nil if 0-pointer is passed. */
	void			pushTable( const Table* tab );

	/** Pushes C function to the stack. Same as C function closure with zero parameters. */
	void			pushCFunction( CFunction f );

	/** Pushes C function closure to the stack with n parameters. */
	void			pushCClosure( CFunction f, int n );

	/** 
	 * Pushes user data to the stack. 
	 * @see setTag
	 * @see newTag
	 */
	void			pushUserTag( void* u, int tag );

	/** Pushes a copy of an item in the stack. */
	void			pushValue( int index );

	/** Pushes boolean to the stack. */
	void			pushBoolean( bool x );

	/** Sets global variable value and pops value from the stack. */
	void			setGlobal( const lang::String& name );

	/** Sets global environment table and pops table from the stack. */
	void			setGlobals();

	/** 
	 * Sets value at specified key in a table. Key-value pair is popped from the stack. 
	 * @param index Index of the table in stack.
	 */
	void			setTable( int index );

	/** 
	 * Sets value at specified key in a table. Key-value pair is popped from the stack. Tag method is not called. 
	 * @param index Index of the table in stack.
	 */
	void			setTableRaw( int index );

	/** 
	 * Sets value at nth key in a table. Value is popped from the stack. Tag method is not called. 
	 * @param index Index of the table in stack.
	 */
	void			setTableRawI( int index, int n );

	/** 
	 * Sets tag of the top item of the stack. 
	 * @see setTag
	 * @see newTag
	 * @see pushUserTag
	 */
	void			setTag( int tag );

	/** Sets top of the stack. */
	void			setTop( int index );

	/** 
	 * Pops a value from the stack and creates reference to it. 
	 * Locked item can't be garbage collected. 
	 */
	int				ref( bool lock );

	/** Removes an item from the stack. */
	void			remove( int index );

	/** Releases a reference. */
	void			unref( int ref );

	/** Force garbage collection. */
	void			gc();

	/** Returns specified stack trace level. Level 0 is current function. */
	lang::String	getStackTrace( int level ) const;

	/** 
	 * Returns type of an item in the stack. 
	 * @see TagType
	 */
	int				getTag( int index ) const;

	/** Returns type of an item in the stack. */
	int				getType( int index ) const;

	/** Returns name of the type returned by getType(). */
	lang::String	getTypeName( int type ) const;

	/** Returns true if an item in the stack is a C function. */
	bool			isCFunction( int index ) const;

	/** Returns true if an item in the stack is a function. */
	bool			isFunction( int index ) const;

	/** Returns true if an item in the stack is a nil value. */
	bool			isNil( int index ) const;

	/** Returns true if an item in the stack is a number. */
	bool			isNumber( int index ) const;

	/** Returns true if an item in the stack is a string. */
	bool			isString( int index ) const;

	/** Returns true if an item in the stack is a table. */
	bool			isTable( int index ) const;

	/** Returns true if an item in the stack is user data. */
	bool			isUserData( int index ) const;

	/** Returns true if an item in the stack is 1 or nil. */
	bool			isBoolean( int index ) const;

	/** Compares two items in the stack. */
	bool			isEqual( int index1, int index2 ) const;

	/** Compares two items in the stack. */
	bool			isLess( int index1, int index2 ) const;

	/** Returns available stack space in bytes. */
	int				stackSpace() const;

	/** Returns number from the stack. */
	float			toNumber( int index ) const;

	/** Returns string from the stack. */
	lang::String	toString( int index ) const;

	/** Returns table reference from the stack. */
	Table			toTable( int index ) const;

	/** Returns C function from the stack. */
	CFunction		toCFunction( int index ) const;

	/** Returns user data from the stack. */
	void*			toUserData( int index ) const;

	/** Returns stack top index. */
	int				top() const;

	/** 
	 * Prints error message to debug output and stderr. 
	 * Function accepts single parameter, message to be printed.
	 * The function expects VM* as closure.
	 * printMessage is set to '_ERRORMESSAGE' in environment initialization.
	 */
	static int		printError( lua_State* lua );

	/** 
	 * Prints message to debug output. 
	 * Function accepts single parameter, message to be printed.
	 * printMessage is set to 'trace' in environment initialization.
	 */
	static int		printMessage( lua_State* lua );

private:
	lua_State*		m_lua;
	lang::String	m_err;

	/** 
	 * Returns value at specified key in a table. Key is popped from the stack, value is pushed. Tag method is not called. 
	 * @param index Index of the table in stack.
	 */
	void			getTableRaw( int index );

	VM( const VM& );
	VM& operator=( const VM& );
};


} // script


#endif // _SCRIPT_VM_H
