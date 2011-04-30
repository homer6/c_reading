#ifndef _MORPHANIMATION_H
#define _MORPHANIMATION_H


#include <sg/Morpher.h>
#include <lang/Object.h>
#include <lang/String.h>
#include <util/Vector.h>


/** 
 * Morph animation sequence. 
 * Morph animation is used to morph set of models, using multiple materials.
 * Morph animation also supports blending between different animations.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MorphAnimation :
	public lang::Object
{
public:
	explicit MorphAnimation( const lang::String& baseMeshName, const lang::String& animName );
	MorphAnimation( const MorphAnimation& other );
	~MorphAnimation();

	/** Adds a morpher to this animation. */
	void	addMorpher( sg::Morpher* morpher );

	/**	Updates morph animation state. */
	void	update( float dt );

	/** Applies morph animation state to the geometry. */
	void	apply();

	/** Starts the animation at specified time offset. */
	void	start( float startTime );

	/** 
	 * Starts blending the animation to destination animation. 
	 * @param dst Destination animation, target for blend operation.
	 * @param blendDelay Total time of blend operation, seconds.
	 * @param dstStartTime Start time offset for the destination animation.
	 */
	void	blendTo( MorphAnimation* dst, float blendDelay, float dstStartTime );

	/** Stops the animation. */
	void	stop();

	/** Sets animation to be removed after ended. */
	void	enableRemoveAfterEnd();

	/** Returns true if the animation is active (playing). */
	bool	active() const;

	/** Returns time of the animation. Requires that the animation is active. */
	float	time() const;

	/** Returns end time of the animation. */
	float	endTime() const;

	/** Returns true if this morph animation is currently target of blending operation. */
	bool	isBlendTarget() const;

	/** Returns true if this morph animation is currently source of blending operation. */
	bool	isBlendSource() const;

	/** Returns phase [0,1) of morph blend operation. 0 means complete source animation, 1 complete destination animation. */
	float	blendPhase() const;

	/** Sets animation to be removed after ended. */
	bool	isRemovedAfterEnd() const;

	/** Returns name of the morph animation. */
	const lang::String&		name() const											{return m_name;}

	/** Returns base mesh name of the morph animation. */
	const lang::String&		baseMeshName() const									{return m_baseMeshName;}

	/** Returns morph blend operation target animation. */
	MorphAnimation*			blendTarget() const;

private:
	util::Vector<P(sg::Morpher)>	m_morphers;
	lang::String					m_baseMeshName;
	lang::String					m_name;
	float							m_time;				// <0 if not active
	float							m_endTime;
	MorphAnimation*					m_blendTarget;		// 0 if none
	MorphAnimation*					m_blendSource;		// 0 if none
	float							m_blendTime;		// current time of blend
	float							m_blendDelay;		// total time of blend
	bool							m_removeAfterEnd;

	bool	valid() const;

	MorphAnimation& operator=( const MorphAnimation& );
};


#endif // _MORPHANIMATION_H
