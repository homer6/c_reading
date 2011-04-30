#include "build.h"
#include <lang/String.h>
#include <lang/Debug.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define EC(x) ((int)x-123)*123-321
#define DC(x) (Char)((x+321)/123+123)

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

void printBuildInfo()
{
	//static const int idstr[] = { EC('V'), EC('a'), EC('l'), EC('t'), EC('o'), EC('n'), EC('e'), 0 };
	static const int idstr[] = { EC('i'), EC('n'), EC('-'), EC('h'), EC('o'), EC('u'), EC('s'), EC('e'), 0 };
	
	String str;
	for ( int i = 0 ; idstr[i] ; ++i )
	{
		Char ch[2] = {0,0};
		ch[0] = DC( idstr[i] );
		str = str + String(ch);
	}

	Debug::println( "Dead Justice Build {0} (for {1})", BUILD_NUMBER, str );
}
