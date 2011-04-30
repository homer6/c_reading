#if 0
#include "PathParticleSystem.h"
#include "swapRemove.h"
#include <anim/KeyFrame.h>
#include <anim/KeyFrameContainer.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <math/Vector3.h>
#include <math/Matrix4x4.h>
#include <assert.h>
#include <stdlib.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace util;
using namespace math;
using namespace anim;

//-----------------------------------------------------------------------------

namespace ps
{


class Path :
	public Object
{
public:
	KeyFrameContainer		curve;

	Path() :
		curve( 3, KeyFrame::DATA_SCALAR )
	{
		curve.setEndBehaviour( KeyFrame::BEHAVIOUR_CONSTANT );
	}
};

class PathParticleSystem::PathParticleSystemImpl :
	public Object
{
public:
	Vector<Path>		paths;
	Vector<float>		particleSpeeds;
	Vector<int>			particleHints;
	Vector<int>			particlePaths;
	Vector<float>		originalParticleSizes;
	float				particleMinSpeed;
	float				particleMaxSpeed;
	float				particleStartScale;
	float				particleEndScale;
	bool				randomPathSelection;
	int					lastSelectedPath;

	PathParticleSystemImpl()
	{
		paths.setSize( 1 );

		particleMinSpeed	= 1.f;
		particleMaxSpeed	= 1.f;
		particleStartScale	= 1.f;
		particleEndScale	= 1.f;
		randomPathSelection	= true;
		lastSelectedPath	= 0;
	}
};

//-----------------------------------------------------------------------------

PathParticleSystem::PathParticleSystem()
{
	m_this = new PathParticleSystemImpl;
	setLocalSpace( true );
}

PathParticleSystem::PathParticleSystem( const PathParticleSystem& other ) :
	SpriteParticleSystem( other )
{
	m_this = new PathParticleSystemImpl( *other.m_this );
	setLocalSpace( true );
}

PathParticleSystem::~PathParticleSystem()
{
}

void PathParticleSystem::update( float dt )
{
	assert( paths() > 0 );

	SpriteParticleSystem::update( dt );

	// update positions
	int particles = this->particles();
	const float* times = particleTimes();
	Vector3* positions = particlePositions();
	int i;
	for ( i = 0 ; i < particles ; ++i )
	{
		// get local space position
		float v[3];
		float t = times[i] * m_this->particleSpeeds[i];
		bool alive = getParticlePosition( i, t, v );
		if ( !alive )
		{
			removeParticle( i );
			--particles;
			--i;
			continue;
		}

		// transform to world space
		Vector3 v1( v[0], v[1], v[2] );
		//Vector3 v1;
		//for ( int j = 0 ; j < 3 ; ++j )
		//	v1[j] = v[0]*wt(j,0) + v[1]*wt(j,1) + v[2]*wt(j,2);
		//v1.x += wt(0,3);
		//v1.y += wt(1,3);
		//v1.z += wt(2,3);

		positions[i] = v1;
	}

	// update sizes
	float* particleSizes = this->particleSizes();
	for ( i = 0 ; i < particles ; ++i )
	{
		int hint = m_this->particleHints[i];
		int path = m_this->particlePaths[i];
		float t = (float)hint / (float)m_this->paths[path].curve.keys();

		float deltaScale = m_this->particleEndScale - m_this->particleStartScale;
		float scale = deltaScale * t + m_this->particleStartScale;
		particleSizes[i] = m_this->originalParticleSizes[i] * scale;
	}
}

void PathParticleSystem::addParticle( const Vector3& /*pos*/, const Vector3& vel, float lifeTime )
{
	assert( paths() > 0 );
	assert( getPathPoints(0) > 0 );

	// select path
	int path = 0;
	if ( m_this->randomPathSelection )
	{
		path = rand() % paths();
	}
	else
	{
		m_this->lastSelectedPath += 1;
		if ( m_this->lastSelectedPath >= paths() )
			m_this->lastSelectedPath = 0;
		path = m_this->lastSelectedPath % paths();
	}
	m_this->particlePaths.add( path );
	m_this->particleHints.add( 0 );

	// randomize speed
	float t = Math::random();
	float speed = m_this->particleMinSpeed + t * (m_this->particleMaxSpeed - m_this->particleMinSpeed);
	assert( speed >= Float::MIN_VALUE );
	m_this->particleSpeeds.add( speed );

	float p[3];
	getParticlePosition( particles(), 0.f, p );
	SpriteParticleSystem::addParticle( Vector3(p[0],p[1],p[2]), vel, lifeTime );

	// store original particle size
	float size0 = particleSizes()[ particles()-1 ];
	m_this->originalParticleSizes.add( size0 );
}

void PathParticleSystem::removeParticle( int index )
{
	assert( index >= 0 && index < particles() );
	assert( index < (int)m_this->particlePaths.size() );
	assert( index < (int)m_this->particleHints.size() );
	assert( index < (int)m_this->particleSpeeds.size() );
	assert( index < (int)m_this->originalParticleSizes.size() );

	swapRemove( m_this->particlePaths, index );
	swapRemove( m_this->particleHints, index );
	swapRemove( m_this->particleSpeeds, index );
	swapRemove( m_this->originalParticleSizes, index );

	SpriteParticleSystem::removeParticle( index );
}

void PathParticleSystem::addPathPoint( float time, const Vector3& pos, int path )
{
	assert( path >= 0 && path < paths() );

	KeyFrame key;
	key.time = time;
	float v[3] = { pos.x, pos.y, pos.z };
	key.setChannels( v, 3 );
	key.interpolation = KeyFrame::INTERPOLATE_CR;
	m_this->paths[path].curve.insertKey( key );
}

int PathParticleSystem::getPathPoints( int path ) const
{
	assert( path >= 0 && path < paths() );
	return m_this->paths[path].curve.keys();
}

void PathParticleSystem::setPaths( int paths )
{
	m_this->paths.setSize( paths );
}

int PathParticleSystem::paths() const
{
	return m_this->paths.size();
}

bool PathParticleSystem::getParticlePosition( int index, float time, float* v3 ) const
{
	v3[2] = v3[1] = v3[0] = 0.f;

	assert( index < (int)m_this->particlePaths.size() );
	int pathIndex = m_this->particlePaths[index];
	assert( pathIndex < (int)m_this->paths.size() );
	Path& path = m_this->paths[ pathIndex ];

	assert( index < (int)m_this->particleHints.size() );
	m_this->particleHints[index] = path.curve.getValue( time, v3, 3, m_this->particleHints[index] );
	bool alive = ( time < path.curve.length() );

	return alive;
}

void PathParticleSystem::setParticleMinSpeed( float v )
{
	m_this->particleMinSpeed = v;
}

void PathParticleSystem::setParticleMaxSpeed( float v )
{
	m_this->particleMaxSpeed = v;
}

float PathParticleSystem::particleMinSpeed() const
{
	return m_this->particleMinSpeed;
}

float PathParticleSystem::particleMaxSpeed() const
{
	return m_this->particleMaxSpeed;
}

void PathParticleSystem::setParticleStartScale( float v )
{
	m_this->particleStartScale = v;
}

void PathParticleSystem::setParticleEndScale( float v )
{
	m_this->particleEndScale = v;
}

float PathParticleSystem::particleStartScale() const
{
	return m_this->particleStartScale;
}

float PathParticleSystem::particleEndScale() const
{
	return m_this->particleEndScale;
}

void PathParticleSystem::setRandomPathSelection( bool enabled )
{
	m_this->randomPathSelection = enabled;
}

bool PathParticleSystem::randomPathSelection() const
{
	return m_this->randomPathSelection;
}


} // ps
#endif
