#ifndef _GAMEFLARESET_H
#define _GAMEFLARESET_H


#include "GameFlare.h"
#include "GameObject.h"
#include <sg/Node.h>
#include <sg/TriangleList.h>
#include <util/Vector.h>


namespace sg {
	class Camera;
	class Mesh;}

class GamePointObject;


/** 
 * Set of flares drawn on top of geometry.
 * Visibility checked by raytracing.
 * Fades in/out after visibility state changes.
 */
class GameFlareSet :
	public GameObject
{
public:
	GameFlareSet( script::VM* vm, io::InputStreamArchive* arch, const lang::String& imageName, 
		float worldSize, float fadeTime, int maxFlares );

	void		update( float dt );
	sg::Node*	getRenderObject( sg::Camera* camera );

	GameFlare&	getFlare( int i )													{return m_flares[i];}

	int			flares() const														{return m_flares.size();}

private:
	int									m_methodBase;
	static ScriptMethod<GameFlareSet>	sm_methods[];

	P(GamePointObject)				m_tracer;
	P(sg::Mesh)						m_mesh;
	P(sg::TriangleList)				m_tri;
	float							m_worldSize;
	float							m_fadeTime;
	int								m_maxFlares;
	float							m_dt;
	util::Vector<GameFlare>			m_flares;
	int								m_nextUpdated;

	int			methodCall( script::VM* vm, int i );
	int			script_addFlare( script::VM* vm, const char* funcName );

	GameFlareSet( const GameFlareSet& );
	GameFlareSet& operator=( const GameFlareSet& );
};


#endif // _GAMEFLARESET_H
