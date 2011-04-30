#ifndef _GAMESURFACE_H
#define _GAMESURFACE_H


#include "GameScriptable.h"


/** 
 * Collision material type. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameSurface :
	public GameScriptable
{
public:
	GameSurface( script::VM* vm, io::InputStreamArchive* arch, const lang::String& typeName );

private:
	GameSurface( const GameSurface& );
	GameSurface& operator=( const GameSurface& );
};


#endif // _GAMESURFACE_H
