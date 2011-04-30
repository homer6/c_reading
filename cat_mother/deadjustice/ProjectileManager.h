#ifndef _PROJECTILEMANAGER_H
#define _PROJECTILEMANAGER_H


#include "GameProjectile.h"
#include <lang/String.h>
#include <lang/Object.h>
#include <util/Vector.h>
#include <util/Hashtable.h>


namespace math {
	class Vector3; }

namespace io {
	class InputStreamArchive; }

namespace ps {
	class ParticleSystemManager; }

namespace script {
	class VM; }

namespace sgu {
	class SceneManager; }

namespace snd {
	class SoundManager; }

class GameCell;
class GameWeapon;


/**
 * <ul>
 * <li>Manages creation and removal of "GameProjectile"-type objects
 * <li>ProjectileManager caches GameProjectiles and does not reallocate memory needlessly
 * <li>On creation GameProjectiles are put into cell, GameProjectile::launch method is called and the object is put in to the "used" array
 * <li>Projectiles that have marked themselves as to be removed will be removed from Cells by ProjectileManager and moved to the "free" array
 * <li>GameProjectiles are classified according to their script filename
 * <li>ProjectileManager precompiles GameProjectile scripts with an allocation method to allocate (n) projectiles when needed
 * </ul>
 * @author Toni Aittoniemi, Jani Kajala (jani.kajala@helsinki.fi)
 */
class ProjectileManager :
	public lang::Object
{
public:
	enum maxprojectiles
	{
		MAXPROJECTILES = 150
	};
	
	ProjectileManager( script::VM* vm, io::InputStreamArchive* arch, snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr, sgu::SceneManager* sceneMgr, GameNoiseManager* noiseMgr );
	~ProjectileManager();

	/** Update all projectiles. */
	void	update( float dt );

	/** Use a projectile from "free" array / create a new projectile / delete old projectile and reset. */
	GameProjectile*	createProjectile( const lang::String& scriptFileName, GameCell* cell, GameWeapon* weapon, const math::Vector3& position, const math::Vector3& direction );

	/** Allocates 'count' projectiles, compiles scripts and adds to cache. */
	void	allocateProjectiles( const lang::String& scriptFileName, int count );

	/** Removes projectiles owner by specific weapon. */
	void	removeWeaponProjectiles( GameWeapon* weapon );

	/** Returns count of used projectiles per script. */
	int		numProjectiles( const lang::String& script ) const;

	/** projectile positions, will not copy more than positions.size() */
	void	getProjectiles( const lang::String& script, util::Vector<P(GameProjectile)>& projectiles ) const;

private:
	struct Projectiles
	{	
		util::Vector< P(GameProjectile) > used;
		util::Vector< P(GameProjectile) > free;

		Projectiles();
	};

	P(script::VM)					m_vm;
	P(io::InputStreamArchive)		m_arch;
	P(snd::SoundManager)			m_soundMgr;
	P(ps::ParticleSystemManager)	m_particleMgr;
	P(sgu::SceneManager)			m_sceneMgr;
	P(GameNoiseManager)				m_noiseMgr;
	int								m_maxProjectilesPerClass;

	util::Hashtable< lang::String, Projectiles >	m_projectileTable;
};


#endif // _PROJECTILEMANAGER_H
