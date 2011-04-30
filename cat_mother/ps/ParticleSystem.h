#ifndef _PS_PARTICLESYSTEM_H
#define _PS_PARTICLESYSTEM_H


#include <sg/Node.h>


namespace sg {
	class Camera;}

namespace pix {
	class Image;}

namespace lang {
	class String;}

namespace math {
	class Vector3;
	class Matrix4x4;}


namespace ps
{


class Shape;
class Force;


/**
 * Abstract base class for all particle systems. 
 * All time related units are seconds and distance related units meters.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ParticleSystem :
	public sg::Node
{
public:
	/** Particle kill selection after maximum count has been reached. */
	enum KillType
	{
		/** No new particles get created when maximum count is exceeded. */
		KILL_NOTHING,
		/** Oldest particles die when maximum count is exceeded. */
		KILL_OLDEST,
		/** Random particles die when maximum count is exceeded. */
		KILL_RANDOM
	};

	/** Creates a default particle system. */
	ParticleSystem();

	/** Creates a copy of other particle system. */
	ParticleSystem( const ParticleSystem& other );

	///
	~ParticleSystem();

	/** 
	 * Updates particle system. Call base class implementation if overriden. 
	 * Always call the base class implementation if you override this in derived classes.
	 */
	virtual void		update( float dt );

	/** Returns a clone of current particle system. */
	virtual sg::Node*	clone() const = 0;

	/** Renders all particles to active device. */
	virtual void		render( sg::Camera* camera, int pass ) = 0;

	/** Computes object visibility in the view frustum. */
	virtual bool		updateVisibility( sg::Camera* camera );

	/** 
	 * Resets the system to initial state. Removes all particles.
	 * Always call the base class implementation if you override this in derived classes.
	 */
	virtual void		reset();

	/**
	 * Adds a new particle to the system. The new particle is the last particle.
	 * Always call the base class implementation if you override this in derived classes.
	 * Derived classes can alter the parameters passed in to the function
	 * when calling the base implementation.
	 */
	virtual void		addParticle( const math::Vector3& pos, const math::Vector3& vel, float lifeTime );

	/**
	 * Removes ith particle from the system.
	 * Always call the base class implementation if you override this in derived classes.
	 */
	virtual void		removeParticle( int index );

	/** Kills particle system so that alive() will return false. */
	void	kill();

	/** 
	 * Sets particle initial position source shape. 
	 * Particle initial positions are randomized inside this shape.
	 */
	void	setPositionShape( Shape* shape );

	/** 
	 * Sets particle initial velocity source shape. 
	 * Particle initial velocities are randomized inside this shape.
	 */
	void	setVelocityShape( Shape* shape );

	/** Sets particle emission rate. */
	void	setEmissionRate( float value );

	/** Sets particle life time. */
	void	setParticleLifeTime( float value );

	/** Sets the system life time. */
	void	setSystemLifeTime( float value );

	/** Sets maximum number of simultanuous particles. */
	void	setMaxParticles( int count );

	/** Sets particle kill selection after maximum count has been reached. */
	void	setKillType( KillType kill );

	/** Adds a force affecting the particle system. */
	void	addForce( Force* force );

	/** Set particles to be emitted to local space. Default is false. */
	void	setLocalSpace( bool enabled );

	/** Sets time when the emission stops. */
	void	setEmissionTime( float t );

	/** Sets minimum speed when any particles are emitted. */
	void	setMinEmissionSpeed( float t );

	/** Sets speed when maximum number of particles are emitted. Default is 0. */
	void	setMaxEmissionSpeed( float t );

	/** Sets bound sphere radius. */
	void	setBoundSphere( float r );

	/** Returns bound sphere radius. */
	float	boundSphere() const;

	/** Returns number of particles in the system. */
	int		particles() const;

	/** Returns maximum number of particles in the system. */
	int		maxParticles() const;

	/** Returns true if particles are emitted to local space.  */
	bool	localSpace() const;

	/** Returns current time of particle system simulation. */
	float	time() const;

	/** Returns particle system life time. */
	float	systemLifeTime() const;

	/** Returns true if the particle system is still alive. */
	bool	alive() const;

	/** Returns particle positions. */
	const math::Vector3*	particlePositions() const;

	/** Returns previous particle positions. */
	const math::Vector3*	particlePreviousPositions() const;

	/** Returns particle velocities. */
	const math::Vector3*	particleVelocities() const;

	/** Returns time elapsed since the particles were created. */
	const float*			particleTimes() const;

	/** Returns time when the particles will be removed. */
	const float*			particleLifeTimes() const;

protected:
	/** Returns particle positions. */
	math::Vector3*			particlePositions();

	/** Returns particle velocities. */
	math::Vector3*			particleVelocities();

	/** Returns time elapsed since the particles were created. */
	float*					particleTimes();

	/** Returns time when the particles will be removed. */
	float*					particleLifeTimes();

private:
	class ParticleSystemImpl;
	P(ParticleSystemImpl) m_this;

	void	addNewParticles( int n, const math::Matrix4x4& worldTransform );
	void	killParticles( int n );
	void	killRandomParticle();
	void	killOldestParticle();
	void	applyForces( float time, float dt );
	void	updateLife( float dt );

	ParticleSystem& operator=( const ParticleSystem& );
};


} // ps


#endif // _PS_PARTICLESYSTEM_H
