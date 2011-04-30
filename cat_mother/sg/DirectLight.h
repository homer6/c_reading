#ifndef _SG_DIRECTIONALLIGHT_H
#define _SG_DIRECTIONALLIGHT_H


#include <sg/Light.h>



namespace sg
{


/**
 * Dynamic directional light source in the scene graph. 
 * Good for simulating distant light sources like for example sun.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DirectLight : 
	public Light
{
public:
	///
	DirectLight();

	/** Creates a copy of this object. */
	DirectLight( const DirectLight& other );

	Node*			clone() const;

	/** Applies this light source to the active rendering device. */
	void			apply();

	/** 
	 * Returns direction of the light in world space.
	 * Use setRotation or setTransform of the Node class
	 * to set the direction of the light.
	 */
	math::Vector3	worldDirection() const;

private:
	DirectLight& operator=( const DirectLight& other );
};


} // sg


#endif // _SG_DIRECTIONALLIGHT_H
