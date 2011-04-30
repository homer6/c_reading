#ifndef _SCRIPT_SCRIPTEXCEPTION_H
#define _SCRIPT_SCRIPTEXCEPTION_H


#include <lang/Exception.h>


namespace script
{


/**
 * Thrown if the application or script performs invalid script operation.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ScriptException :
	public lang::Exception
{
public:
	///
	ScriptException( const lang::Format& msg )										: Exception(msg) {}
};


} // script


#endif // _SCRIPT_SCRIPTEXCEPTION_H

