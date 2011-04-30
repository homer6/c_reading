#ifndef _GAMEPATH_H
#define _GAMEPATH_H


#include "GameScriptable.h"
#include <lang/String.h>
#include <util/Vector.h>
#include <math/Vector3.h>


/** 
 * AI guard path. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GamePath :
	public GameScriptable
{
public:
	explicit GamePath( script::VM* vm, io::InputStreamArchive* arch, const lang::String& name );
	
	void					addPoint( const math::Vector3& pt );

	int						getClosestPointIndex( const math::Vector3& pt ) const;
	const math::Vector3&	getPoint( int i ) const;
	int						points() const;

private:
	int									m_methodBase;
	static ScriptMethod<GamePath>		sm_methods[];

	util::Vector<math::Vector3>	m_points;

	int		methodCall( script::VM* vm, int i );
	int		script_getClosestPointIndex( script::VM* vm, const char* funcName );
	int		script_getPointPosition( script::VM* vm, const char* funcName );
	int		script_points( script::VM* vm, const char* funcName );

	GamePath( const GamePath& other );
	GamePath& operator=( const GamePath& other );
};


#endif // _GAMEPATH_H
