#ifndef _SG_CAMERA_H
#define _SG_CAMERA_H


#include <sg/Node.h>
#include <sg/ViewFrustum.h>
#include <math/Matrix4x4.h>


namespace sg
{


/**
 * Viewer node in scene graph.
 * Camera contains point-of-view, field-of-view, viewport and 
 * other attributes that desribe how the scene is rendered.
 * There can be multiple cameras in the scene graph.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Camera : 
	public Node
{
public:
	/** Generic camera constants. */
	enum Constants 
	{ 
		/** Last rendering pass (inclusive). */
		LAST_RENDERING_PASS	= (1<<6)
	};

	/** Creates default camera. */
	Camera();

	/** Copy by value. New camera is unlinked. */
	Camera( const Camera& other );

	///
	~Camera();

	Node*	clone() const;

	void	blendState( anim::Animatable** anims,
				const float* times, const float* weights, int n );

	/** 
	 * Sets target rectangle used to render to the frame buffer. 
	 * Device coordinates are used: Origin is top left, x grows right and y grows down. 
	 *
	 * @param x Leftmost (inclusive) column of the viewport.
	 * @param y Topmost (inclusive) row of the viewport.
	 * @param width Width of the viewport. Pass 0 to use back buffer width.
	 * @param height Height of the viewport. Pass 0 to use back buffer height.
	 */
	void	setViewport( int x, int y, int width, int height );

	/** Sets horizontal field of view (in radians). */
	void	setHorizontalFov( float horzFov );

	/** Sets front plane distance. */
	void	setFront( float front );
	
	/** Sets back plane distance. */
	void	setBack( float back );

	/** Renders the scene to active device. */
	void	render();

	/** Resets processing and rendering statistics. */
	void	resetStatistics();

	/** 
	 * Updates cachedWorldTransform(), cachedWorldToCamera(), cachedViewTransform(),
	 * cachedProjectionTransform() and cachedViewProjectionTransform(),
	 * Called before visibility checks in rendering.
	 * After this isInView can be used until camera transform is modified.
	 */
	void	updateCachedTransforms();

	/** 
	 * Returns target rectangle used to render to the frame buffer. 
	 * Device coordinates are used: Origin is top left, x grows right and y grows down. 
	 *
	 * @param x [out] Leftmost (inclusive) column of the viewport.
	 * @param y [out] Topmost (inclusive) row of the viewport.
	 * @param width [out] Width of the viewport. 0 if back buffer width is used and back buffer is not yet initialized.
	 * @param height [out] Height of the viewport. 0 if back buffer height is used and back buffer is not yet initialized.
	 */
	void	getViewport( int* x, int* y, int* width, int* height ) const;

	/** Returns width of the viewport in pixels. 0 if back buffer width is used and back buffer is not yet initialized. */
	int		viewportWidth() const;

	/** Returns height of the viewport in pixels. 0 if back buffer height is used and back buffer is not yet initialized. */
	int		viewportHeight() const;

	/** Returns center X of the viewport in pixels. 0 if back buffer width is used and back buffer is not yet initialized. */
	int		viewportCenterX() const;

	/** Returns center Y of the viewport in pixels. 0 if back buffer width is used and back buffer is not yet initialized. */
	int		viewportCenterY() const;

	/** Returns front plane distance. */
	float	front() const;
	
	/** Returns back plane distance. */
	float	back() const;

	/** Returns horizontal field of view in radians. */
	float	horizontalFov() const;

	/** Returns vertical field of view in radians. */
	float	verticalFov() const;

	/** 
	 * Returns size of an item in pixels after projection transformation.
	 * If the returned value is 0 then the viewport has not been set up.
	 * @param distanceZ Distance to the object along camera Z-axis.
	 * @param size Size (diameter/radius/whatever) of the object in 3D scene camera space along camera X-axis.
	 */
	float	getProjectedSize( float distanceZ, float size ) const;

	/**
	 * Returns world to camera transformation.
	 * The function can only be called during rendering.
	 */
	const math::Matrix4x4&	cachedWorldToCamera() const;

	/**
	 * Returns view transformation.
	 * The function can only be called during rendering.
	 */
	const math::Matrix4x4&	cachedViewTransform() const;

	/**
	 * Returns projection transformation.
	 * The function can only be called during rendering.
	 */
	const math::Matrix4x4&	cachedProjectionTransform() const;

	/**
	 * Returns view-projection transformation.
	 * The function can only be called during rendering.
	 */
	const math::Matrix4x4&	cachedViewProjectionTransform() const;

	/** Returns camera view frustum. */
	const ViewFrustum&		viewFrustum() const;

	/** Returns projection transform. */
	math::Matrix4x4			projectionTransform() const;

	/** Returns view (inverse world) transform. */
	math::Matrix4x4			viewTransform() const;

	/** Returns combined view-projection transform. */
	math::Matrix4x4			viewProjectionTransform() const;

	/** 
	 * Returns true if a sphere in world space is within the view frustum. 
	 * The function can only be called during rendering.
	 */
	bool	isInView( const math::Vector3& center, float radius ) const;

	/** Returns number of processed objects. */
	int		processedObjects() const;

	/** Returns number of lights used rendering. */
	int		renderedLights() const;
	
	/** Returns number of objects rendered. */
	int		renderedObjects() const;
	
	/** Returns number of primitives rendered. */
	int		renderedPrimitives() const;
	
	/** Returns number of triangles rendered. */
	int		renderedTriangles() const;

	/** Returns number of material changes. */
	int		materialChanges() const;

private:
	int						m_x;
	int						m_y;
	int						m_width;
	int						m_height;
	ViewFrustum				m_viewFrustum;
	mutable float			m_cachedWidthProjectionScalar;
	math::Matrix4x4			m_worldToCamera;
	math::Matrix4x4			m_view;
	math::Matrix4x4			m_projection;
	math::Matrix4x4			m_viewProjection;
	float					m_aspect;

	int						m_processedObjects;
	int						m_renderedLights;
	int						m_renderedObjects;
	int						m_renderedPrimitives;
	int						m_renderedTriangles;
	int						m_materialChanges;

	void	defaults();
	void	assign( const Camera& other );

	/**
	 * Returns true if sphere in camera space coordinates is inside view frustum.
	 * @param pos Sphere position in camera space.
	 * @param r Sphere radius in world space.
	 */
	bool	isSphereInViewFrustum( const math::Vector3& pos, float r ) const;

	/** Prepares everything for rendering the scene, see Camera.cpp. */
	void	prepareRender();

	Camera& operator=( const Camera& other );
};


} // sg


#endif // _SG_CAMERA_H
