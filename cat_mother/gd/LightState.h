#ifndef _GD_LIGHTSTATE_H
#define _GD_LIGHTSTATE_H


#include <pix/Colorf.h>
#include <math/Vector3.h>


namespace gd
{


/** 
 * Description of light properties.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class LightState
{
public:
	/** 
	 * Type of light source. 
	 */
	enum LightType
	{
		/** Point light. */
		LIGHT_POINT,
		/** Spot light. */
		LIGHT_SPOT,
		/** Distant light. */
		LIGHT_DIRECT
	};

	/** Type of light source. */
    LightType			type;

	/** Diffuse color of the light source. */
    pix::Colorf			diffuse;

	/** Specular color of the light source. */
    pix::Colorf			specular;
    
	/** Ambient color of the light source. */
	pix::Colorf			ambient;
    
	/** World space position of the light source. */
	math::Vector3		position;
    
	/** World space direction of the light source. */
	math::Vector3		direction;
    
	/** World space range of the light source. */
	float				range;
    
	/** Decrease in illumination between theta and phi. */
	float				falloff;

    /** Constant attenuation. */
	float				attenuation0;
    
    /** Linear attenuation. */
	float				attenuation1;
    
    /** Quadratic attenuation. */
	float				attenuation2;
    
    /** Inner spotlight cone angle. */
	float				theta;
    
    /** Outer spotlight cone angle. */
	float				phi;
};


} // gd


#endif // _GD_LIGHTSTATE_H
