#ifndef _SG_MORPHER_H
#define _SG_MORPHER_H


#include <sg/Primitive.h>


namespace anim {
	class VectorInterpolator;}


namespace sg
{


class Model;
class MorphTarget;


/** 
 * Weighted geometry morpher. Contains set of weighted morph targets. 
 * Morpher morphs base model by weighted morph targets to output model.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Morpher :
	public sg::Primitive
{
public:
	Morpher();
	~Morpher();

	Morpher( const Morpher& other, int shareFlags );

	Primitive* clone( int shareFlags ) const;

	/** Combines morphers with identical channel layouts. */
	void		blendState( anim::Animatable** anims, 
					const float* times, const float* weights, int n );

	/** Draws morphed base model. */
	void		draw();

	/** Returns true if the base model is visible. */
	bool		updateVisibility( const math::Matrix4x4& modelToCamera, 
					const ViewFrustum& viewFrustum );

	/** Uploads object to the rendering device. */
	void		load();

	/** Unloads object from the rendering device. */
	void		unload();

	/** Releases resources of the object. Object cannot be used after this. */
	void		destroy();

	/** Sets base model of the morph set. */
	void		setBase( Model* model );

	/** 
	 * Sets output model of the morph set. 
	 * If output model is not set then it is created when the morpher is first used.
	 */
	void		setOutput( Model* model );

	/** 
	 * Applies current state to the output model. Creates the output model
	 * if not already exist.
	 * @param reset If true then output model is reset to initial state before applying current state.
	 */
	void		apply( bool reset );

	/** Adds a morph target to the set. */
	void		addTarget( MorphTarget* model );

	/** Sets morph target weight. */
	void		setTargetWeight( const lang::String& name, float weight );

	/** Sets morph target weight animation. */
	void		setTargetWeightController( const lang::String& name, anim::VectorInterpolator* anim );

	/** Returns base model bounding sphere. */
	float		boundSphere() const;

	/** Returns number of used bones by this primitive. */
	int			usedBones() const;

	/** Returns array of used bones by this primitive. */
	const int*	usedBoneArray() const;

	/** Returns number of morph targets in the set. */
	int			targets() const;

	/** Returns ith morph target weight animation if any. */
	anim::VectorInterpolator*	getTargetWeightController( int i ) const;

	/** Returns base model of the morph set if any. */
	Model*		base() const;

	/** Returns output model of the morph if any. */
	Model*		output() const;

	/** Returns true if the model is valid base for all morph targets in this morpher. */
	bool		isValidBase( const Model* model ) const;

	/** Returns animation end time in seconds. */
	float		endTime() const;

	/** Returns ith morph target. */
	MorphTarget*	getTarget( int i ) const;

	/** Returns vertex format used by this geometry. */
	VertexFormat	vertexFormat() const;

private:
	class MorpherImpl;
	friend class MorpherImpl;
	P(MorpherImpl)		m_this;

	Morpher( Morpher& );
	Morpher& operator=( Morpher& );
};


} // sg


#endif // _SG_MORPHER_H
