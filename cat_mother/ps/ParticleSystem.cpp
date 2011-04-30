#include "ParticleSystem.h"
#include "Shape.h"
#include "Force.h"
#include "swapRemove.h"
#include <sg/Camera.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <math/Matrix4x4.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define MAX_UPDATE_INTERVAL 20e-3f

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace ps
{


class ParticleSystem::ParticleSystemImpl :
	public lang::Object
{
public:
	P(Shape)						posShape;
	P(Shape)						velShape;
	float							emissionRate;
	float							emissionTime;
	float							particleLifeTime;
	float							systemLifeTime;
	float							minEmissionSpeed;
	float							maxEmissionSpeed;
	int								maxParticles;
	bool							localSpace;
	ParticleSystem::KillType		kill;
	Vector< P(Force) >				forces;
	float							boundSphere;
	
	Vector3							prevPosition;
	float							time;
	float							newParticles;
	int								particles;
	Vector<float>					particleTimes;
	Vector<float>					particleLifeTimes;
	Vector<Vector3>					particlePositions;
	Vector<Vector3>					particlePreviousPositions;
	Vector<Vector3>					particleVelocities;
	Vector<Vector3>					particleForces;

	ParticleSystemImpl() :
		forces( Allocator<P(Force)>(__FILE__,__LINE__) ),
		particleTimes( Allocator<float>(__FILE__,__LINE__) ),
		particleLifeTimes( Allocator<float>(__FILE__,__LINE__) ),
		particlePositions( Allocator<Vector3>(__FILE__,__LINE__) ),
		particlePreviousPositions( Allocator<Vector3>(__FILE__,__LINE__) ),
		particleVelocities( Allocator<Vector3>(__FILE__,__LINE__) ),
		particleForces( Allocator<Vector3>(__FILE__,__LINE__) )
	{
		emissionRate			= 100.f;
		emissionTime			= 0.f;
		particleLifeTime		= 1.f;
		systemLifeTime			= 100e3f;
		minEmissionSpeed		= 0.f;
		maxEmissionSpeed		= 0.f;
		maxParticles			= 100;
		localSpace				= false;
		kill					= KILL_RANDOM;
	
		prevPosition			= Vector3(0,0,0);
		boundSphere				= 0.f;	

		time					= 0.f;
		newParticles			= 0.f;
		particles				= 0;
	}
};

//-----------------------------------------------------------------------------

ParticleSystem::ParticleSystem()
{
	m_this = new ParticleSystemImpl;
}

ParticleSystem::ParticleSystem( const ParticleSystem& other ) :
	Node( other )
{
	m_this = new ParticleSystemImpl( *other.m_this );
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::setPositionShape( Shape* shape )
{
	m_this->posShape = shape;
}

void ParticleSystem::setVelocityShape( Shape* shape )
{
	m_this->velShape = shape;
}

void ParticleSystem::setEmissionRate( float value )
{
	m_this->emissionRate = value;
}

void ParticleSystem::setParticleLifeTime( float value )
{
	m_this->particleLifeTime = value;
}

void ParticleSystem::setSystemLifeTime( float value )
{
	m_this->systemLifeTime = value;
}

void ParticleSystem::setMaxParticles( int count )
{
	m_this->maxParticles = count;
}

void ParticleSystem::setKillType( KillType kill )
{
	m_this->kill = kill;
}

void ParticleSystem::addForce( Force* force )
{
	m_this->forces.add( force );
}

bool ParticleSystem::updateVisibility( sg::Camera* camera )
{
	return camera->isInView( cachedWorldTransform().translation(), boundSphere() );
}

void ParticleSystem::update( float dt )
{
	// reset previous position
	if ( 0.f == m_this->time )
		m_this->prevPosition = worldTransform().translation();

	// emission scale by speed
	float emissionSpeedScale = 1.f;
	Matrix4x4 wtm = worldTransform();
	if ( m_this->maxEmissionSpeed > 0.f && dt > Float::MIN_VALUE )
	{
		float speed = (wtm.translation() - m_this->prevPosition).length() / dt;
		if ( speed <= m_this->minEmissionSpeed )
			emissionSpeedScale = 0.f;
		else if ( speed >= m_this->maxEmissionSpeed )
			emissionSpeedScale = 1.f;
		else
			emissionSpeedScale = (speed-m_this->minEmissionSpeed) / (m_this->maxEmissionSpeed - m_this->minEmissionSpeed);
	}

	// newCount = number of new particles to create in this update
	const int maxParticles = m_this->maxParticles;
	if ( m_this->time <= m_this->emissionTime || m_this->emissionTime == 0.f )
		m_this->newParticles += dt * m_this->emissionRate * emissionSpeedScale;
	int newCount = (int)m_this->newParticles;
	m_this->newParticles -= (float)newCount;
	if ( newCount > maxParticles )
		newCount = maxParticles;

	applyForces( m_this->time, dt );
	updateLife( dt );

	// create new particles
	if ( m_this->particles+newCount > maxParticles )
		killParticles( newCount+m_this->particles-maxParticles );
	if ( m_this->particles+newCount > maxParticles )
		newCount = maxParticles - m_this->particles;

	if ( newCount > 0 )
	{
		if ( m_this->localSpace )
			addNewParticles( newCount, Matrix4x4(1) );
		else
			addNewParticles( newCount, wtm );
	}

	m_this->time += dt;
	m_this->prevPosition = wtm.translation();
}

void ParticleSystem::reset()
{
	while ( m_this->particles > 0 )
		removeParticle( m_this->particles-1 );

	m_this->prevPosition = Vector3(0,0,0);
	m_this->time = 0.f;
	m_this->newParticles = 0;

	assert( m_this->particles == 0 );
	assert( m_this->particleTimes.size() == 0 );
	assert( m_this->particleLifeTimes.size() == 0 );
	assert( m_this->particlePositions.size() == 0 );
	assert( m_this->particlePreviousPositions.size() == 0 );
	assert( m_this->particleVelocities.size() == 0 );

	setTransform( Matrix4x4(1) );
  	setState( 0.f );
}

void ParticleSystem::killParticles( int n )
{
	for ( int i = 0 ; i < n ; ++i )
	{
		switch ( m_this->kill )
		{
		case ParticleSystem::KILL_NOTHING:
			break;
		case ParticleSystem::KILL_OLDEST:
			killOldestParticle();
			break;
		case ParticleSystem::KILL_RANDOM:
			killRandomParticle();
			break;
		}
	}
}

void ParticleSystem::killRandomParticle()
{
	assert( m_this->particles > 0 );

	int i = (int)(Math::random() * m_this->particles);
	if ( i < 0 )
		i = 0;
	else if ( i >= m_this->particles )
		i = m_this->particles-1;

	if ( i < m_this->particles )
		removeParticle( i );
}

void ParticleSystem::killOldestParticle()
{
	assert( m_this->particles > 0 );

	int oldest = 0;
	float oldestTime = 0.f;
	int i = 0;
	for ( i = 0 ; i < m_this->particles ; ++i )
	{
		float t = m_this->particleTimes[i];
		if ( t > oldestTime )
		{
			oldestTime = t;
			oldest = i;
		}
	}

	if ( oldest < m_this->particles )
		removeParticle( oldest );
}

void ParticleSystem::addNewParticles( int n, const Matrix4x4& worldTransform )
{
	if ( m_this->particles+n > m_this->maxParticles )
		n = m_this->maxParticles - m_this->particles;

	// add particles spread between last and current position
	Matrix4x4 wtm = worldTransform;
	Vector3 delta(0,0,0);
	if ( !m_this->localSpace && n > 0 )
		delta = (wtm.translation() - m_this->prevPosition) * (1.f/n);

	for ( int i = 0 ; i < n ; ++i )
	{
		Vector3 pos(0,0,0);
		Vector3 vel(0,0,0);

		if ( m_this->posShape )
			m_this->posShape->getRandomPoint( &pos );
		if ( m_this->velShape )
			m_this->velShape->getRandomPoint( &vel );

		Vector4 pos4 = wtm * Vector4(pos.x,pos.y,pos.z,1);
		Vector4 vel4 = wtm * Vector4(vel.x,vel.y,vel.z,0);
		Vector3 pos3 = Vector3( pos4.x, pos4.y, pos4.z );
		Vector3 vel3 = Vector3( vel4.x, vel4.y, vel4.z );
		addParticle( pos3, vel3, m_this->particleLifeTime );

		wtm.setTranslation( wtm.translation()+delta );
	}
}

void ParticleSystem::addParticle( const Vector3& pos, const Vector3& vel, float lifeTime )
{
	assert( particles() < maxParticles() );

	m_this->particleTimes.add( 0.f );
	m_this->particleLifeTimes.add( lifeTime );
	m_this->particlePositions.add( pos );
	m_this->particlePreviousPositions.add( pos );
	m_this->particleVelocities.add( vel );
	m_this->particles += 1;
}

void ParticleSystem::removeParticle( int index )
{
	assert( index >= 0 && index < particles() );
	assert( index < (int)m_this->particleTimes.size() );
	assert( index < (int)m_this->particleLifeTimes.size() );
	assert( index < (int)m_this->particlePositions.size() );
	assert( index < (int)m_this->particlePreviousPositions.size() );
	assert( index < (int)m_this->particleVelocities.size() );

	swapRemove( m_this->particleTimes, index );
	swapRemove( m_this->particleLifeTimes, index );
	swapRemove( m_this->particlePositions, index );
	swapRemove( m_this->particlePreviousPositions, index );
	swapRemove( m_this->particleVelocities, index );

	m_this->particles -= 1;
}

void ParticleSystem::applyForces( float time, float dt )
{
	// collect forces
	m_this->particleForces.clear();
	m_this->particleForces.setSize( m_this->particles, Vector3(0,0,0) );
	int i;
	for ( i = 0 ; i < (int)m_this->forces.size() ; ++i )
		m_this->forces[i]->apply( time, dt, m_this->particlePositions.begin(), m_this->particleVelocities.begin(), m_this->particleForces.begin(), m_this->particles );

	// integrate forces
	// (integration depends on framerate but thats *exactly* what we want
	// since we don't want to spend time on 'accurate' particle effects
	// in slow computers)
	for ( i = 0 ; i < m_this->particles ; ++i )
	{
		Vector3& vel = m_this->particleVelocities[i];
		vel += m_this->particleForces[i] * dt;
		m_this->particlePreviousPositions[i] = m_this->particlePositions[i];
		m_this->particlePositions[i] += vel * dt;
	}
}

void ParticleSystem::updateLife( float dt )
{
	for ( int i = 0 ; i < m_this->particles ; ++i )
	{
		float& t = m_this->particleTimes[i];
		t += dt;

		if ( t > m_this->particleLifeTimes[i] )
		{
			removeParticle( i );
			--i;
		}
	}
}

int ParticleSystem::particles() const
{
	return m_this->particles;
}

int ParticleSystem::maxParticles() const
{
	return m_this->maxParticles;
}

float ParticleSystem::time() const
{
	return m_this->time;
}

const math::Vector3* ParticleSystem::particlePositions() const
{
	assert( particles() == (int)m_this->particlePositions.size() );
	return m_this->particlePositions.begin();
}

const math::Vector3* ParticleSystem::particlePreviousPositions() const
{
	assert( particles() == (int)m_this->particlePreviousPositions.size() );
	return m_this->particlePreviousPositions.begin();
}

const math::Vector3* ParticleSystem::particleVelocities() const
{
	assert( particles() == (int)m_this->particleVelocities.size() );
	return m_this->particleVelocities.begin();
}

const float* ParticleSystem::particleTimes() const
{
	assert( particles() == (int)m_this->particleTimes.size() );
	return m_this->particleTimes.begin();
}

const float* ParticleSystem::particleLifeTimes() const
{
	assert( particles() == (int)m_this->particleLifeTimes.size() );
	return m_this->particleLifeTimes.begin();
}

math::Vector3* ParticleSystem::particlePositions()
{
	assert( particles() == (int)m_this->particlePositions.size() );
	return m_this->particlePositions.begin();
}

math::Vector3* ParticleSystem::particleVelocities()
{
	assert( particles() == (int)m_this->particleVelocities.size() );
	return m_this->particleVelocities.begin();
}

float* ParticleSystem::particleTimes()
{
	assert( particles() == (int)m_this->particleTimes.size() );
	return m_this->particleTimes.begin();
}

float* ParticleSystem::particleLifeTimes()
{
	assert( particles() == (int)m_this->particleLifeTimes.size() );
	return m_this->particleLifeTimes.begin();
}

void ParticleSystem::setLocalSpace( bool enabled )
{
	m_this->localSpace = enabled;
}

bool ParticleSystem::localSpace() const
{
	return m_this->localSpace;
}

float ParticleSystem::systemLifeTime() const
{
	return m_this->systemLifeTime;
}

bool ParticleSystem::alive() const
{
	return m_this->time < m_this->systemLifeTime;
}

void ParticleSystem::setBoundSphere( float r )
{
	m_this->boundSphere = r;
}

float ParticleSystem::boundSphere() const
{
	return m_this->boundSphere;
}

void ParticleSystem::setEmissionTime( float t )
{
	m_this->emissionTime = t;
}

void ParticleSystem::setMinEmissionSpeed( float v )
{
	m_this->minEmissionSpeed = v;
}

void ParticleSystem::setMaxEmissionSpeed( float v )
{
	m_this->maxEmissionSpeed = v;
}

void ParticleSystem::kill()
{
	m_this->time = m_this->systemLifeTime;
}


} // ps
