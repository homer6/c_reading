#include <StdAfx.h>
#include "DebugUtil.h"
#include <lang/Debug.h>

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

void DebugUtil::printTM( const lang::String& name, const Matrix3& tm )
{
	Debug::println( "{0} transform:", name );
	for ( int i = 0 ; i < 4 ; ++i )
		Debug::println( "  {0} {1} {2}", tm.GetRow(i).x, tm.GetRow(i).y, tm.GetRow(i).z );
}
