#ifndef _SG_GDUTIL_H
#define _SG_GDUTIL_H


#include <gd/LightState.h>
#include <pix/Colorf.h>
#include <math/Vector3.h>


namespace gd {
	class VertexFormat;}


namespace sg
{


class VertexFormat;


/** 
 * Graphics device layer usage helpers. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GdUtil
{
public:
	/** 
	 * Initializes a description for a light source.
	 *
	 * @param diff Diffuse color emitted by the light.
	 * @param spec Specular color emitted by the light.
	 * @param amb Ambient color emitted by the light.
	 * @param range Distance beyond which the light has no effect. Maximum allowable value is maxRange().
	 * @param attenuation0 Constant attenuation. Value specifies how the light intensity changes over distance.
	 * @param attenuation1 Linear attenuation. Value specifies how the light intensity changes over distance.
	 * @param attenuation2 Quadratic attenuation. Value specifies how the light intensity changes over distance.
	 * @param falloff Decrease in illumination between a spotlight's inner cone and the outer edge of the outer cone.
	 * @param innerConeAngle Angle, in radians, of a fully illuminated spotlight cone.
	 * @param outerConeAngle Angle, in radians, defining the outer edge of the spotlight's outer cone. Points outside this cone are not lit by the spotlight.
	 */
	static void		setLightState( gd::LightState& ls,
						gd::LightState::LightType type,
						const pix::Colorf& diff, const pix::Colorf& spec, 
						const pix::Colorf& amb,
						const math::Vector3& position, 
						const math::Vector3& direction,
						float range, float attenuation0, 
						float attenuation1, float attenuation2,
						float falloff, float innerConeAngle, 
						float outerConeAngle );

	/** Converts sg VertexFormat to gd VertexFormat. */
	static void		togd( const VertexFormat& vf, gd::VertexFormat* gdvf );

	/** Converts gd VertexFormat to sg VertexFormat. */
	static void		tosg( const gd::VertexFormat& gdvf, VertexFormat* vf );
};


} // sg


#endif // _SG_GDUTIL_H
