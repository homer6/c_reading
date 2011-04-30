#ifndef _GAMEDYNAMICOBJECT_H
#define _GAMEDYNAMICOBJECT_H


#include "GameObject.h"
#include "GameSurface.h"
#include <util/Vector.h>


namespace sg {
	class Shader;
	class Light;
	class Mesh;}

namespace sgu {
	class SceneManager;}

namespace bsp {
	class BSPTree;
	class BSPNode;}

class GameBSPTree;
class GamePointObject;
class ProjectileManager;


/** 
 * Dynamic geometric object in game level. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameDynamicObject :
	public GameObject
{
public:
	GameDynamicObject( script::VM* vm, io::InputStreamArchive* arch, 
		sgu::SceneManager* sceneMgr, snd::SoundManager* soundMgr, 
		ps::ParticleSystemManager* particleMgr, ProjectileManager* projectileMgr, GameNoiseManager* noiseMgr,
		sg::Node* geometry,	const lang::String& bspFileName, int bspBuildPolySkip,
		const util::Vector<P(GameSurface)>& collisionMaterialTypes, bsp::BSPTree* cachedBSPTree );

	/** Update dynamic object, */
	void			update( float dt );

	/** Returns object to be used in rendering. */
	sg::Node*		getRenderObject( sg::Camera* camera );

	/** Returns dynamic object collision BSP tree. */
	GameBSPTree*	bspTree() const;

private:
	int										m_methodBase;
	static ScriptMethod<GameDynamicObject>	sm_methods[];

	P(sgu::SceneManager)					m_sceneMgr;
	P(sg::Node)								m_geometry;
	ProjectileManager*						m_projectileMgr;
	P(GameBSPTree)							m_bsptree;
	P(GamePointObject)						m_visibilityChecker;
	math::Vector3							m_spinAxis;
	float									m_spinSpeed;

	// key frame animation
	P(sg::Node)								m_worldAnim;
	float									m_worldAnimTime;

	bool	isVisibleFrom( const math::Vector3& worldPos ) const;
	bool	isVisiblePoint( const math::Vector3& worldPos, const math::Vector3& point ) const;
	void	applyWorldSpaceAnimations();
	void	setShaderParams( sg::Shader* fx, sg::Mesh* mesh, sg::Light* keylight );

	int		methodCall( script::VM* vm, int i );
	int		script_playWorldSpaceAnimation( script::VM* vm, const char* funcName );
	int		script_setSpin( script::VM* vm, const char* funcName );
	int		script_stopWorldSpaceAnimation( script::VM* vm, const char* funcName );

	GameDynamicObject( const GameDynamicObject& );
	GameDynamicObject& operator=( const GameDynamicObject& );
};


#endif // _GAMEDYNAMICOBJECT_H

