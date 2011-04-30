#include "GamePath.h"
#include "ScriptUtil.h"
#include <lang/Float.h>
#include <lang/Error.h>
#include <script/ScriptException.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;
using namespace script;

//-----------------------------------------------------------------------------

ScriptMethod<GamePath> GamePath::sm_methods[] =
{
	//ScriptMethod<GamePath>( "funcName", script_funcName ),
	ScriptMethod<GamePath>( "getClosestPointIndex", script_getClosestPointIndex ),
	ScriptMethod<GamePath>( "getPointPosition", script_getPointPosition ),
	ScriptMethod<GamePath>( "points", script_points ),
};

//-----------------------------------------------------------------------------

GamePath::GamePath( VM* vm, io::InputStreamArchive* arch, const String& name ) :
	GameScriptable( vm, arch, 0, 0 ),
	m_methodBase( -1 ),
	m_points( Allocator<Vector3>(__FILE__) )
{
	m_methodBase = ScriptUtil<GamePath,GameScriptable>::addMethods( this, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );

	setName( name );
}
	
void GamePath::addPoint( const Vector3& pt )
{
	m_points.add( pt );
}

int GamePath::getClosestPointIndex( const Vector3& pt ) const
{
	int closeIndex = -1;
	float closeDistSqr = Float::MAX_VALUE;

	for ( int i = 0 ; i < m_points.size() ; ++i )
	{
		float distSqr = (m_points[i] - pt).lengthSquared();
		if ( distSqr < closeDistSqr )
		{
			closeIndex = i;
			closeDistSqr = distSqr;
		}
	}

	if ( closeIndex == -1 )
		throw Error( Format("Failed to find closest point to AI guard path {0}", name()) );
	return closeIndex;
}

const Vector3& GamePath::getPoint( int i ) const
{
	assert( i >= 0 && i < points() );

	if ( i < 0 || i >= m_points.size() )
		throw Error( Format("Invalid index while using AI guard path {0}", name()) );
	return m_points[i];
}

int	GamePath::points() const
{
	return m_points.size();
}

int GamePath::methodCall( VM* vm, int i ) 
{
	return ScriptUtil<GamePath,GameScriptable>::methodCall( this, vm, i, m_methodBase, sm_methods, sizeof(sm_methods)/sizeof(sm_methods[0]) );
}

int GamePath::script_getClosestPointIndex( VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects x,y,z in world space, returns closest path point index (one-based)", funcName) );
	
	float x = vm->toNumber(1);
	float y = vm->toNumber(2);
	float z = vm->toNumber(3);

	int closest = getClosestPointIndex( Vector3(x,y,z) );
	vm->pushNumber( (float)(closest+1) );
	return 1;
}

int GamePath::script_getPointPosition( VM* vm, const char* funcName ) 
{
	int tags[] = {VM::TYPE_NUMBER, VM::TYPE_NUMBER};
	if ( !hasParams(tags,sizeof(tags)/sizeof(tags[0])) )
		throw ScriptException( Format("{0} expects point and axis indices (point index one-based, axis index zero-based)", funcName) );
	
	int point = getIndex( vm, funcName, 1, points()+1, 1 ) - 1;
	int axis = getIndex( vm, funcName, 0, 3, 2 );

	vm->pushNumber( getPoint(point)[axis] );
	return 1;
}

int GamePath::script_points( VM* vm, const char* funcName ) 
{
	if ( vm->top() != 0 )
		throw ScriptException( Format("{0} returns number of points in path", funcName) );
	vm->pushNumber( (float)points() );
	return 1;
}
