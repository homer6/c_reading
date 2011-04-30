#ifndef _SG_SHADOWSHADER_H
#define _SG_SHADOWSHADER_H


#include <sg/Shader.h>


namespace sg
{


class Material;


/**
 * Shader for shadow volumes.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ShadowShader :
	public Shader
{
public:
	/** Generic shadow constants. */
	enum Constants 
	{ 
		/** Default rendering passes for shadow volumes. */
		DEFAULT_SHADOW_VOLUME_PASS		= (1<<2),
		/** Default rendering pass for shadow fill polygon. */
		DEFAULT_SHADOW_FILL_PASS		= (1<<3)
	};

	///
	ShadowShader();

	///
	ShadowShader( const ShadowShader& other );

	///
	~ShadowShader();

	/** Copy by value. */
	Shader*		clone() const;

	/** Releases resources of the object. */
	void		destroy();

	/** Uploads object to the rendering device. */
	void		load();

	/** Unloads object from the rendering device. */
	void		unload();

	/**	Returns number of passes needed to render this material. */
	int			begin();

	/**	Does nothing. */
	void		end();

	/** 
	 * Applies specified sub-pass of this shader to the active rendering device. 
	 * The first sub-pass is 0 and the last is the value returned by begin(), exclusive.
	 * @param pass Sub-pass to active.
	 * @see begin
	 * @see end
	 */
	void		apply( int pass );

	/** 
	 * Set to true if shadow volume polygons are stored in CCW point order. 
	 * Default is false.
	 */
	void		setFlip( bool flip );

	/**
	 * Set to true to use conventional stenciling method.
	 * Default is false.
	 */
	void		setOld( bool old );

	/** Sets vertex format used by this material. */
	void		setVertexFormat( const VertexFormat& vf );

	/** Returns vertex format used by this material. */
	VertexFormat vertexFormat() const;

private:
	P(Material)	m_mat;
	bool		m_flip;
	bool		m_old;

	ShadowShader& operator=( const ShadowShader& other );
};


} // sg


#endif // _SG_SHADOWSHADER_H
