#ifndef _SCRIPT_THIS_H
#define _SCRIPT_THIS_H


#include <script/VM.h>


namespace script
{


/** 
 * Sets table as global 'this'. Restores old 'this' at destructor. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class This
{
public:
	This( VM* vm, Table& tab ) : 
		m_vm( vm )
	{
		vm->getGlobal( "this" );
		m_old = vm->ref( true );
		
		vm->pushTable( tab );
		vm->setGlobal( "this" );
	}

	~This()
	{
		m_vm->getRef( m_old );
		m_vm->setGlobal( "this" );
		m_vm->unref( m_old );
	}

private:
	VM*	m_vm;
	int m_old;

	This( const This& );
	This& operator=( const This& );
};


} // script


#endif // _SCRIPT_THIS_H
