#ifndef _SCRIPT_TABLE_H
#define _SCRIPT_TABLE_H


#include <lang/Object.h>
#include <lang/String.h>


struct lua_State;


namespace script
{


class VM;


/** 
 * Wrapper for Lua table reference. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Table :
	public lang::Object
{
public:
	Table();

	/** Creates a table to specified script VM. */
	explicit Table( VM* vm );

	/** Copy by reference. */
	Table( const Table& other );
	
	~Table();

	/** Copy by reference. */
	Table&			operator=( const Table& other );

	void			remove( int index );
	void			remove( const lang::String& name );
	void			setString( int index, const lang::String& v );
	void			setNumber( int index, float v );
	void			setTable( int index, const Table& v );
	void			setTable( int index, const Table* v );
	void			setBoolean( int index, bool v );
	void			setUserData( int index, void* userData, int tag );
	void			setString( const lang::String& name, const lang::String& v );
	void			setNumber( const lang::String& name, float v );
	void			setTable( const lang::String& name, const Table& v );
	void			setTable( const lang::String& name, const Table* v );
	void			setBoolean( const lang::String& name, bool v );
	void			setUserData( const lang::String& name, void* userData, int tag );

	lang::String	getString( const lang::String& name );
	lang::String	getString( int index );
	float			getNumber( const lang::String& name );
	float			getNumber( int index );
	Table			getTable( const lang::String& name );
	Table			getTable( int index );
	bool			getBoolean( const lang::String& name );
	bool			getBoolean( int index );
	void*			getUserData( int index );
	void*			getUserData( const lang::String& name );

	bool			isNil( const lang::String& name );
	bool			isNil( int index );

	/** Pushes member by name to script VM stack. */
	void			pushMember( int index ) const;

	/** Pushes member by index to script VM stack. */
	void			pushMember( const lang::String& name ) const;

	/** Returns number of elements in the table. This number is either value of 'n' or largest numerical index with non-nil value. */
	int				size() const;

	/** Returns true if the table has been initialized to a VM. */
	bool			initialized() const;

	/** Return Lua table reference or -1 if not initialized. */
	int				lua() const;

private:
	friend class VM;

	lua_State*	m_lua;
	int			m_ref;
};


} // script


#endif // _SCRIPT_TABLE_H
