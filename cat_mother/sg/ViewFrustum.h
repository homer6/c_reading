#ifndef _SG_VIEWFRUSTUM_H
#define _SG_VIEWFRUSTUM_H


#include <math/Vector4.h>


namespace sg
{


/** 
 * Camera view frustum. 
 * View frustum planes are (right, top, left, bottom, front, back).
 * Plane normals point away from the frustum.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ViewFrustum
{
public:
	/** Number of planes (6) defining the view frustum volume. */
	static const int PLANE_COUNT;

	/** Creates default view frustum. */
	ViewFrustum();

	/** Sets field of view (in radians). */
	void	setHorizontalFov( float horzFov );

	/** Sets front plane distance. */
	void	setFront( float front );
	
	/** Sets back plane distance. */
	void	setBack( float back );

	/** Returns front plane distance. */
	float	front() const;
	
	/** Returns back plane distance. */
	float	back() const;

	/** Returns horizontal field of view in radians. */
	float	horizontalFov() const;

	/** Returns vertical field of view in radians. */
	float	verticalFov() const;

	/** 
	 * Return front-plane view rectangle dimensions. 
	 * @param x Dimension is the camera space width/2 of the view on front plane.
	 * @param y Dimension is the camera space height/2 of the view on front plane.
	 * @param z Dimension is the camera space distance of the front plane.
	 */
	void	getViewDimensions( float* x, float* y, float* z ) const;

	/** 
	 * Returns view frustum planes (6) in camera space.
	 * Plane normals point away from the view frustum volume.
	 * @see PLANE_COUNT
	 */
	const math::Vector4*	planes() const;

private:
	float					m_front;
	float					m_back;
	float					m_horzFov;
	mutable math::Vector4	m_planes[6];
	mutable bool			m_planesDirty;

	void	refreshPlanes() const;
};


#include "ViewFrustum.inl"


} // sg


#endif // _SG_VIEWFRUSTUM_H
