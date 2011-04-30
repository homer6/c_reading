#include "GameSurface.h"
#include "config.h"

//-----------------------------------------------------------------------------

GameSurface::GameSurface( script::VM* vm, io::InputStreamArchive* arch, const lang::String& typeName ) :
	GameScriptable( vm, arch, 0, 0 )
{
	setName( typeName );
}
