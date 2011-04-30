#ifndef _SGMESH_H
#define _SGMESH_H


#include "SgNode.h"
#include <util/Vector.h>
#include <math/OBBox.h>
#include <math/Matrix4x4.h>


namespace bsp {
	class BSPNode;}


class GmModel;
class GmPatchList;
class ShadowVolumeBuilder;


/**
 * Mesh to be exported.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgMesh :
	public SgNode
{
public:
	/** Bone index and the rest transformation. */
	class Bone
	{
	public:
		/** Index of the bone or -1 if none. */
		int				index;
		/** Bone rest transformation. */
		math::Matrix4x4	rest;

		///
		Bone();
	};

	/** Geometry data. */
	GmModel*				model;

	/** Shadow volume. */
	ShadowVolumeBuilder*	shadow;

	/** Bounding box of the geometry. */
	math::OBBox				obb;

	/** 
	 * Level of detail group ID. 
	 * Identifies meshes that are different LODs of the same entity.
	 * If the mesh is no LOD then lodID is "".
	 */
	lang::String			lodID;

	/**
	 * Level of detail group ID index. 
	 */
	int						lodNum;

	/** Minimum limit (inclusive) of the LOD range, pixels. */
	float					lodMin;

	/** Maximum limit (inclusive) of the LOD range, pixels. */
	float					lodMax;

	/** Bones of this mesh. */
	util::Vector<Bone>		bones;

	SgMesh();
	~SgMesh();

	void	write( io::ChunkOutputStream* out ) const;
	bool	isAnimated() const;

private:
	SgMesh( const SgMesh& );
	SgMesh& operator=( const SgMesh& );
};



#endif // _SGMESH_H
