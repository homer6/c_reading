#ifndef _PS_SPRITEPARTICLESYSTEM_H
#define _PS_SPRITEPARTICLESYSTEM_H


#include <ps/ParticleSystem.h>


namespace sg {
	class Texture;}

namespace io {
	class InputStream;
	class InputStreamArchive;}

namespace lang {
	class String;}


namespace ps
{


class Shape;
class Force;


/**
 * Particle system where particles are 2D bitmaps.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SpriteParticleSystem :
	public ParticleSystem
{
public:
	/** Image animation behaviour. */
	enum BehaviourType
	{
		/** Animation loops at the end. */
		BEHAVIOUR_LOOP,
		/** Animation playback flips direction at the end. */
		BEHAVIOUR_MIRROR,
		/** Animation lasts as long as particle life. */
		BEHAVIOUR_LIFE,
		/** Animation frame is selected randomly. */
		BEHAVIOUR_RANDOM
	};

	/** Blending mode. */
	enum BlendType
	{
		/** Additive blending. This is default. */
		BLEND_ADD,
		/** Multiplicative blending. */
		BLEND_MUL
	};

	/** Creates a default particle system. */
	SpriteParticleSystem();

	///
	~SpriteParticleSystem();

	sg::Node*	clone() const;

	/** Loads particle system description from (.ps) text file. */
	explicit SpriteParticleSystem( const lang::String& filename );

	/** Loads particle system description from (.ps) archive input stream. */
	explicit SpriteParticleSystem( const lang::String& filename, io::InputStreamArchive* zip );

	/** Creates a copy of other particle system. */
	SpriteParticleSystem( const SpriteParticleSystem& other );

	/** 
	 * Updates particle system. Call base class implementation if overriden. 
	 * Always call the base class implementation if you override this in derived classes.
	 */
	void		update( float dt );

	/** Renders all particles to active device. */
	void		render( sg::Camera* camera, int pass );

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

	/** Sets the bitmap of the sprite. */
	void		setImage( sg::Texture* tex );

	/** Sets the bitmap animation of the sprite. */
	void		setImage( sg::Texture* tex, int rows, int cols, int frames, float fps, BehaviourType end );

	/** Sets minimum initial Z-rotation angle (radians) for the particles. */
	void		setParticleMinRotation( float angle );

	/** Sets maximum initial Z-rotation angle (radians) for the particles. */
	void		setParticleMaxRotation( float angle );

	/** Sets minimum Z-rotation angle speed (radians/second) for the particles. */
	void		setParticleMinRotationSpeed( float angleSpeed );

	/** Sets maximum Z-rotation angle speed (radians/second)  for the particles. */
	void		setParticleMaxRotationSpeed( float angleSpeed );

	/** Sets minimum size for the particles. */
	void		setParticleMinSize( float size );

	/** Sets maximum size for the particles. */
	void		setParticleMaxSize( float size );

	/** Sets particle system blending mode. */
	void		setBlend( BlendType blend );

	/** Sets particle system velocity related scaling. Cannot be used with rotating particles. */
	void		setSpeedScale( float scale );

	/** Returns minimum size for the particles. */
	float		particleMinSize() const;

	/** Returns maximum size for the particles. */
	float		particleMaxSize() const;

	/** Returns minimum initial Z-rotation angle (radians) for the particles. */
	float		particleMinRotation() const;

	/** Returns maximum initial Z-rotation angle (radians) for the particles. */
	float		particleMaxRotation() const;

	/** Returns minimum Z-rotation angle speed (radians/second) for the particles. */
	float		particleMinRotationSpeed() const;

	/** Returns maximum Z-rotation angle speed (radians/second)  for the particles. */
	float		particleMaxRotationSpeed() const;

	/** Returns particle sizes. */
	const float* particleSizes() const;

	/** Sets particle system blending mode. */
	BlendType	blend() const;

protected:
	float*		particleSizes();

private:
	class SpriteParticleSystemImpl;
	P(SpriteParticleSystemImpl) m_this;

	void	load( io::InputStream* in, io::InputStreamArchive* zip );
	void	createMaterial();
	void	createSprite();
	void	prepare( sg::Camera* camera );


	SpriteParticleSystem& operator=( const SpriteParticleSystem& );
};


} // ps


#endif // _PS_SPRITEPARTICLESYSTEM_H
