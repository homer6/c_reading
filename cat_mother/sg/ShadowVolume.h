#ifndef _SG_SHADOWVOLUME_H
#define _SG_SHADOWVOLUME_H


#include <sg/Primitive.h>
#include <math/Matrix4x4.h>


namespace sg
{


class Model;
class TriangleList;


/** 
 * Shadow volume of Model primitive. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ShadowVolume :
	public Primitive
{
public:
	/** 
	 * Creates a dynamic shadow volume.
	 * Dynamic shadow volume must be rendered with the same
	 * mesh as the original model, or otherwise transformation
	 * errors will occur.
	 * @param model Source geometry for the shadow.
	 * @param light Light direction in world space.
	 * @param shadowLength Maximum length of the shadow volume.
	 */
	ShadowVolume( Model* model, const math::Vector3& light, float shadowLength );

	/** 
	 * Creates a static shadow volume from precomputed data.
	 * @param silhuette Silhuette (2 triangles per edge) of the shadow volume.
	 * @param volume Triangles generating the shadow volume.
	 * @param endCap Shadow volume end cap plane in model space.
	 * @param light Light direction in world space.
	 */
	ShadowVolume( TriangleList* silhuette, TriangleList* volume, 
		const math::Vector4& endCap, const math::Vector3& light );

	///
	ShadowVolume( const ShadowVolume& other, int shareFlags );

	///
	~ShadowVolume();

	///
	Primitive*	clone( int shareFlags ) const;

	/** Releases resources of the object. */
	void	destroy();

	/** Uploads object to the rendering device. */
	void	load();

	/** Unloads object from the rendering device. */
	void	unload();

	/** Sets dynamic shadow parameters. */
	void	setDynamicShadow( const math::Vector3& light, float shadowLength );

	/** Computes primitive visibility in the view frustum. */
	bool	updateVisibility( const math::Matrix4x4& modelToCamera, 
				const ViewFrustum& viewFrustum );

	/** Draws the primitive to the active rendering device. */
	void	draw();

	/** Sets how far shadow volume is translated from the camera to avoid flickering. */
	void	setViewOffset( float viewOffset );

	/** Returns shadow volume bounding radius. */
	float	boundSphere() const;

	/** Returns number of triangles in the shadow when it was last rendered. */
	int		shadowTriangles() const;

	/** Returns true if the shadow volume is animated. */
	bool	dynamicShadow() const;

	/** Returns vertex format used by this geometry. */
	VertexFormat vertexFormat() const;

	/** 
	 * Returns number of used bones by this primitive.
	 * Default is 0.
	 */
	int			usedBones() const;

	/** 
	 * Returns array of used bones by this primitive.
	 * If the primitive has no bones the return value is 0.
	 */
	const int*	usedBoneArray() const;

	/** Returns total number of build shadow silhuettes. */
	static int	buildSilhuettes();

	/** Returns total number of clipped shadow silhuettes. */
	static int	clippedSilhuettes();

	/** Returns total number of clipped shadow silhuette polygons. */
	static int	clippedSilhuetteQuads();

	/** Returns total number of clipped shadow volumes. */
	static int	clippedVolumes();

	/** Returns total number of polygons in clipped shadow volumes. */
	static int	clippedTriangleVolumes();

	/** Returns number of cap polygons. */
	static int	volumeCapPolygons();

	/** Returns total number of rendered shadows. */
	static int	renderedShadows();

	/** Returns total number of rendered shadows triangles. */
	static int	renderedShadowTriangles();

private:
	math::Vector3	m_light;
	float			m_shadowLength;
	P(TriangleList)	m_shadowSilhuette;
	P(TriangleList)	m_shadowVolume;
	P(Model)		m_dynamicModel;
	int				m_polys;
	math::Vector3	m_capPlaneNormal;
	math::Vector3	m_capPlanePoint;
	math::Matrix3x3	m_rot;
	float			m_viewOffset;

	ShadowVolume();
	ShadowVolume( const ShadowVolume& );
	ShadowVolume& operator=( const ShadowVolume& );
};


} // sg


#endif // _SG_SHADOWVOLUME_H
