#ifndef _GAMEWEAPON_H
#define _GAMEWEAPON_H


#include "GameObject.h"
#include <pix/Colorf.h>


namespace io {
	class InputStreamArchive;}

namespace math {
	class Vector3; }

namespace sg {
	class Camera;
	class Light;
	class LOD; 
	class Mesh;
	class Node;
	class Light;
	class Primitive; }

namespace sgu {
	class SceneManager; }

class GameCell;
class GameCharacter;
class ProjectileManager;


/** 
 * Weapon used by characters in the game.
 * @see GameProjectile
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameWeapon :
	public GameObject
{
public:
	enum FireMode
	{
		FIRE_AUTO,
		FIRE_SINGLE,
	};

	GameWeapon( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr,
			sgu::SceneManager* sceneMgr, ProjectileManager* projectileMgr, GameNoiseManager* noiseMgr );

	~GameWeapon();

	sg::Node*			getRenderObject( sg::Camera* camera );
	void				update( float dt );
	void				setShellsRemaining( int val );
	void				fire( GameCharacter* shooter, const math::Vector3& direction, bool applyRecoilError=true );
	void				ejectShell();
	void				looseClip();
	void				reload();
	void				setOwner( GameCharacter* character );
	void				setFireRate( float rate );

	GameCharacter*		owner() const;
	sg::Node*			mesh() const;
	bool				ready() const;
	const lang::String&	bullet() const;
	float				fireRate() const;
	int					shellsPerClip() const;
	int					shellsRemaining() const;
	int					shotsPerLaunch() const;
	float				spreadConeLimitAngle() const;
	
	/** Returns true if the weapon has been 'clicked' after running out of ammo. */
	bool				clipEmpty() const;

	FireMode			fireMode() const;
	float				accumulatedRecoilError() const;
	float				recoilErrorPerShot() const;
	float				recoilErrorCorrection() const;
	float				maxRecoilError() const;
	float				minRecoilError() const;

	/** Returns world space bullet emit position. */
	math::Vector3		launchPosition() const;

	/** Returns key light (if any) information for this object. */
	virtual sg::Light*	keylight() const;

private:
	// Visuals
	P(sg::Node)				m_mesh;
	P(sgu::SceneManager)	m_sceneMgr;
	GameCharacter*			m_owner;
	pix::Colorf				m_groundLightmapColor;
	ProjectileManager*		m_projectileMgr;

	// Simulation
	lang::String	m_bullet;
	lang::String	m_emptyShell;
	float			m_fireRate;
	float			m_fireDeltaTime;
	float			m_shellEjectDelay;
	float			m_shellEjectTime;
	bool			m_shellInChamber;
	int				m_shells;
	int				m_shellsPerClip;
	int				m_shotsPerLaunch;
	float			m_spreadConeLimitAngle;
	bool			m_clipEmpty;
	FireMode		m_fireMode;

	// Error
	float			m_accumulatedRecoilError;
	float			m_recoilErrorPerShot;
	float			m_recoilErrorCorrection;
	float			m_maxRecoilError;
	float			m_minRecoilError;


	// Visuals effects (particles, shell projectiles, sounds) of firing a weapon
	void			fireVisuals( GameCharacter* shooter );

	// Randomly perturbs a vector
	math::Vector3	perturbVector( const math::Vector3& src, float limit, const math::Vector3& up, const math::Vector3& right );

	// Shader parameters
	void	updateShaderParameters();
	void	setShaderParams( sg::Shader* fx, sg::Mesh* mesh, sg::Light* keylight );
	
	// Script functions
	int									m_methodBase;
	static ScriptMethod<GameWeapon>		sm_methods[];

	int		methodCall( script::VM* vm, int i );
	int		script_getShellsPerClip( script::VM* vm, const char* funcName );
	int		script_fireAt( script::VM* vm, const char* funcName );
	int		script_fireWithoutBullet( script::VM* vm, const char* funcName );
	int		script_getShellsRemaining( script::VM* vm, const char* funcName );
	int		script_owner( script::VM* vm, const char* funcName );
	int		script_reload( script::VM* vm, const char* funcName );
	int		script_setMesh( script::VM* vm, const char* funcName );
	int		script_setBullet( script::VM* vm, const char* funcName );
	int		script_setEmptyShell( script::VM* vm, const char* funcName );
	int		script_setFireMode( script::VM* vm, const char* funcName );
	int		script_setFireRate( script::VM* vm, const char* funcName );
	int		script_setShellEjectDelay( script::VM* vm, const char* funcName );
	int		script_setShellsPerClip( script::VM* vm, const char* funcName );
	int		script_setShellsRemaining( script::VM* vm, const char* funcName );
	int		script_setShotsPerLaunch( script::VM* vm, const char* funcName );
	int		script_setSpreadConeAngle( script::VM* vm, const char* funcName );
	int		script_setRecoilErrorPerShot( script::VM* vm, const char* funcName );
	int		script_setRecoilErrorCorrectionPerSec( script::VM* vm, const char* funcName );
	int		script_setRecoilErrorMax( script::VM* vm, const char* funcName );
	int		script_setRecoilErrorMin( script::VM* vm, const char* funcName );

	GameWeapon( const GameWeapon& );
	GameWeapon& operator=( const GameWeapon& );
};


#endif // _GAMEWEAPON_H
