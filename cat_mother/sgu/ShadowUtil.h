#ifndef _SGU_SHADOWUTIL_H
#define _SGU_SHADOWUTIL_H


#include <sg/Primitive.h>
#include <pix/Color.h>


namespace sg {
	class Mesh;
	class Node;}

namespace math {
	class Vector3;}


namespace sgu
{


/** 
 * Common shadow helper functions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ShadowUtil
{
public:
	/** 
	 * Creates a screen-space shadow filler primitive. One per scene. 
	 * @param color Color (including opacity) of the shadow.
	 * @param width Width of the shadow (in pixels).
	 * @param height Height of the shadow (in pixels).
	 */
	static P(sg::Primitive)	createShadowFiller( const pix::Color& opacity, 
								float width, float height );

	/**
	 * Sets shader of every shadow volume in the scene graph.
	 */
	static void		setShadowVolumeShaders( sg::Node* scene,
						sg::Shader* shadowShader );

	/** 
	 * Creates a dynamic shadow from the named source shadow mesh and links the shadow to the actual mesh. 
	 * <ol>
	 * <li>Unlinks root and shadowMesh.
	 * <li>Finds mesh called shadowName from the root node hierarchy.
	 * <li>Replaces shadowMesh primitives with ShadowVolume primitives created from Models of found shadowName mesh.
	 * <li>Links shadow mesh to root.
	 * </ol>
	 */
	static void		updateDynamicShadow( sg::Node* root, sg::Mesh* shadowMesh, 
						const lang::String& shadowName, 
						const math::Vector3& lightWorld, float shadowLength, float shadowViewOffset );

	/**
	 * Sets all shadow volume shaders in hierarchy to use specified rendering passes.
	 * @param passShadow Rendering pass for shadow volumes.
	 */
	static void		setShadowRenderPass( sg::Node* root, int passShadow );
};


} // sgu


#endif // _SGU_SHADOWUTIL_H
 