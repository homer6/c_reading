#ifndef _GAMECUTSCENE_H
#define _GAMECUTSCENE_H


#include "GameObject.h"
#include <sg/Node.h>
#include <util/Vector.h>


class ProjectileManager;


/** 
 * In-game cut scene description.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameCutScene :
	public GameScriptable
{
public:
	GameCutScene( script::VM* vm, io::InputStreamArchive* arch, 
		sgu::SceneManager* sceneMgr, snd::SoundManager* soundMgr, 
		ps::ParticleSystemManager* particleMgr, ProjectileManager* projectileMgr );

	~GameCutScene();

	/** Updates cut scene state. */
	void			update( float dt );

	/** Skips rest of the cut scene. */
	void			skip();

	/** Sets time of cut scene playback. */
	void			setTime( float time );

	/** Returns true if cut scene has ended. */
	bool			ended() const;

	/** Returns time of cut scene playback. */
	float			time() const;

	/** Returns cut scene camera front plane distance. */
	float			cameraFront() const;

	/** Returns cut scene camera back plane distance. */
	float			cameraBack() const;

private:
	class SceneObject
	{
	public:
		P(sg::Node)		obj;
		P(sg::Node)		parent;
	};

	static ScriptMethod<GameCutScene>	sm_methods[];
	int									m_methodBase;

	P(sgu::SceneManager)				m_sceneMgr;
	P(ProjectileManager)				m_projectileMgr;
	float								m_time;
	float								m_endTime;
	P(sg::Node)							m_scene;
	float								m_sceneTime;
	util::Vector<SceneObject>			m_sceneObjects;	// particles and sounds
	float								m_cameraFront;
	float								m_cameraBack;

	/** Stops all sounds and particles. */
	void	removeSceneObjects();

	int		methodCall( script::VM* vm, int i );
	int		script_endTime( script::VM* vm, const char* funcName );
	int		script_playScene( script::VM* vm, const char* funcName );
	int		script_setEndTime( script::VM* vm, const char* funcName );
	int		script_playObjectParticleSystem( script::VM* vm, const char* funcName );
	int		script_playObjectSound( script::VM* vm, const char* funcName );
	int		script_setCutSceneCameraFrontAndBackPlanes( script::VM* vm, const char* funcName );
	int		script_time( script::VM* vm, const char* funcName );

	GameCutScene( const GameCutScene& );
	GameCutScene& operator=( const GameCutScene& );
};


#endif // _GAMECUTSCENE_H
