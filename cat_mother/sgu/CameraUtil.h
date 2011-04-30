#ifndef _SGU_CAMERAUTIL_H
#define _SGU_CAMERAUTIL_H


namespace sg {
	class Node;
	class Camera;}

#include <util/Vector.h>
#include <math/Vector3.h>



namespace sgu
{


/** 
 * Common camera helper functions. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class CameraUtil
{
public:
	/** 
	 * Finds out proper camera front- and back plane distances.
	 * The function requires that all mesh bounding spheres are set properly.
	 */
	static void	setFrontAndBackPlanes( sg::Camera* camera, sg::Node* scene );

	/** 
	 * Converts (Lightwave) zoom-factor to field-of-view angles.
	 * @param zoom Zoom-factor.
	 * @param width Viewport width.
	 * @param height Viewport height.
	 * @param hfov [out] Receives horizontal field-of-view. (radians)
	 * @param vfov [out] Receives vertical field-of-view. (radians)
	 */ 
	static void	convertZoomToFov( float zoom, float width, float height,
							float* hfov, float* vfov );

	/**
	 * Picks a node from scene with ray.
	 * Only Meshes containing Models can be picked.
	 * If the Mesh node is part of a LOD then LOD is returned instead.
	 * @param root Root node where to start picking.
	 * @param rayPos Position of the ray in world space.
	 * @param rayDir Direction of the ray in world space.
	 * @param distance [out] Receives distance to picked position if not null.
	 * @return 0 if node not found.
	 */
	static sg::Node*	pick( sg::Node* root, const math::Vector3& rayPos, 
							const math::Vector3& rayDir,
							float* distance );
};


} // sgu


#endif // _SGU_CAMERAUTIL_H
 