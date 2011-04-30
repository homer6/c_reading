#ifndef _SG_MORPHTARGET_H
#define _SG_MORPHTARGET_H


#include <sg/Primitive.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <math/Vector3.h>
#include <stdint.h>


namespace sg
{


class Model;


/**
 * Target of morphing.
 * @see Morpher
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MorphTarget :
	public Primitive
{
public:
	/** Internal encoding used for the deltas. */
	struct Delta
	{
		uint16_t	vertexIndex;
		uint16_t	dx;
		uint16_t	dy;
		uint16_t	dz;
	};

	MorphTarget();
	~MorphTarget();

	MorphTarget( const MorphTarget& other, int shareFlags );

	Primitive* clone( int shareFlags ) const;

	/** Uploads object to the rendering device. Does nothing as MorphTargets are never uploaded. */
	void	load();

	/** Unloads object from the rendering device. Does nothing as MorphTargets are never uploaded. */
	void	unload();

	/** Releases resources of the object. Object cannot be used after this. */
	void	destroy();

	/** Does nothing as morph targets are not rendered. */
	void	draw();

	/** Sets name of the morph target. */
	void	setName( const lang::String& name );

	/** Sets name of the material this morph target. */
	void	setMaterialName( const lang::String& name );

	/** 
	 * Adds a vertex delta to the target. 
	 * Scale (maximum length) of the deltas must be computed beforehand. 
	 * @param vertexIndex Index of the base model vertex to modify.
	 * @param delta Offset to the base model vertex.
	 * @param scale Maximum delta length of this morph target. Must be same for all deltas within same target.
	 */
	void	addDelta( int vertexIndex, const math::Vector3& delta, float scale );

	/** 
	 * Adds vertex deltas to the morph target. 
	 * See struct Delta and morph target implementation for encoding details.
	 */
	void	addDeltas( Delta* deltas, int count, float scale );

	/** 
	 * Applies weighted target to the specified base. 
	 * Base model must be locked for reading/writing. 
	 */
	void	apply( Model* model, float weight );

	/** Returns true if this morph target affects only valid model vertices. */
	bool	isValidBase( const Model* model ) const;

	/** Returns name of the morph target. */
	const lang::String&	name() const;

	/** Returns name of the material this morph target. */
	const lang::String&	materialName() const;

	/** Returns vertex format used by this geometry. */
	VertexFormat	vertexFormat() const;

private:
	util::Vector<Delta>	m_deltas;
	float				m_scale;			// maximum delta length / 16383
	lang::String		m_name;
	lang::String		m_materialName;

	MorphTarget( const MorphTarget& );
	MorphTarget& operator=( const MorphTarget& );
};


} // sg


#endif // _SG_MORPHTARGET_H
