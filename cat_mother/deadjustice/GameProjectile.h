#ifndef _GAMEPROJECTILE_H
#define _GAMEPROJECTILE_H


#include "GameObject.h"
#include "ScriptUtil.h"
#include <pix/Colorf.h>


namespace math {
	class Vector3; }

namespace io {
	class InputStreamArchive; }

namespace ps {
	class ParticleSystemManager; }

namespace script {
	class VM; }

namespace sg {
	class Camera;
	class Light;
	class Mesh;
	class Node; 
	class Primitive; 
	class Shader; }

namespace sgu {
	class SceneManager; }

namespace snd {
	class SoundManager; }


class CollisionInfo;
class ProjectileManager;
class GameCharacter;
class GameWeapon;


/** 
 * Bullet or shell game object. Created through ProjectileManager. 
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameProjectile :
	public GameObject
{
public:
	GameProjectile( ProjectileManager* projectileMgr, sgu::SceneManager* sceneMgr, script::VM* vm, 
		io::InputStreamArchive* arch, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr,
		GameNoiseManager* noiseMgr );

	/** Returns graphics engine object to be rendered. */
	sg::Node*	getRenderObject( sg::Camera* camera );

	/** 
	 * Checks collision of this object against BSP tree. 
	 * Updates CollisionInfo only if collision happens.
	 */
	void	checkCollisionsAgainstCell( const math::Vector3& start, const math::Vector3& delta, bsp::BSPNode* bsptree, GameCell* collisionCell, CollisionInfo* cinfo );

	/** 
	 * Checks collisions against dynamic objects. 
	 * Updates CollisionInfo only if collision happens.
	 */
	void	checkCollisionsAgainstObjects( const math::Vector3& start, const math::Vector3& delta, const util::Vector<GameObjectDistance>& objects, CollisionInfo* cinfo );

	/** Update projectile movement. */
	void	update( float dt );
		
	/** Launch projectile. */
	void	launch( const math::Vector3& direction );

	/** Sets this projectile to be removed in next update. */
	void	removeInNextUpdate();

	/** Sets light color. */
	void	setGroundLightmapColor( const pix::Colorf& color );

	/** Sets weapon that fired the projectile. */
	void	setWeapon( GameWeapon* weapon );

	/** Returns true if projectile should be removed. */
	bool	shouldBeRemoved() const;

	/** Returns amount how much this projectile makes damage. */
	float	damage() const;

	/** Returns key light (if any) information for this object. */
	virtual sg::Light*	keylight() const;

	/** Returns weapon associated with this projectile. */
	GameWeapon*			weapon() const;

private:
	int									m_methodBase;
	static ScriptMethod<GameProjectile>	sm_methods[];

	ProjectileManager*				m_projectileMgr;
	sgu::SceneManager*				m_sceneMgr;	
	P(sg::Node)						m_mesh;
	pix::Colorf						m_groundLightmapColor;
	P(GameWeapon)					m_weapon;

	math::Vector3					m_spinAxis;
	float							m_spinSpeed;
	bool							m_removable;
	float							m_damage;
	float							m_launchVelocity;
	float							m_age;
	float							m_ageLimit;
	float							m_gravity;
	float							m_damageAttenuationRange;
	float							m_maxRange;
	float							m_movedDistance;
	bool							m_keepOnCollision;
	bool							m_alignOnCollision;
	bool							m_hitCharacter;
	bool							m_hasHit;
	lang::String					m_lastBoneHit;

	// collisions
	void	hit( CollisionInfo* cinfo );
	void	hitGeometry( CollisionInfo* cinfo );
	void	hitCharacter( CollisionInfo* cinfo, GameCharacter* character );

	// Shader parameters
	void	updateShaderParameters( sg::Camera* camera );
	void	setShaderParams( sg::Primitive* prim, sg::Shader* fx, sg::Mesh* mesh, sg::Camera* camera, sg::Light* keylight );

	// Scripting
	int		methodCall( script::VM* vm, int i );
	int		script_damage( script::VM* vm, const char* funcName );
	int		script_enableKeepOnCollision( script::VM* vm, const char* funcName );
	int		script_enableAlignOnCollision( script::VM* vm, const char* funcName );
	int		script_enableHitCharacter( script::VM* vm, const char* funcName );
	int		script_getWeapon( script::VM* vm, const char* funcName );
	int		script_setAgeLimit( script::VM* vm, const char* funcName );
	int		script_setDamage( script::VM* vm, const char* funcName );
	int		script_setGravity( script::VM* vm, const char* funcName );
	int		script_setLaunchVelocity( script::VM* vm, const char* funcName );
	int		script_setDamageAttenuationStartRange( script::VM* vm, const char* funcName );
	int		script_setMaxRange( script::VM* vm, const char* funcName );
	int		script_setMesh( script::VM* vm, const char* funcName );
	int		script_setSpin( script::VM* vm, const char* funcName );

	GameProjectile( const GameProjectile& other );
	GameProjectile& operator= ( const GameProjectile& other );
};


#endif // _GAMEPROJECTILE_H
