#ifndef _SG_LOCKEXCEPTION_H
#define _SG_LOCKEXCEPTION_H


#include <lang/Exception.h>


namespace sg
{


/** 
 * Throw if graphics device locking failed. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LockException :
	public lang::Exception
{
public:
	LockException( const lang::String& typeName )	: lang::Exception( lang::Format("Failed to lock {0}", typeName) ) {}
};


} // sg


#endif // _SG_LOCKEXCEPTION_H
