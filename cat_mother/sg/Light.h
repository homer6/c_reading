#ifndef _SG_LIGHT_H
#define _SG_LIGHT_H


#include <sg/Node.h>
#include <pix/Colorf.h>


namespace sg
{


/**
 * Base class for dynamic light sources in the scene graph.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Light : 
	public Node
{
public:
	/** Maximum light range. */
	static const float	MAX_RANGE;

	/** Maximum (spotlight) cone angle. */
	static const float	MAX_CONE_ANGLE;

	///
	~Light();

	/** Applies this light source to the active rendering device. */
	virtual void		apply() = 0;

	/** Sets diffuse color of the light. */
	void				setDiffuseColor( const pix::Colorf& diffuse );

	/** Sets specular color of the light. */
	void				setSpecularColor( const pix::Colorf& specular );
	
	/** Sets light intensity. Normalized range is [0,1]. */
	void				setAmbientColor( const pix::Colorf& ambient );

	/** Sets light intensity [0,1]. */
	void				setIntensity( float value );

	/** Returns diffuse color of the light. */	
	const pix::Colorf&	diffuseColor() const;
	
	/** Returns specular color of the light. */
	const pix::Colorf&	specularColor() const;
	
	/** Returns ambient color of the light. */
	const pix::Colorf&	ambientColor() const;

	/** Returns light intensity. Normalized range is [0,1]. */
	float				intensity() const;

protected:
	Light();
	Light( const Light& other );

private:
	pix::Colorf				m_diffuse;
	pix::Colorf				m_specular;
	pix::Colorf				m_ambient;
	float					m_intensity;

	void	assign( const Light& other );

	Light& operator=( const Light& other );
};


} // sg


#endif // _SG_LIGHT_H
