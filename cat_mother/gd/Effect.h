#ifndef _GD_EFFECT_H
#define _GD_EFFECT_H


#include <gd/VertexFormat.h>


namespace pix {
	class Color;}

namespace math {
	class Vector4;
	class Matrix4x4;}


namespace gd
{


class BaseTexture;
class GraphicsDevice;


/** 
 * Interface to surface shader effect.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Effect
{
public:
	/** Handle to Effect parameter. */
	struct Parameter {};

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
	struct ParameterDesc
	{
		/** Name (ASCII-7) of parameter. */
		const char*		name;
		/** Type of parameter data. */
		ParameterType	dataType;
		/** Class of parameter object. */
		ParameterClass	dataClass;
		/** Number of elements if array parameter. 0 otherwise. */
		int				elements;
	};

	/** 
	 * Initializes the surface effect.
	 * @param data Pointer to ASCII-7 string description of the effect.
	 * @param size Size of the effect description.
	 * @return 0 if ok, error code otherwise. (see Errors.h)
	 */
	virtual int			create( gd::GraphicsDevice* device, const void* data, int size ) = 0;

	/** Deinitializes the rendering device explicitly. */
	virtual void		destroy() = 0;

	/** Copy by value. */
	virtual void		duplicate( const gd::Effect* other ) = 0;

	/** Increments reference count. */
	virtual void		addReference() = 0;

	/** Decrements reference count and deletes the object if no more references left. */
	virtual void		release() = 0;

	/** Sets bool parameter to effect. */
	virtual void		setBoolean( Parameter* param, bool value ) = 0;

	/** Sets int parameter to effect. */
	virtual void		setInt( Parameter* param, int value ) = 0;

	/** Sets color parameter to effect. */
	virtual void		setColor( Parameter* param, const pix::Color& value ) = 0;

	/** Sets float parameter to effect. */
	virtual void		setFloat( Parameter* param, float value ) = 0;

	/** Sets texture parameter to effect. */
	virtual void		setTexture( Parameter* param, gd::BaseTexture* value ) = 0;

	/** Sets 4x4 matrix parameter to effect. */
	virtual void		setMatrix4x4( Parameter* param, const math::Matrix4x4& value ) = 0;

	/** Sets 4x4 matrix parameter array to effect. */
	virtual void		setMatrix4x4Array( Parameter* param, const math::Matrix4x4* values, int count ) = 0;

	/** Sets 4x4 matrix parameter pointer array to effect. */
	virtual void		setMatrix4x4PointerArray( Parameter* param, const math::Matrix4x4** values, int count ) = 0;

	/** Sets 4-vector parameter to effect. */
	virtual void		setVector4( Parameter* param, const math::Vector4& value ) = 0;

	/** 
	 * Begins rendering the effect.
	 * @param passes [out] Receives number of passes needed to render the effect.
	 */
	virtual void		begin( gd::GraphicsDevice* device, int* passes ) = 0;

	/**
	 * Sets ith pass to render the effect.
	 * begin() must be called before this.
	 */
	virtual void		apply( int pass ) = 0;

	/** 
	 * Ends rendering the effect and restores modified states.
	 */
	virtual void		end() = 0;

	/** Enables polygon sorting for this effect. */
	virtual void		setPolygonSorting( bool enabled ) = 0;

	/** Sets vertex format used with this effect. */
	virtual void		setVertexFormat( const gd::VertexFormat& vf ) = 0;

	/** Returns vertex format used with this effect. */
	virtual gd::VertexFormat vertexFormat() const = 0;

	/** Returns string describing last error if any or empty string otherwise. */
	virtual const char*	lastErrorString() const = 0;

	/** Returns number of user parameters used by the shader. Default is 0. */
	virtual int			parameters() const = 0;

	/** 
	 * Gets 4-vector parameter value. 
	 * @exception Exception If parameter not found.
	 */
	virtual void		getVector4( Parameter* param, math::Vector4* value ) const = 0;

	/** Returns handle to ith parameter. */
	virtual Parameter*	getParameter( int i, Parameter* parent=0 ) const = 0;

	/** Returns handle to ith parameter. */
	virtual Parameter*	getParameter( const char* name, Parameter* parent=0 ) const = 0;

	/** 
	 * Returns description  of ith user parameter used by the shader. 
	 * @param desc [out] Receives parameter description.
	 */
	virtual void		getParameterDesc( Parameter* param, ParameterDesc* desc ) const = 0;

protected:
	Effect() {}
	virtual ~Effect() {}

private:
	Effect( const Effect& );
	Effect& operator=( const Effect& );
};


} // gd


#endif // _GD_EFFECT_H
