#ifndef _GAMELEVEL_H
#define _GAMELEVEL_H


#include "GameScriptable.h"
#include "GameWeapon.h"
#include "GameCharacter.h"
#include "GameFlareSet.h"
#include "GameCell.h"
#include "GamePath.h"
#include "GameBoxTrigger.h"
#include "GameDynamicObject.h"
#include <bsp/BSPTree.h>
#include <pix/Color.h>
#include <sgu/NodeGroupSet.h>
#include <util/Vector.h>
#include <util/Hashtable.h>
#include <math/Vector3.h>


namespace lang {
	class String; }

namespace script {
	class VM; }

namespace io {
	class InputStreamArchive; }

namespace music {
	class MusicManager; }

namespace sg {
	class LineList;
	class Camera;
	class Effect;
	class Node; }

namespace sgu {
	class SceneManager; }

namespace snd {
	class SoundManager; }

namespace ps {
	class ParticleSystemManager; }

class GameCamera;
class GameCutScene;
class GameCharacter;
class GameCell;
class GameObject;
class GameWeapon;
class GameNoiseManager;
class ProjectileManager;


/** 
 * Game level contains cells, portals and objects.
 * @author Jani Kajala (jani.kajala@helsinki.fi), Toni Aittoniemi
 */
class GameLevel :
	public GameScriptable
{
public:
	/** 
	 * Creates level with scripting support. 
	 * @param bspBuildPolySkip Quality of BSP when building one. Lower value is better, 0 best.
	 */
	explicit GameLevel( script::VM* vm, io::InputStreamArchive* arch, 
		snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr, 
		sgu::SceneManager* sceneMgr, music::MusicManager* musicMgr,
		ProjectileManager* projectileMgr, GameNoiseManager* noiseMgr,
		int bspBuildPolySkip );

	///
	~GameLevel();

	/** Updates level. */
	void			update( float dt );

	/** Removes all non-main characters from the level. Debug feature. */
	void			removeNonMainCharacters();

	/** Skips any active cut scene. */
	void			skipCutScene();

	/** Removes weapon from the level. */
	void			removeWeapon( GameWeapon* weapon );

	/** Camera informs level if view changes instantly so flares can be updated by the change. */
	void			signalViewChanged();

	/** Returns number of cells. */
	int				cells() const;

	/** Returns number of characters. */
	int				characters() const;

	/** Returns number of triggers. */
	int				triggers() const;

	/** Returns number of guard paths. */
	int				paths() const;

	/** Returns number of dynamic objects. */
	int				dynamicObjects() const;

	/** Returns cell by index. */
	GameCell*		getCell( int index ) const;

	/** Returns cell by name. */
	GameCell*		getCell( const lang::String& name ) const;

	/** Retrieves character by index. */
	GameCharacter*	getCharacter( int index ) const;

	/** Retrieves character by name. */
	GameCharacter*	getCharacter( const lang::String& name ) const;

	/** Retrieves trigger by index. */
	GameBoxTrigger*	getTrigger( int index ) const;

	/** Returns ith guard path. */
	GamePath*		getPath( int i ) const;

	/** Returns guard path by name. */
	GamePath*		getPath( const lang::String& name ) const;

	/** Returns ith dynamic object. */
	GameDynamicObject* getDynamicObject( int i ) const;

	/** Returns dynamic object by name. */
	GameDynamicObject* getDynamicObject( const lang::String& name ) const;

	/** Retrieves main character. */
	GameCharacter*	mainCharacter() const;

	/** Returns default collision material in this level. */
	GameSurface*	defaultCollisionMaterialType() const;

	/** Retrieves weapon by index. */
	GameWeapon*		getWeapon( int index ) const;

	/** Returns number of weapons. */
	int				weapons() const;

	/** Returns true if cut scene is active. */
	bool			isActiveCutScene() const;

	/** Returns active cut scene. */
	GameCutScene*	activeCutScene() const;

	/** Returns stencil shadow color. */
	pix::Color		shadowColor() const;

	/** Returns true if level ended. */
	bool			ended() const;

	/** Returns ith flare set. */
	GameFlareSet*	getFlareSet( int i ) const										{return m_flareSetList[i];}

	/** Returns number of flare sets. */
	int				flareSets() const												{return m_flareSetList.size();}

private:
	enum NodeClass
	{
		NODECLASS_NORMAL,
		NODECLASS_CELL,
		NODECLASS_PORTAL,
		NODECLASS_PATH,
		NODECLASS_CAMERA,
		NODECLASS_BACKGROUND,
	};

	// scripting
	int								m_methodBase;
	static ScriptMethod<GameLevel>	sm_methods[];
	
	// Interfaces
	P(script::VM)					m_vm;
	P(io::InputStreamArchive)		m_arch;
	P(snd::SoundManager)			m_soundMgr;
	P(ps::ParticleSystemManager)	m_particleMgr;
	P(sgu::SceneManager)			m_sceneMgr;
	P(music::MusicManager)			m_musicMgr;
	P(ProjectileManager)			m_projectileMgr;
	P(GameNoiseManager)				m_noiseMgr;
	bool							m_levelEnded;

	// Geometry
	P(sgu::NodeGroupSet)			m_animSet;
	util::Vector<P(GameCell)>		m_cells;
	int								m_bspBuildPolySkip;
	P(GameSurface)					m_defaultCollisionMaterialType;
	util::Vector<P(GameSurface)>	m_collisionMaterialTypes;
	pix::Color						m_shadowColor;
	util::Vector<P(sg::Node)>		m_backgrounds;
	P(sg::Effect)					m_lightmapShader;

	// Object lists 
	util::Vector<P(GameBoxTrigger)>		m_triggerList;
	util::Vector<P(GameCharacter)>		m_characterList;
	util::Vector<P(GameWeapon)>			m_weaponList;
	util::Vector<P(GamePath)>			m_pathList;
	util::Vector<P(GameDynamicObject)>	m_dynamicObjectList;
	util::Hashtable<lang::String,P(bsp::BSPTree)>	m_dynamicObjectBSPs;
	util::Vector<P(GameFlareSet)>		m_flareSetList;
	P(GameCharacter)					m_mainCharacter;
	P(GameCutScene)						m_cutScene;
	util::Vector<P(GameObject)>			m_removed;

	/** Helper func for removing object from cell and adding it to m_removed list for to be removed in next update. */
	void				removeObject( GameObject* obj );

	/** Loads cells from a level scene file. */
	void				loadFile( const lang::String& filename );

	/** Create a dynamic object to the level. */
	GameDynamicObject*	createDynamicObject( sg::Node* node, const lang::String& bspFileName, const math::Matrix4x4& tm, GameCell* cell );

	/** Returns node classification by name tags. */
	static NodeClass	getNodeClass( sg::Node* node );	

	/** Removes _001 suffix from string if any. */
	static lang::String removeEndNumberSuffix( const lang::String& str );

	/**
	 * When a (C++) method is called from a script, this function
	 * is executed and unique method identifier is passed as parameter.
	 * Derived classes must override this if they add new scriptable methods.
	 * @param vm Script virtual machine executing the method.
	 * @param i Unique identifier (index) of the called method.
	 * @return Number of arguments returned in the script stack.
	 */
	virtual int		methodCall( script::VM* vm, int i );

	int		script_createNoise( script::VM* vm, const char* funcName );
	int		script_createBoxTrigger( script::VM* vm, const char* funcName );
	int		script_createCharacter( script::VM* vm, const char* funcName );
	int		script_createWeapon( script::VM* vm, const char* funcName );
	int		script_createFlareSet( script::VM* vm, const char* funcName );
	int		script_endLevel( script::VM* vm, const char* funcName );
	int		script_getCell( script::VM* vm, const char* funcName );
	int		script_getPath( script::VM* vm, const char* funcName );
	int		script_getCharacter( script::VM* vm, const char* funcName );
	int		script_getDynamicObject( script::VM* vm, const char* funcName );
	int		script_loadDynamicObjects( script::VM* vm, const char* funcName );
	int		script_loadProjectiles( script::VM* vm, const char* funcName );
	int		script_isActiveCutScene( script::VM* vm, const char* funcName );
	int		script_importGeometry( script::VM* vm, const char* funcName );
	int		script_playCutScene( script::VM* vm, const char* funcName );
	int		script_setBackgroundToCells( script::VM* vm, const char* funcName );
	int		script_setShadowColor( script::VM* vm, const char* funcName );
	int		script_setMainCharacter( script::VM* vm, const char* funcName );
	int		script_signalExplosion( script::VM* vm, const char* funcName );
	int		script_skipCutScene( script::VM* vm, const char* funcName );
	int		script_removeTrigger( script::VM* vm, const char* funcName );
	int		script_removeWeapon( script::VM* vm, const char* funcName );
	int		script_removeCharacter( script::VM* vm, const char* funcName );
	int		script_removeDynamicObjects( script::VM* vm, const char* funcName );

	GameLevel( const GameLevel& other );
	GameLevel& operator=( const GameLevel& other );
};


#endif // _GAMELEVEL_H
