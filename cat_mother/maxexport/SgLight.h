#ifndef _SGLIGHT_H
#define _SGLIGHT_H


#include "SgNode.h"
#include <pix/Colorf.h>


/** 
 * Light source to be exported.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgLight :
	public SgNode
{
public:
	/** Type of light source. */
	enum LightType
	{
		/** Distant light source. */
		LIGHT_DIRECT,
		/** Omni light source. */
		LIGHT_POINT,
		/** Spot light source. */
		LIGHT_SPOT,
		/** Ambient light source. */
		LIGHT_AMBIENT
	};

	/** Type of light source. */
	LightType		type;
	/** Is light on/off. */
	bool			on;
	/** RGC color. */
	pix::Colorf		color;
	/** Intensity multiplier. */
	float			intensity;
	/** Spot light inner cone angle in degrees. */
	float			hotsize;
	/** Spot light outer cone angle in degrees. */
	float			fallsize;
	/** Near attenuation enabled/disabled. */
	bool			nearAtten;
	/** Start distance of near attenuation (if enabled). */
	float			nearAttenStart;
	/** End distance of near attenuation (if enabled). */
	float			nearAttenEnd;
	/** Far attenuation enabled/disabled. */
	bool			farAtten;
	/** Start distance of far attenuation (if enabled). */
	float			farAttenStart;
	/** End distance of far attenuation (if enabled). */
	float			farAttenEnd;
	/** Decay type: 0=none, 1=inverse, 2=inverse square. */
	float			decay;
	/** Decay radius from decay controller. */
	float			decayRadius;

	/** Temporary distance used for sorting lights. */
	float			tempDistance;

	///
	SgLight();

	///
	void	write( io::ChunkOutputStream* out ) const;

	/** Adds effect of this light to set of vertices. */
	void	lit( const math::Vector3* vertexWorldPositions, 
				const math::Vector3* vertexWorldNormals,
				math::Vector3* vertexDiffuseColors,
				int verts );

	/** 
	 * Returns attenuation of light between specified positions. 
	 * @return 1 if no attenuation, < 1 otherwise.
	 */
	float	getAttenuation( const math::Vector3& worldVertexPos,
				const math::Vector3& worldLightPos ) const;
};


#endif // _SGLIGHT_H
