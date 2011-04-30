#ifndef _BLENDER_H
#define _BLENDER_H


#include "AnimationParams.h"
#include "BlendData.h"
#include <util/Vector.h>
#include <util/Hashtable.h>
#include <lang/Object.h>


namespace lang {
	class String; }

/**
 * Blender automates animation blending. Animations are added with the "addBlend" method.
 * Animations are automatically faded in with the supplied blend delay time & target weight parameters.
 * Blender does not concern itself with animation's timing or relative weights, 
 * animation parameters can be modified with "setAnimWeight" and "setAnimtime" methods.
 * The animation can be faded out with the "fadeoutBlend" method.
 *
 * Blender uses AnimationParams-type structures as input and output. 
 *
 * @author Toni Aittoniemi
 */
class Blender :
	public lang::Object
{
public:
	/** Special animation ID's */
	enum ID
	{
		/** Represents an Invalid animation ID. */
		INVALIDID = -1

	};
	
	///
	Blender();

	///
	~Blender();

	/** 
	 * Updates blend times and states according to delta time. 
	 * @param dt delta time in seconds
	 */
	void			update( float dt );					

	/**
	 * Adds a new blend to the Blender.
	 * @param animationparams animation parameters, set weight to 0 to perform a fade-in from no weight to targetWeight.
	 * @param targetWeight desired animation weight at the end of blend
	 * @return blender ID
	 */
	int				addBlend( const AnimationParams& animationparams, float targetWeight );

	/**
	 * Adds a new blend to the Blender and sets it immediately to STATIC.
	 * @param animationparams animation parameters.
	 * @return blender ID
	 */
	int				addBlendInstantly( const AnimationParams& animationparams);

	/**
	 * Updates animation time of an existing blend and returns false if the blend has been removed.
	 * @param id blender ID
	 * @param float new animation time
	 * @return true if succesful, false if blender ID was removed from the blender (by a fade out call)
	 */
	int				setAnimTime( int id, float time );

	/**
	 * Updates animation weight of an existing blend and returns false if the blend has been removed.
	 * @param id blender ID
	 * @param float new animation weight
	 * @return true if succesful, false if blender ID was removed from the blender (by a fade out call)
	 */
	int				setAnimWeight( int id, float weight );

	/**
	 * Sets maximum slew rate for animation weight change in 1 second
	 * @param rate max slew / second
	 */
	void			setMaxSlewRate( float rate );

	/**
	 * Starts fade-out of an animation in the Blender.
	 * @param id blender ID
	 */	
	void			fadeoutBlend( int id );

	/**
	 * Starts fade-out of all animations in the Blender.
	 */	
	void			fadeoutAllBlends();

	/** Removes all animations from blender. */
	void			removeAllBlends();

	/**
	 * Copies the current state of the Blender to the buffer supplied, sum weights normalized to 1 if the total weight exceeds normalizelimit parameter.
	 * @param maxAnims Maximum number of animations allowed to be copied into buffer.
	 * @param buffer [out] Buffer for result parameters. Only blends with weights above zero are copied.
	 * @param normalizelimit Sum weights are normalized to 1 if the total weight exceeds this parameter.
	 * @return number of blends copied.
	 */
	int				getResult( int maxAnims, AnimationParams* buffer, float normalizelimit = 0.f );

	/** 
	 * Gets names, times & weights of anims currently in the blender.
	 * @param maxAnims maximum number of animations allowed to be copied into buffer
	 * @param names [out] Names of the running animations.
	 * @param times [out] Current times of the running animations.
	 * @param weights [out] Current weights of the running animations.
	 * @return Number of animations stored to output buffers.
	 */
	int				getAnims( int maxAnims, lang::String* names, float* times, float* weights ) const;

	/**
	 * Returns the total number of blends currently running in the blender.
	 * @return total number of blends currently running in the blender
	 */
	int				blends() const;

	/**
	 * Returns a string representing the Blender's current state
	 * @return string representing the Blender's current state
	 */
	lang::String	stateString( bool normalizeWeights=false ) const;

	/** Returns anim by id */
	const AnimationParams& getAnim( int id ) const;

	void getBlends( BlendData* blends ) const;

	/** Retrieves a blend from m_blends array by blender ID. */
	BlendData*		getBlend( int id ) const;

private:

	/** Maximum change of weight in 1 second. */
	float							m_maxSlewRate;

	/** Running number that represents blender ID's, it is reset to 0 when it is equal to Integer::MAX_VALUE. */
	int								m_idgenerator;
	/** Stores blender data. */
	util::Vector<P(BlendData)>		m_blends;

	/** Retains inactive data to be re-used when needed. */
	util::Vector<P(BlendData)>		m_blendCache;

};


#endif // _BLENDER_H
