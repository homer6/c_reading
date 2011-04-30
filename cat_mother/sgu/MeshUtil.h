#ifndef _SGU_MESHUTIL_H
#define _SGU_MESHUTIL_H


namespace sg {
	class Node;
	class Model;
	class Mesh;}

namespace math {
	class OBBox;}


namespace sgu
{


/** 
 * Common mesh helper functions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MeshUtil
{
public:
	/** Splits large Model primitives to smaller ones. */
	static void	splitModels( sg::Mesh* mesh, int minPolys, float minSize );

	/** Splits model and adds splitted primitives to the mesh. */
	static void	splitModel( sg::Model* model, sg::Mesh* mesh );

	/** 
	 * Restores bones of cloned scene graph meshes. See Mesh::restoreBones(). 
	 * @exception Exception If assigned bone not found from the hierarchy.
	 */
	static void	restoreBones( sg::Node* root );

	/**
	 * Sets all shaders in hierarchy to use specified rendering passes.
	 * @param passSolid Rendering pass for solid geometry.
	 * @param passTransparent Rendering pass for transparent geometry.
	 */
	static void	setRenderPass( sg::Node* root, int passSolid, int passTransparent );
};


} // sgu


#endif // _SGU_MESHUTIL_H
 