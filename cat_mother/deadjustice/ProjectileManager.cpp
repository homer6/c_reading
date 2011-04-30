#include "ProjectileManager.h"
#include "GameProjectile.h"
#include "GameWeapon.h"
#include "GameNoiseManager.h"
#include "GameCell.h"
#include <io/InputStreamArchive.h>
#include <ps/ParticleSystemManager.h>
#include <lang/Exception.h>
#include <lang/Debug.h>
#include <sgu/SceneManager.h>
#include <snd/SoundManager.h>
#include <math/Vector3.h>
#include <script/VM.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;
using namespace util;

//-----------------------------------------------------------------------------

ProjectileManager::ProjectileManager( script::VM* vm, io::InputStreamArchive* arch, 
	snd::SoundManager* soundMgr, ps::ParticleSystemManager* particleMgr, sgu::SceneManager* sceneMgr, GameNoiseManager* noiseMgr ) :
	m_vm( vm ),
	m_arch( arch ),
	m_soundMgr( soundMgr ),
	m_particleMgr( particleMgr ),
	m_sceneMgr( sceneMgr ),
	m_noiseMgr( noiseMgr ),
	m_projectileTable( Allocator< HashtablePair< String, ProjectileManager::Projectiles > >(__FILE__) ),
	m_maxProjectilesPerClass( ProjectileManager::MAXPROJECTILES )
{
}

ProjectileManager::~ProjectileManager()
{
	for ( HashtableIterator< String, Projectiles > i = m_projectileTable.begin(); i != m_projectileTable.end(); ++i )
	{
		Projectiles& current = i.value();
		int p;

		for ( p = 0; p < current.used.size(); ++p )
			current.used[p] = 0;

		for ( p = 0; p < current.free.size(); ++p )
			current.free[p] = 0;
	}	
}

void ProjectileManager::update( float dt ) 
{
	for ( HashtableIterator< String, Projectiles > i = m_projectileTable.begin(); i != m_projectileTable.end(); ++i )
	{
		Projectiles& current = i.value();

		int p = 0;
		while( p < current.used.size() )
		{
			bool removed = false;
			if ( current.used[p]->shouldBeRemoved() )
			{
				current.used[p]->setPosition( 0, Vector3(0,0,0) );
				current.free.add( current.used[p] );
				current.used.remove(p);
				removed = true;
			}
			if ( !removed )
			{
				current.used[p]->update( dt );
				p++;
			}
		}
	}
}

GameProjectile* ProjectileManager::createProjectile( const lang::String& scriptFileName, GameCell* cell, GameWeapon* weapon, const math::Vector3& position, const math::Vector3& direction ) 
{
	Projectiles& current = m_projectileTable[scriptFileName];
	P(GameProjectile) projectile = 0;

	if ( current.free.size() > 0 )
	{
		// Use a free projectile
		int last = current.free.size() - 1;
		projectile = current.free[last];
		current.free.remove( last );
	}
	else if ( current.used.size() < m_maxProjectilesPerClass )
	{
		// Allocate new projectile
		projectile = new GameProjectile( this, m_sceneMgr, m_vm, m_arch, m_soundMgr, m_particleMgr, m_noiseMgr );
		projectile->compile( scriptFileName );
	}
	else
	{
		// Force re-use an old projectile, whatever the situation
		current.used[0]->setPosition( cell, position );
		current.used[0]->launch( direction);
		P(GameProjectile) projectile = current.used[0];
		current.used.remove( 0 );

		Debug::printlnWarning( "ProjectileManager was forced to re-use an existing projectile. Try increasing maximum projectile count." );
	}

	projectile->setPosition( cell, position );
	projectile->launch( direction );
	projectile->setWeapon( weapon );
	current.used.add( projectile );
	return projectile;
}

void ProjectileManager::allocateProjectiles( const lang::String& scriptFileName, int count ) 
{
	Projectiles& current = m_projectileTable[scriptFileName];
	P(GameProjectile) projectile = 0;
	
	for ( int i = 0; i < count; ++i )
	{
		projectile = new GameProjectile( this, m_sceneMgr, m_vm, m_arch, m_soundMgr, m_particleMgr, m_noiseMgr );
		projectile->compile( scriptFileName );
		current.free.add( projectile );		
	}
}

void ProjectileManager::removeWeaponProjectiles( GameWeapon* weapon )
{
	for ( HashtableIterator< String, Projectiles > i = m_projectileTable.begin(); i != m_projectileTable.end(); ++i )
	{
		Projectiles& current = i.value();

		for ( int p = 0 ; p < current.used.size() ; ++p )
		{
			GameProjectile* projectile = current.used[p];
			if ( projectile->weapon() == weapon )
				projectile->removeInNextUpdate();
		}
	}
}

int ProjectileManager::numProjectiles( const lang::String& script ) const 
{
	if ( !m_projectileTable.containsKey( script ) )
		return 0;

	return m_projectileTable[script].used.size();
}

void ProjectileManager::getProjectiles( const lang::String& script, util::Vector<P(GameProjectile)>& projectiles ) const 
{
	if ( !m_projectileTable.containsKey( script ) )
		return;
	
	int copycount = m_projectileTable[script].used.size() < projectiles.size() ? m_projectileTable[script].used.size() : projectiles.size();

	for ( int i = 0; i < copycount; ++i )
	{
		projectiles[i] = m_projectileTable[script].used[i];
	}
}

//-----------------------------------------------------------------------------

ProjectileManager::Projectiles::Projectiles() :
	used( Allocator< P(GameProjectile) >(__FILE__) ),
	free( Allocator< P(GameProjectile) >(__FILE__) )
{
}
