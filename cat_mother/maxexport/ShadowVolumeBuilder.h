#ifndef _SHADOWVOLUMEBUILDER_H
#define _SHADOWVOLUMEBUILDER_H


#include <util/Vector.h>
#include <math/Matrix4x4.h>


class SgMesh;
class SgLight;
class ProgressInterface;


/** 
 * Class for computing shadow volumes. 
 *
 * Shadow volume consists of two triangle meshes: (generating) volume
 * and silhuette. Generating volume is set of triangles which
 * do not face to light. Silhuette is set of quads (triangle pairs) for
 * each silhuette edge. Triangle pair topology is (v0,v1,v2), (v0,v2,v3),
 * where v0,v1 are silhuette quad start points and v2,v3 end points.
 * 'End' refers to the shadow volume end further from light.
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ShadowVolumeBuilder :
	public lang::Object
{
public:
	/** 
	 * Creates a dynamic/static shadow from given mesh. 
	 * In case keylight does not exist then (0,-1,0) dummy direct light is used.
	 * forceDynamic parameter can be used if shadowed object needs 
	 * to be animatable even thought the object does not have animation now.
	 */
	ShadowVolumeBuilder( SgMesh* mesh, SgLight* keylight, bool forceDynamic );

	~ShadowVolumeBuilder();

	/** Recomputes silhuette from volume triangles and volume lengths. */
	void	refreshSilhuette();

	/** Project shadow volume ends to common plane. */
	void	projectVolumes();

	/** Sets silhuette to be recomputed from volume- and volume length data. */
	void	setSilhuetteDirty();

	/** Returns access to silhuette triangles (2 triangles per edge). */
	util::Vector<math::Vector3>&	silhuette();

	/** Returns access to shadow volume generating polygons. */
	util::Vector<math::Vector3>&	volume();

	/** Returns access to shadow volume lengths. */
	util::Vector<float>&			volumeLengths();

	/** Returns access to silhuette triangles (2 triangles per edge). */
	const util::Vector<math::Vector3>&	silhuette() const;

	/** Returns access to shadow volume generating polygons. */
	const util::Vector<math::Vector3>&	volume () const;

	/** Returns access to shadow volume lengths. */
	const util::Vector<float>&			volumeLengths() const;

	/** Returns light direction in model space. */
	const math::Vector3&			lightModel() const;

	/** Returns light direction in world space. */
	const math::Vector3&			lightWorld() const;

	/** Returns shadow volume end cap plane equation. */
	math::Vector4					capPlane() const;

	const lang::String&				modelName() const;
	float							shadowLength() const;
	bool							dynamicShadow() const;

	static void		optimize( util::Vector<P(SgMesh)>& meshes, 
						ProgressInterface* progress,
						float maxShadowSampleDistance,
						const math::Vector3& lightWorld );

private:
	math::Vector3				m_lightWorld;
	math::Vector3				m_lightModel;
	util::Vector<math::Vector3>	m_silhuette;
	util::Vector<math::Vector3>	m_volume;
	util::Vector<float>			m_volumeLengths;
	lang::String				m_modelName;
	float						m_shadowLength;
	bool						m_dynamicShadow;
	math::Vector3				m_capPlaneNormal;
	math::Vector3				m_capPlanePoint;
	math::Matrix4x4				m_worldToModel;
	bool						m_silhuetteDirty;

	ShadowVolumeBuilder( const ShadowVolumeBuilder& );
	ShadowVolumeBuilder& operator=( const ShadowVolumeBuilder& );
};


#endif // _SHADOWVOLUMEBUILDER_H
