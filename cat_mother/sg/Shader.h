#ifndef _SG_SHADER_H
#define _SG_SHADER_H


#include <sg/VertexFormat.h>
#include <sg/ContextObject.h>
#include <lang/String.h>


namespace sg {
	class BaseTexture;}

namespace pix {
	class Color;}

namespace math {
	class Vector4;
	class Matrix4x4;}


namespace sg
{


/**
 * Abstract base for geometry shaders.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Shader :
	public ContextObject
{
public:
	/** User parameter data type. */
	enum ParameterType
	{
		/** Unsupported parameter type. */
		PT_UNSUPPORTED,
		/** Boolean parameter. */
		PT_BOOL,
		/** Integer parameter. */
		PT_INT,
		/** Float parameter. */
		PT_FLOAT,
		/** Color parameter. */
		PT_COLOR,
		/** Texture parameter. */
		PT_TEXTURE,
		/** Texture sampler parameter. */
		PT_SAMPLER,
		/** Parameter type count. */
		PT_COUNT
	};

	/** User parameter data class. */
	enum ParameterClass
	{
		/** Unsupported parameter class. */
		PC_UNSUPPORTED,
		/** Scalar parameter. */
		PC_SCALAR,
		/** Vector4 parameter. */
		PC_VECTOR4,
		/** Matrix4x4 parameter. */
		PC_MATRIX4X4,
		/** Object parameter. */
		PC_OBJECT,
		/** Parameter class count. */
		PC_COUNT
	};

	/** Parameter description. */
	class ParameterDesc
	{
	public:
		/** Name (ASCII-7) of parameter. */
		lang::String	name;
		/** Type of parameter data. */
		ParameterType	dataType;
		/** Class of parameter object. */
		ParameterClass	dataClass;
		/** Number of elements if array parameter. 0 otherwise. */
		int				elements;
	};

	///
	Shader();

	///
	virtual ~Shader();

	/** Returns clone of this object. */
	virtual Shader*		clone() const = 0;

	/** 
	 * Begins rendering geometry using this shader. 
	 * @return Number of sub-passes needed to render this shader.
	 * @see apply
	 * @see end
	 */
	virtual int			begin() = 0;

	/** Ends rendering geometry using this shader. */
	virtual void		end() = 0;

	/** 
	 * Applies specified sub-pass of this shader to the active rendering device. 
	 * The first sub-pass is 0 and the last is the value returned by begin(), exclusive.
	 * @param pass Sub-pass to active.
	 * @see begin
	 * @see end
	 */
	virtual void		apply( int pass ) = 0;

	/** Sets name of the shader. */
	void				setName( const lang::String& name );

	/**
	 * Sets the rendering passes of the shader. Default is 1.
	 * @param pass Pass bits. The first pass is front-to-back and the rest are back-to-front.
	 */
	void				setPass( int pass );

	/** Sets vertex format used by this shader. */
	virtual void		setVertexFormat( const VertexFormat& vf ) = 0;

	/** Sets boolean user parameter to effect. */
	virtual void		setBoolean( const lang::String& name, bool value );

	/** Sets int user parameter to effect. */
	virtual void		setInt( const lang::String& name, int value );

	/** Sets float user parameter to effect. */
	virtual void		setFloat( const lang::String& name, float value );

	/** Sets 4-vector user parameter to effect. */
	virtual void		setVector4( const lang::String& name, const math::Vector4& value );

	/** Sets 4x4 matrix user parameter to effect. */
	virtual void		setMatrix4x4( const lang::String& name, const math::Matrix4x4& value );

	/** Sets 4x4 matrix array user parameter to effect. */
	virtual void		setMatrix4x4Array( const lang::String& name, const math::Matrix4x4* values, int count );

	/** Sets 4x4 matrix pointer array user parameter to effect. */
	virtual void		setMatrix4x4PointerArray( const lang::String& name, const math::Matrix4x4** values, int count );

	/** Sets color user parameter to effect. */
	virtual void		setColor( const lang::String& name, const pix::Color& value );

	/** Sets texture user parameter to effect. */
	virtual void		setTexture( const lang::String& name, sg::BaseTexture* value );

	/** 
	 * Gets 4-vector parameter value. 
	 * @exception Exception If parameter not found.
	 */
	virtual void		getVector4( const lang::String& name, math::Vector4* value ) const;

	/** Returns true if the shader has specified parameter. */
	virtual bool		hasParameter( const lang::String& name ) const;

	/** 
	 * Gets texture user parameter from effect. 
	 * @exception Exception If parameter not found.
	 */
	virtual sg::BaseTexture* getTexture( const lang::String& name ) const;

	/** Returns name of the shader. */
	const lang::String&	name() const;

	/** Returns the primary rendering passes of the shader. */
	int					pass() const;

	/** Returns vertex format used by this shader. */
	virtual VertexFormat vertexFormat() const = 0;

	/** Returns number of user parameters used by the shader. Default is 0. */
	virtual int			parameters() const;

	/** 
	 * Returns description  of ith user parameter used by the shader. 
	 * @param desc [out] Receives description of the parameter.
	 */
	virtual void		getParameterDesc( int i, ParameterDesc* desc ) const;

	/** Returns string description of parameter type. */
	static lang::String	toString( ParameterType pt );

	/** Returns string description of parameter class. */
	static lang::String	toString( ParameterClass pc );

protected:
	Shader( const Shader& other );

private:
	int				m_pass;
	lang::String	m_name;

	Shader& operator=( const Shader& other );
};


} // sg


#endif // _SG_SHADER_H
