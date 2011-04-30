#include "GdUtil.h"
#include "config.h"
#include <sg/VertexFormat.h>
#include <gd/LightState.h>
#include <gd/VertexFormat.h>

//-----------------------------------------------------------------------------

namespace sg
{


void GdUtil::setLightState( gd::LightState& ls,
	gd::LightState::LightType type,
	const pix::Colorf& diff, const pix::Colorf& spec, const pix::Colorf& amb,
	const math::Vector3& position, const math::Vector3& direction,
	float range, float attenuation0, float attenuation1, float attenuation2,
	float falloff, float innerConeAngle, float outerConeAngle )
{
	ls.type				= type;
	ls.diffuse			= diff;
	ls.specular			= spec;
	ls.ambient			= amb;
	ls.position			= position;
	ls.direction		= direction;
	ls.range			= range;
	ls.falloff			= falloff;
	ls.attenuation0		= attenuation0;
	ls.attenuation1		= attenuation1;
	ls.attenuation2		= attenuation2;
	ls.theta			= innerConeAngle;
	ls.phi				= outerConeAngle;
}

void GdUtil::togd( const VertexFormat& vf, gd::VertexFormat* gdvf )
{
	*gdvf = *reinterpret_cast<const gd::VertexFormat*>( &vf );
}

void GdUtil::tosg( const gd::VertexFormat& gdvf, VertexFormat* vf )
{
	*vf = *reinterpret_cast<const VertexFormat*>( &gdvf );
}


} // sg
