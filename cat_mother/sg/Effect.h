#ifndef _SG_EFFECT_H
#define _SG_EFFECT_H


#include <sg/Shader.h>
#include <sg/VertexFormat.h>
#include <util/Vector.h>


namespace io {
	class InputStream;}

namespace gd {
	class GraphicsDevice;
	class Effect;}

namespace pix {
	class Colorf;
	class Color;}

namespace math {
	class Vector4;
	class Matrix4x4;}

namespace lang {
	class String;}


namespace sg
{


class BaseTexture;
class GraphicsDevice;


/**
 * Effect object can contain multiple techniques to render the same surface effect.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Effect :
	public Shader
{
public:
	/** Creates effect from ASCII-7 description. */
	explicit Effect( io::InputStream* in );

	/** Copy by value. */
	Effect( const Effect& other );

	///
	~Effect();

	/** Copy by value. */
	Shader*			clone() const;
	
	/** Destroys the object. */
	void			destroy();

	/** Does nothing (effect already loaded when created). */
	void			load();

	/** Uploads object to the rendering device. */
	void			load( const void* src, int srcBytes, const lang::String& srcName );

	/** Unloads object from the rendering device. */
	void			unload();

	/** 
	 * Begins rendering geometry using this shader. 
	 * @return Number of sub-passes needed to render this shader.
	 * @see apply
	 * @see end
	 */
	int				begin();

	/** Ends rendering geometry using this shader. */
	void			end();

	/** 
	 * Applies specified sub-pass of this shader to the active rendering device. 
	 * The first sub-pass is 0 and the last is the value returned by begin(), exclusive.
	 * @param pass Sub-pass to active.
	 * @see begin
	 * @see end
	 */
	void			apply( int pass );

	/** Sets int parameter to effect. */
	void			setInt( const lang::String& name, int value );

	/** Sets float parameter to effect. */
	void			setFloat( const lang::String& name, float value );

	/** Sets 4-vector parameter to effect. */
	void			setVector4( const lang::String& name, const math::Vector4& value );

	/** Sets 4x4 matrix parameter to effect. */
	void			setMatrix4x4( const lang::String& name, const math::Matrix4x4& value );

	/** Sets 4x4 matrix parameter array to effect. */
	void			setMatrix4x4Array( const lang::String& name, const math::Matrix4x4* values, int count );

	/** Sets 4x4 matrix parameter pointer array to effect. */
	void			setMatrix4x4PointerArray( const lang::String& name, const math::Matrix4x4** values, int count );

	/** Sets color parameter to effect. */
	void			setColor( const lang::String& name, const pix::Color& value );

	/** Sets texture parameter to effect. */
	void			setTexture( const lang::String& name, sg::BaseTexture* value );

	/** Sets vertex format used by this shader. */
	void			setVertexFormat( const VertexFormat& vf );

	/** Enables polygon sorting for this effect. */
	void			setPolygonSorting( bool enabled );

	/** 
	 * Gets 4-vector parameter value. 
	 * @exception Exception If parameter not found.
	 */
	void			getVector4( const lang::String& name, math::Vector4* value ) const;

	/** Returns true if the effect has specified parameter. */
	bool			hasParameter( const lang::String& name ) const;

	/** 
	 * Gets texture parameter to effect. 
	 * @exception Exception If parameter not found.
	 */
	sg::BaseTexture*	getTexture( const lang::String& name ) const;

	/** Returns vertex format used by this shader. */
	VertexFormat	vertexFormat() const;

	/** Returns number of user parameters used by the shader. Default is 0. */
	int				parameters() const;

	/** 
	 * Returns description  of ith user parameter used by the shader. 
	 * @param desc [out] Receives description of the parameter.
	 */
	void			getParameterDesc( int i, ParameterDesc* desc ) const;

private:
	// Textures need to be stored in high level side also to keep reference alive.
	class TexParam
	{
	public:
		lang::String		name;
		P(sg::BaseTexture)	value;
	};

	P(gd::Effect)				m_fx;
	util::Vector<TexParam>		m_params;
	VertexFormat				m_vf;
	bool						m_sort;

	Effect& operator=( const Effect& other );
};


} // sg


#endif // _SG_EFFECT_H
