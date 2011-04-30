#ifndef _SG_MESH_H
#define _SG_MESH_H


#include <sg/Node.h>


namespace sg
{


class Primitive;
class Camera;


/**
 * Container for visual primitives.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Mesh : 
	public Node
{
public:
	///
	Mesh();

	/** Creates a copy of this object. */
	Mesh( const Mesh& other );

	///
	~Mesh();

	Node*		clone() const;

	/** Computes object visibility in the view frustum. */
	bool		updateVisibility( Camera* camera );

	/** Renders this mesh to the active device. */
	void		render( Camera* camera, int pass );

	/** Adds a visual primitive to the mesh. */
	void		addPrimitive( Primitive* primitive );

	/** Removes a visual primitive from the mesh. */
	void		removePrimitive( int index );

	/** Removes all visual primitives from the mesh. */
	void		removePrimitives();

	/** 
	 * Adds a bone to the mesh.
	 * @param bone Bone node.
	 * @param restTransform Transform from bone to skin space in non-deforming pose.
	 */
	void		addBone( Node* bone, const math::Matrix4x4& restTransform );

	/** 
	 * Sets bones identical to the other mesh. 
	 * Used for example adding shadow mesh to existing mesh (as child node).
	 */
	void		setBones( const Mesh* other );

	/**
	 * Restores bones of a clone by finding identical named nodes 
	 * from the current hierarchy. Use to meshes after cloning whole 
	 * scene graph, otherwise cloned scene graph meshes will use
	 * bones from the original scene graph.
	 * @param root Root of the cloned scene.
	 * @exception Exception If assigned bone not found from the hierarchy.
	 */
	void		restoreBones( Node* root );

	/** Removes a bone from the mesh. */
	void		removeBone( int index );

	/** Returns specified visual primitive of the mesh. */
	Primitive*	getPrimitive( int index ) const;

	/** Returns number of visual primitives in the mesh. */
	int			primitives() const;

	/** Returns a bone from the mesh. */
	Node*		getBone( int index ) const;

	/** Returns transform from skin space to bone space. */
	const math::Matrix4x4&	getBoneInverseRestTransform( int index ) const;

	/** 
	 * Returns model->world transforms for mesh (at index 0) and n bones.
	 * Matrices are returned in 4x3 format (translation at row 3, X-axis at column 0).
	 * @param count Space for transforms, must be bones()+1.
	 */
	void		getBoneMatrix4x3Array( math::Matrix4x4* tm, int count ) const;

	/** Returns number of bones in the mesh. */
	int			bones() const;

	/** 
	 * Returns union of primitive bounding spheres. 
	 * Requires that the primitives are not locked.
	 */
	float		boundSphere() const;

private:
	class MeshImpl;
	P(MeshImpl) m_this;

	Mesh& operator=( const Mesh& other );
};


} // sg


#endif // _SG_MESH_H
