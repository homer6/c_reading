#ifndef _PS_PATHPARTICLESYSTEM_H
#define _PS_PATHPARTICLESYSTEM_H


#include <ps/SpriteParticleSystem.h>


namespace ps
{


class Shape;
class Force;


/**
 * Particle system where particles (2D sprites) follow specified path.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PathParticleSystem :
	public SpriteParticleSystem
{
public:
	/** Creates a default particle system. */
	PathParticleSystem();

	/** Creates a copy of other particle system. */
	PathParticleSystem( const PathParticleSystem& other );

	///
	~PathParticleSystem();

	/** 
	 * Updates particle system. Call base class implementation if overriden. 
	 * Always call the base class implementation if you override this in derived classes.
	 */
	void		update( float dt );

	/**
	 * Adds a new particle to the system.
	 * Always call the base class implementation if you override this in derived classes.
	 * Derived classes can alter the parameters passed in to the function
	 * when calling the base implementation.
	 */
	void		addParticle( const math::Vector3& pos, const math::Vector3& vel, float lifeTime );

	/**
	 * Removes ith particle from the system.
	 * Always call the base class implementation if you override this in derived classes.
	 */
	void		removeParticle( int index );

	/** Sets minimum particle speed (units/second). */
	void		setParticleMinSpeed( float v );

	/** Sets maximum particle speed (units/second). */
	void		setParticleMaxSpeed( float v );

	/** Set start scale of the particles. */
	void		setParticleStartScale( float value );

	/** Set end scale of the particles. */
	void		setParticleEndScale( float value );

	/** Sets number of blended paths. Default is 1. */
	void		setPaths( int paths );

	/** Adds a point to the path. */
	void		addPathPoint( float time, const math::Vector3& pos, int path=0 );

	/** Enables/disables random path selection. Default is true. */
	void		setRandomPathSelection( bool enabled );

	/** Returns number of points in the path. */
	int			getPathPoints( int path ) const;

	/** Returns number of blended paths. Default is 1. */
	int			paths() const;

	/** Returns minimum particle speed (units/second). */
	float		particleMinSpeed() const;

	/** Returns maximum particle speed (units/second). */
	float		particleMaxSpeed() const;

	/** Returns start scale of the particles. */
	float		particleStartScale() const;

	/** Returns end scale of the particles. */
	float		particleEndScale() const;

	/** Returns true if random path selection is enabled. */
	bool		randomPathSelection() const;

private:
	class PathParticleSystemImpl;
	P(PathParticleSystemImpl) m_this;

	void		defaults();
	bool		getParticlePosition( int index, float time, float* v3 ) const;

	PathParticleSystem& operator=( const PathParticleSystem& );
};


} // ps


#endif // _PS_PATHPARTICLESYSTEM_H
