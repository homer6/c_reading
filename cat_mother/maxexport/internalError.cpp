#include "StdAfx.h"
#include <lang/Debug.h>
#include <lang/Error.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

void internalError( const char* fname, int line, const char* expr )
{
	Debug::printlnError( "Internal error: {0}({1,#}), failed expression: \"{2}\"", fname, line, expr );
	throw Error( Format("Internal error: {0}({1,#})\nFailed expression: {2}\n(report to programmers!)", fname, line, expr) );
}
