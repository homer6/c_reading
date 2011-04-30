#ifndef _SG_SCENE_H
#define _SG_SCENE_H


#include <sg/Node.h>
#include <pix/Color.h>


namespace sg
{


/**
 * Class for controlling global scene parameters.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Scene :
	public Node
{
public:
	/** 
	 * Operation modes for traditional fog effect.
	 */
	enum FogMode
	{
		/** No fogging */
		FOG_NONE,
		/** Linear fogging */
		FOG_LINEAR,
		/** Exponential fogging */
		FOG_EXP,
		/** Squared exponential fogging */
		FOG_EXP2
	};

	///
	Scene();

	Node*		clone() const;

	/** Sets scene animation end in seconds. */
	void		setAnimationEnd( float time );

	/** 
	 * Sets fogging mode.  
	 */
	void		setFog( FogMode mode );

	/** 
	 * Sets color of the fog. 
	 */
	void		setFogColor( const pix::Color& color );
	
	/** 
	 * Sets fog start distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	void		setFogStart( float start );
	
	/** 
	 * Sets fog end distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	void		setFogEnd( float end );
	
	/** 
	 * Sets fog start density. Affects when fog mode is exp or exp^2. 
	 * Valid value range is between [0,1]. 
	 */
	void		setFogDensity( float density );

	/** Sets ambient color. */
	void		setAmbientColor( const pix::Color& color );

	/** Returns scene animation end in seconds. */
	float				animationEnd() const;

	/** Returns fogging mode. */
	FogMode				fog() const;

	/** Returns color of the fog (if fog is enabled). */
	const pix::Color&	fogColor() const;
	
	/** 
	 * Returns fog start distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	float				fogStart() const;
	
	/** 
	 * Returns fog end distance. Affects when fog mode is linear. 
	 * Value scale is same as with view frustum front and back plane distance.
	 */
	float				fogEnd() const;
	
	/** 
	 * Returns fog start density. Affects when fog mode is exp or exp^2. 
	 * Valid value range is between [0,1]. 
	 */
	float				fogDensity() const;

	/** Returns ambient color. */
	const pix::Color&	ambientColor() const;

private:
	FogMode							m_fog;
	float							m_fogStart;
	float							m_fogEnd;
	float							m_fogDensity;
	pix::Color						m_fogColor;
	pix::Color						m_ambientColor;
	float							m_animEnd;

	Scene( const Scene& );
	Scene& operator=( const Scene& );
};


} // sg


#endif // _SG_SCENE_H
