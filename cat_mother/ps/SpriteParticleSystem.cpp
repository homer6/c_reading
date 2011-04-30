#include "SpriteParticleSystem.h"
#include "Box.h"
#include "Sphere.h"
#include "Gravity.h"
#include "swapRemove.h"
#include <io/IOException.h>
#include <io/CommandReader.h>
#include <io/FileInputStream.h>
#include <io/InputStreamReader.h>
#include <io/InputStreamArchive.h>
#include <gd/GraphicsDevice.h>
#include <sg/Camera.h>
#include <sg/Context.h>
#include <sg/Texture.h>
#include <sg/Material.h>
#include <sg/VertexLock.h>
#include <sg/VertexFormat.h>
#include <sg/TriangleList.h>
#include <pix/SurfaceFormat.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <math/Noise.h>
#include <math/Vector2.h>
#include <math/FloatUtil.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace ps
{


class SpriteParticleSystem::SpriteParticleSystemImpl :
	public lang::Object
{
public:
	class Rect
	{
	public:
		float u0;
		float v0;
		float u1;
		float v1;
	};

	P(Material)						mat;
	P(TriangleList)					tri;
	float							dt;
	Vector<Rect>					sources;
	Vector<int>						particleAnims;
	Vector<float>					particleSizes;
	Vector<float>					particleAngles;
	Vector<float>					particleInitAngles;
	Vector<float>					particleAngleSpeeds;

	P(Texture)						animImages;
	int								animImageRows;
	int								animImageCols;
	int								animFrames;
	float							animFps;
	SpriteParticleSystem::BehaviourType	animEnd;

	float							speedScale;
	float							minSize;
	float							maxSize;
	float							minAngle;
	float							maxAngle;
	float							minAngleSpeed;
	float							maxAngleSpeed;
	BlendType						blend;

	SpriteParticleSystemImpl() :
		dt(0.f),
		sources( Allocator<Rect>(__FILE__,__LINE__) ),
		particleAnims( Allocator<int>(__FILE__,__LINE__) ),
		particleSizes( Allocator<float>(__FILE__,__LINE__) ),
		particleAngles( Allocator<float>(__FILE__,__LINE__) ),
		particleInitAngles( Allocator<float>(__FILE__,__LINE__) ),
		particleAngleSpeeds( Allocator<float>(__FILE__,__LINE__) )
	{
		animImageRows		= 1;
		animImageCols		= 1;
		animEnd				= SpriteParticleSystem::BEHAVIOUR_LOOP;

		speedScale			= 0.f;
		minSize				= 1.f;
		maxSize				= 1.f;
		minAngle			= 0.f;
		maxAngle			= 0.f;
		minAngleSpeed		= 0.f;
		maxAngleSpeed		= 0.f;
		blend				= BLEND_ADD;
	}

	static VertexFormat vertexFormat()
	{
		VertexFormat vf;
		vf.addRHW();
		vf.addTextureCoordinate(2);
		return vf;
	}
};

//-----------------------------------------------------------------------------

/** Returns pseudo-random value in range [minv,maxv). */
inline static float random( float minv, float maxv )
{
	float t = Math::random();
	float v = minv + t * (maxv - minv);
	return v;
}

/** Loads texture from archive or from file. */
static P(Texture) loadTex( const String& str, InputStreamArchive* zip )
{
	if ( zip )
	{
		P(InputStream) texin = zip->getInputStream( str );
		P(Texture) tex = new Texture( texin, str );
		texin->close();
		return tex;
	}
	else
	{
		P(InputStream) texin = new FileInputStream( str );
		P(Texture) tex = new Texture( texin, str );
		texin->close();
		return tex;
	}
}

//-----------------------------------------------------------------------------

SpriteParticleSystem::SpriteParticleSystem()
{
	m_this = new SpriteParticleSystemImpl;
}

SpriteParticleSystem::~SpriteParticleSystem()
{
}

SpriteParticleSystem::SpriteParticleSystem( const String& filename )
{
	P(InputStream) in = new FileInputStream( filename );
	load( in, 0 );
	in->close();
}

SpriteParticleSystem::SpriteParticleSystem( const String& filename, InputStreamArchive* zip )
{
	P(InputStream) in = zip->getInputStream( filename );
	load( in, zip );
	in->close();
}

SpriteParticleSystem::SpriteParticleSystem( const SpriteParticleSystem& other ) :
	ParticleSystem( other )
{
	m_this = new SpriteParticleSystemImpl( *other.m_this );
}

Node* SpriteParticleSystem::clone() const
{
	return new SpriteParticleSystem( *this );
}

void SpriteParticleSystem::addParticle( const math::Vector3& pos, const math::Vector3& vel, float lifeTime )
{
	ParticleSystem::addParticle( pos, vel, lifeTime );

	m_this->particleAnims.add( 0 );

	// randomize angles
	float ang = random(m_this->minAngle,m_this->maxAngle);
	m_this->particleAngles.add( ang );
	m_this->particleInitAngles.add( ang );
	m_this->particleAngleSpeeds.add( random(m_this->minAngleSpeed,m_this->maxAngleSpeed) );

	// randomize size
	m_this->particleSizes.add( random(m_this->minSize,m_this->maxSize) );
}

void SpriteParticleSystem::removeParticle( int index )
{
	assert( index >= 0 && index < (int)m_this->particleAnims.size() );
	assert( index < (int)m_this->particleSizes.size() );
	assert( index < (int)m_this->particleAngles.size() );
	assert( index < (int)m_this->particleInitAngles.size() );
	assert( index < (int)m_this->particleAngleSpeeds.size() );

	swapRemove( m_this->particleAnims, index );
	swapRemove( m_this->particleSizes, index );
	swapRemove( m_this->particleAngles, index );
	swapRemove( m_this->particleInitAngles, index );
	swapRemove( m_this->particleAngleSpeeds, index );

	ParticleSystem::removeParticle( index );
}

void SpriteParticleSystem::setImage( Texture* tex )
{
	setImage( tex, 1, 1, 1, 1.f, BEHAVIOUR_LOOP );
}

void SpriteParticleSystem::setImage( Texture* tex, int rows, int cols, int frames, float fps, BehaviourType end )
{
	m_this->animImageRows	= rows;
	m_this->animImageCols	= cols;
	m_this->animFrames		= frames;
	m_this->animFps			= fps;
	m_this->animEnd			= end;
	m_this->animImages		= tex;
	m_this->mat				= 0;
}

void SpriteParticleSystem::createMaterial()
{
	m_this->mat = new Material;

	if ( m_this->blend == BLEND_ADD )
	{
		m_this->mat->setBlend( Material::BLEND_ONE, Material::BLEND_ONE );
		m_this->mat->setPolygonSorting( false );
	}
	else if ( m_this->blend == BLEND_MUL )
	{
		m_this->mat->setBlend( Material::BLEND_SRCALPHA, Material::BLEND_INVSRCALPHA );
		m_this->mat->setPolygonSorting( true );
	}

	m_this->mat->setDepthWrite( false );
	m_this->mat->setLighting( false );
	m_this->mat->setFogDisabled( true );
	m_this->mat->setVertexFormat( SpriteParticleSystemImpl::vertexFormat() );

	if ( m_this->animImages )
		m_this->mat->setTexture( 0, m_this->animImages );

	if ( m_this->tri )
		m_this->tri->setShader( m_this->mat );
}

void SpriteParticleSystem::createSprite()
{
	if ( maxParticles() > 0 )
	{
		m_this->tri = new TriangleList( 2*3*maxParticles(), SpriteParticleSystemImpl::vertexFormat(), TriangleList::USAGE_DYNAMIC );

		float du = 1.f / m_this->animImageCols;
		float dv = 1.f / m_this->animImageRows;
		for ( int j = 0 ; j < m_this->animImageRows ; ++j )
		{
			for ( int i = 0 ; i < m_this->animImageCols ; ++i )
			{
				float u = du * i;
				float v = dv * j;

				SpriteParticleSystemImpl::Rect rc;
				rc.u0 = u;
				rc.v0 = v;
				rc.u1 = u+du;
				rc.v1 = v+dv;
				m_this->sources.add( rc );
			}
		}

		if ( m_this->mat )
			m_this->tri->setShader( m_this->mat );
	}
}

void SpriteParticleSystem::prepare( Camera* camera )
{
	// world/local space particles
	Matrix4x4 worldTm;
	if ( localSpace() )
		worldTm = cachedWorldTransform();
	else
		worldTm = Matrix4x4(1.f);

	// transforms
	Matrix4x4 viewTm = camera->cachedViewTransform();
	Matrix4x4 worldViewTm = viewTm * worldTm;
	Matrix4x4 projTm = camera->cachedProjectionTransform();
	Matrix4x4 totalTm = projTm * worldViewTm;

	TriangleList*	tri					= m_this->tri;
	const int*		particleSources		= m_this->particleAnims.begin();
	const Vector3*	particlePositions	= this->particlePositions();
	const Vector3*	particleVelocities	= this->particleVelocities();
	const float*	particleSizes		= m_this->particleSizes.begin();
	const float*	particleAngles		= m_this->particleAngles.begin();
	const int		particles			= this->particles();
	const float		viewportDimX		= .5f * (float)camera->viewportWidth();
	const float		viewportDimY		= .5f * (float)camera->viewportHeight();
	const float		viewportCenterX		= (float)camera->viewportCenterX();
	const float		viewportCenterY		= (float)camera->viewportCenterY();

	// prepare triangle list for rendering
	tri->setVertices( particles*3*2 );
	VertexLock<TriangleList> lk( tri, TriangleList::LOCK_WRITE );
	float* vdata;
	int vpitch;
	tri->getVertexPositionData( &vdata, &vpitch );

	// set particle vertices
	float* v = vdata;
	for ( int i = 0 ; i < particles ; ++i )
	{
		// particle position in view space
		Vector3 viewPos;
		worldViewTm.transform( particlePositions[i], &viewPos );

		// particle dimensions in screen space
		Vector4 pdim = projTm * Vector4(particleSizes[i]*.5f, 0.f, viewPos.z, 1.f);
		float persp = 1.f/pdim.w;
		pdim *= persp;
		pdim.x *= viewportDimX;
		Vector2 dimx( pdim.x, 0.f );
		Vector2 dimy( 0, dimx.x );

		// particle position in screen space
		Vector4 pp4 = projTm * Vector4(viewPos.x,viewPos.y,viewPos.z,1.f);
		pp4 *= persp;
		pp4.x = viewportCenterX + pp4.x * viewportDimX;
		pp4.y = viewportCenterY - pp4.y * viewportDimY;
		Vector2 pp(pp4.x,pp4.y);

		if ( m_this->speedScale > 0.f )
		{
			Vector3 viewVel = viewTm.rotate( particleVelocities[i] );
			viewVel.z = 0.f;

			// view space points
			Vector3 viewPoints[4];
			Vector3 deltax = viewVel * m_this->speedScale;
			if ( deltax.length() > Float::MIN_VALUE )
			{
				if ( deltax.length() < particleSizes[i]*.5f )
					deltax = deltax.normalize() * particleSizes[i]*.5f;
				Vector3 deltay( -deltax.y, deltax.x, 0.f );
				deltay = deltay.normalize() * particleSizes[i]*.5f;
				viewPoints[0] = viewPos - deltax + deltay;
				viewPoints[1] = viewPos + deltax + deltay;
				viewPoints[2] = viewPos + deltax - deltay;
				viewPoints[3] = viewPos - deltax - deltay;
			}

			// screen space points
			Vector4 screenPoints[4];
			for ( int i = 0 ; i < 4 ; ++i )
			{
				screenPoints[i] = projTm * Vector4(viewPoints[i].x,viewPoints[i].y,viewPoints[i].z,1.f);
				screenPoints[i] *= 1.f / screenPoints[i].w;
				screenPoints[i].x = viewportCenterX + screenPoints[i].x*viewportDimX;
				screenPoints[i].y = viewportCenterY - screenPoints[i].y*viewportDimY;
			}

			pp = Vector2( (screenPoints[0].x+screenPoints[2].x)*.5f, (screenPoints[0].y+screenPoints[2].y)*.5f );
			dimx = Vector2( screenPoints[1].x-screenPoints[0].x, screenPoints[1].y-screenPoints[0].y )*.5f;
			dimy = Vector2( screenPoints[2].x-screenPoints[1].x, screenPoints[2].y-screenPoints[1].y )*.5f;

			//if ( particleAngles[i] != 0.f )
			//	Debug::printlnWarning( "Particle system {0} uses rotation angles but SpeedScale overrides them", name() );
		}
		else if ( particleAngles[i] != 0.f )
		{
			// zrot =  cz           -sz			  0
			//         sz            cz			  0
			//         0              0           1
			float cz = Math::cos( particleAngles[i] );
			float sz = Math::sin( particleAngles[i] );
			float m[2][2] =
			{
				cz, -sz,
				sz, cz
			};

			// rotate points
			dimx = Vector2( m[0][0]*dimx.x + m[0][1]*dimx.y, m[1][0]*dimx.x + m[1][1]*dimx.y );
			dimy = Vector2( m[0][0]*dimy.x + m[0][1]*dimy.y, m[1][0]*dimy.x + m[1][1]*dimy.y );
		}

		// compute screen quad
		Vector2 pd0 = -dimx - dimy;
		Vector2 pd1 =  dimx - dimy;
		Vector2 pd2 =  dimx + dimy;
		Vector2 pd3 = -dimx + dimy;
		Vector2 p0 = pp + pd0;
		Vector2 p1 = pp + pd1;
		Vector2 p2 = pp + pd2;
		Vector2 p3 = pp + pd3;

		// tri 0,1,2 (WARNING: assumes that 2-vector texcoord follow 4-vector position in vertex data)
		const SpriteParticleSystemImpl::Rect& rc = m_this->sources[ particleSources[i] ];
		v[0] = p0.x;
		v[1] = p0.y;
		v[2] = pp4.z;
		v[3] = pp4.w;
		v[4] = rc.u0;
		v[5] = rc.v0;
		v += vpitch;
		v[0] = p1.x;
		v[1] = p1.y;
		v[2] = pp4.z;
		v[3] = pp4.w;
		v[4] = rc.u1;
		v[5] = rc.v0;
		v += vpitch;
		v[0] = p2.x;
		v[1] = p2.y;
		v[2] = pp4.z;
		v[3] = pp4.w;
		v[4] = rc.u1;
		v[5] = rc.v1;
		v += vpitch;

		// tri 0,2,3
		v[0] = p0.x;
		v[1] = p0.y;
		v[2] = pp4.z;
		v[3] = pp4.w;
		v[4] = rc.u0;
		v[5] = rc.v0;
		v += vpitch;
		v[0] = p2.x;
		v[1] = p2.y;
		v[2] = pp4.z;
		v[3] = pp4.w;
		v[4] = rc.u1;
		v[5] = rc.v1;
		v += vpitch;
		v[0] = p3.x;
		v[1] = p3.y;
		v[2] = pp4.z;
		v[3] = pp4.w;
		v[4] = rc.u0;
		v[5] = rc.v1;
		v += vpitch;
	}
}

void SpriteParticleSystem::render( Camera* camera, int pass )
{
	assert( m_this->mat );
	assert( m_this->tri );

	// render particles in pass 2
	if ( particles() > 0 &&
		m_this->tri && 
		m_this->mat && 
		Material::DEFAULT_TRANSPARENCY_PASS == pass )
	{
		prepare( camera );
		m_this->tri->draw();
	}
}

void SpriteParticleSystem::update( float dt )
{
	ParticleSystem::update( dt );

	const float*	times		= particleTimes();
	const int		particles	= this->particles();

	// update particle Z-rotations
	if ( m_this->minAngleSpeed != 0.f || m_this->maxAngleSpeed != 0.f )
	{
		float* particleAngles = m_this->particleAngles.begin();
		const float* particleInitAngles = m_this->particleInitAngles.begin();
		const float* particleAngleSpeeds = m_this->particleAngleSpeeds.begin();

		for ( int i = 0 ; i < particles ; ++i )
		{
			particleAngles[i] = particleInitAngles[i] + particleAngleSpeeds[i] * times[i];
		}
	}

	// update particle anims
	if ( m_this->animFrames > 1 )
	{
		const float* lifeTimes = particleLifeTimes();
		int frames = m_this->animFrames;
		int frames2 = frames * 2;
		assert( particles == (int)m_this->particleAnims.size() );

		int i;
		switch ( m_this->animEnd )
		{
		case SpriteParticleSystem::BEHAVIOUR_LOOP:
			for ( i = 0 ; i < particles ; ++i )
			{
				float frame = times[i] * m_this->animFps;
				int framei = FloatUtil::floatToInt( frame ) % frames;
				m_this->particleAnims[i] = framei;
			}
			break;

		case SpriteParticleSystem::BEHAVIOUR_MIRROR:
			for ( i = 0 ; i < particles ; ++i )
			{
				float frame = times[i] * m_this->animFps;
				int framei = FloatUtil::floatToInt( frame ) % frames2;
				if ( framei >= frames )
					framei = frames2 - framei - 1;
				assert( framei >= 0 && framei < frames );
				m_this->particleAnims[i] = framei;
			}
			break;

		case SpriteParticleSystem::BEHAVIOUR_LIFE:
			for ( i = 0 ; i < particles ; ++i )
			{
				float frame = times[i]/lifeTimes[i] * frames;
				int framei = FloatUtil::floatToInt( frame );
				if ( framei >= frames )
					framei = frames-1;
				m_this->particleAnims[i] = framei;
			}
			break;

		case SpriteParticleSystem::BEHAVIOUR_RANDOM:
			for ( i = 0 ; i < particles ; ++i )
			{
				float frame = times[i] * m_this->animFps;
				int framei = FloatUtil::floatToInt( frame );
				float v = Noise::noise2( framei*.312345f+0.98123097f, i*0.962401f+0.1632327f );
				int vi = *reinterpret_cast<int*>(&v);
				vi &= 0x7FFFFFFF;
				m_this->particleAnims[i] = vi % frames;
			}
			break;
		}
	}

	m_this->dt = dt;
}

void SpriteParticleSystem::setParticleMinSize( float size )
{
	m_this->minSize = size;
}

void SpriteParticleSystem::setParticleMaxSize( float size )
{
	m_this->maxSize = size;
}

float SpriteParticleSystem::particleMinSize() const
{
	return m_this->minSize;
}

float SpriteParticleSystem::particleMaxSize() const
{
	return m_this->maxSize;
}

void SpriteParticleSystem::setParticleMinRotation( float angle )
{
	m_this->minAngle = angle;
}

void SpriteParticleSystem::setParticleMaxRotation( float angle )
{
	m_this->maxAngle = angle;
}

void SpriteParticleSystem::setParticleMinRotationSpeed( float angleSpeed )
{
	m_this->minAngleSpeed = angleSpeed;
}

void SpriteParticleSystem::setParticleMaxRotationSpeed( float angleSpeed )
{
	m_this->maxAngleSpeed = angleSpeed;
}

float SpriteParticleSystem::particleMinRotation() const
{
	return m_this->minAngle;
}

float SpriteParticleSystem::particleMaxRotation() const
{
	return m_this->maxAngle;
}

float SpriteParticleSystem::particleMinRotationSpeed() const
{
	return m_this->minAngleSpeed;
}

float SpriteParticleSystem::particleMaxRotationSpeed() const
{
	return m_this->maxAngleSpeed;
}

const float* SpriteParticleSystem::particleSizes() const
{
	return m_this->particleSizes.begin();
}

float* SpriteParticleSystem::particleSizes()
{
	return m_this->particleSizes.begin();
}

void SpriteParticleSystem::load( InputStream* in, InputStreamArchive* zip )
{
	m_this = new SpriteParticleSystemImpl;
	SpriteParticleSystem* ps = this;
	
	P(InputStreamReader) inreader = new InputStreamReader( in );
	P(CommandReader) reader = new CommandReader( inreader, in->toString() );

	String str;
	while ( reader->readString(str) )
	{
		if ( str.startsWith("#") )
		{
			reader->readLine( str );
			continue;
		}
		else if ( str == "ObjectName" )
		{
			reader->readLine( str );
			ps->setName( str );
		}
		else if ( str == "EmissionRate" )
		{
			float emissionRate = reader->readFloat();
			ps->setEmissionRate( emissionRate );
		}
		else if ( str == "ParticleLifeTime" )
		{
			float particleLifeTime = reader->readFloat();
			ps->setParticleLifeTime( particleLifeTime );
		}
		else if ( str == "SystemLifeTime" )
		{
			float systemLifeTime = reader->readFloat();
			ps->setSystemLifeTime( systemLifeTime );
		}
		else if ( str == "MaxParticles" )
		{
			int maxParticles = reader->readInt();
			ps->setMaxParticles( maxParticles );
		}
		else if ( str == "LocalSpace" )
		{
			ps->setLocalSpace( 0 != reader->readInt() );
		}
		else if ( str == "Size" )
		{
			float minSize = reader->readFloat();
			float maxSize = reader->readFloat();
			ps->setParticleMinSize( minSize );
			ps->setParticleMaxSize( maxSize );
		}
		else if ( str == "Kill" )
		{
			reader->readString( str );
			KillType killType = ParticleSystem::KILL_RANDOM;
			if ( str == "RANDOM" )
				killType = ParticleSystem::KILL_RANDOM;
			else if ( str == "OLDEST" )
				killType = ParticleSystem::KILL_OLDEST;
			else if ( str == "NOTHING" )
				killType = ParticleSystem::KILL_NOTHING;
			else
				throw IOException( Format("Particle system {0} kill type unknown ({1})", in->toString(), str) );
			ps->setKillType( killType );
		}
		else if ( str == "Image" )
		{
			reader->readLine( str );
			ps->setImage( loadTex(str,zip) );
		}
		else if ( str == "ImageAnim" )
		{
			reader->readString( str );
			P(Texture) tex = loadTex( str, zip );
			int rows = reader->readInt();
			int cols = reader->readInt();
			int frames = reader->readInt();
			float fps = reader->readFloat();
			reader->readString( str );
			BehaviourType end = BEHAVIOUR_LOOP;
			if ( str == "LOOP" )
				end = BEHAVIOUR_LOOP;
			else if ( str == "MIRROR" )
				end = BEHAVIOUR_MIRROR;
			else if ( str == "LIFE" )
				end = BEHAVIOUR_LIFE;
			else if ( str == "RANDOM" )
				end = BEHAVIOUR_RANDOM;
			ps->setImage( tex, rows, cols, frames, fps, end );
		}
		else if ( str == "ActivationTime" )
		{
			float t = reader->readFloat();
			Debug::println( "Particle system {0} uses deprecated command: ActivationTime {1}", in->toString(), t );
		}
		else if ( str == "Angle" )
		{
			float minAngle = Math::toRadians( reader->readFloat() );
			float maxAngle = Math::toRadians( reader->readFloat() );
			ps->setParticleMinRotation( minAngle );
			ps->setParticleMaxRotation( maxAngle );
		}
		else if ( str == "AngleSpeed" )
		{
			float minAngle = Math::toRadians( reader->readFloat() );
			float maxAngle = Math::toRadians( reader->readFloat() );
			ps->setParticleMinRotationSpeed( minAngle );
			ps->setParticleMaxRotationSpeed( maxAngle );
		}
		else if ( str == "PositionSphere" )
		{
			Vector3 pos;
			pos.x = reader->readFloat();
			pos.y = reader->readFloat();
			pos.z = reader->readFloat();
			float r = reader->readFloat();
			ps->setPositionShape( new Sphere( pos, r ) );
		}
		else if ( str == "VelocitySphere" )
		{
			Vector3 pos;
			pos.x = reader->readFloat();
			pos.y = reader->readFloat();
			pos.z = reader->readFloat();
			float r = reader->readFloat();
			ps->setVelocityShape( new Sphere( pos, r ) );
		}
		else if ( str == "PositionBox" )
		{
			Vector3 boxmin, boxmax;
			boxmin.x = reader->readFloat();
			boxmin.y = reader->readFloat();
			boxmin.z = reader->readFloat();
			boxmax.x = reader->readFloat();
			boxmax.y = reader->readFloat();
			boxmax.z = reader->readFloat();
			ps->setPositionShape( new Box( boxmin, boxmax ) );
		}
		else if ( str == "VelocityBox" )
		{
			Vector3 boxmin, boxmax;
			boxmin.x = reader->readFloat();
			boxmin.y = reader->readFloat();
			boxmin.z = reader->readFloat();
			boxmax.x = reader->readFloat();
			boxmax.y = reader->readFloat();
			boxmax.z = reader->readFloat();
			ps->setVelocityShape( new Box( boxmin, boxmax ) );
		}
		else if ( str == "Gravity" )
		{
			float g = reader->readFloat();
			ps->addForce( new Gravity(g) );
		}
		else if ( str == "BoundSphere" )
		{
			float r = reader->readFloat();
			ps->setBoundSphere( r );
		}
		else if ( str == "EmissionTime" )
		{
			float t = reader->readFloat();
			ps->setEmissionTime( t );
		}
		else if ( str == "MinEmissionSpeed" )
		{
			float t = reader->readFloat();
			ps->setMinEmissionSpeed( t );
		}
		else if ( str == "MaxEmissionSpeed" )
		{
			float t = reader->readFloat();
			ps->setMaxEmissionSpeed( t );
		}
		else if ( str == "SpeedScale" )
		{
			float t = reader->readFloat();
			ps->setSpeedScale( t );
		}
		else if ( str == "Blend" )
		{
			reader->readString( str );
			if ( str == "MUL" )
				ps->setBlend( BLEND_MUL );
			else if ( str == "ADD" )
				ps->setBlend( BLEND_ADD );
			else
				throw IOException( Format("Unknown particle system blending mode: {0}",str) );
		}
		else
		{
			throw IOException( Format("Unknown particle system command: {0}",str) );
		}
	}

	createSprite();
	createMaterial();
}

void SpriteParticleSystem::setBlend( SpriteParticleSystem::BlendType blend )
{
	m_this->blend = blend;
}

SpriteParticleSystem::BlendType SpriteParticleSystem::blend() const
{
	return m_this->blend;
}

void SpriteParticleSystem::setSpeedScale( float scale )
{
	m_this->speedScale = scale;
}


} // ps
