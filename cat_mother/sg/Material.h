#ifndef _SG_MATERIAL_H
#define _SG_MATERIAL_H


#include <sg/Shader.h>


namespace gd {
	class GraphicsDevice;
	class BaseTexture;
	class Material;}

namespace pix {
	class Colorf;
	class Color;}

namespace math {
	class Matrix4x4;}

namespace lang {
	class String;}


namespace sg
{


class BaseTexture;
class GraphicsDevice;


/**
 * Standard material. Describes rendering state of the device:
 * Material consists of various attributes which affect how
 * the surface gets rendered and how lights affect the surface.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Material :
	public Shader
{
public:
	/** Material flags. */
	enum Flags
	{
		/** Material is enabled. */
		FLAG_ENABLED	= 1,
		/** Material has been modified. */
		FLAG_DIRTY		= FLAG_ENABLED*2,
		/** Material needs to be rendered in multiple passes. */
		FLAG_MULTIPASS	= FLAG_DIRTY*2,
		/** begin() has been called. */
		FLAG_INBEGIN	= FLAG_MULTIPASS*2,
		/** Default flags. */
		FLAG_DEFAULT	= FLAG_ENABLED|FLAG_DIRTY
	};

	/** Generic material constants. */
	enum Constants 
	{ 
		/** Maximum number of texture layers per material. */
		TEXTURE_LAYERS				= 4,
		/** Default rendering pass for opaque materials. */
		DEFAULT_OPAQUE_PASS			= (1<<0),
		/** Default rendering pass for transparent materials. */
		DEFAULT_TRANSPARENCY_PASS	= (1<<5)
	};

	/** 
	 * Comparison functions. 
	 */
	enum CmpFunc
	{
		/** Fails always */
		CMP_NEVER, 
		/** Succeeds when the new value is nearer than the old one */
		CMP_LESS, 
		/** Succeeds when the new value is equal to the old one */
		CMP_EQUAL, 
		/** Succeeds when the new value is less or equal to the old one */
		CMP_LESSEQUAL, 
		/** Succeeds when the new value is greater than the old one */
		CMP_GREATER, 
		/** Succeeds when the new value is inequal to the old one */
		CMP_NOTEQUAL, 
		/** Succeeds when the new value is greater or equal to the old one */
		CMP_GREATEREQUAL, 
		/** Succeeds always */
		CMP_ALWAYS
	};

	/** 
	 * Source/destination alpha blending mode. 
	 *
	 * Rs,Gs,Bs,As refer to red, green, blue and alpha channels of the source color, respectively.
	 * Rd,Gd,Bd,Ad refer to red, green, blue and alpha channels of the destination color, respectively.
	 */
	enum BlendMode
	{
		/** Blend factor is (0, 0, 0, 0).  */
		BLEND_ZERO, 
		/** Blend factor is (1, 1, 1, 1).  */
		BLEND_ONE, 
		/** Blend factor is (Rs, Gs, Bs, As).  */
		BLEND_SRCCOLOR, 
		/** Blend factor is (1-Rs, 1-Gs, 1-Bs, 1-As).  */
		BLEND_INVSRCCOLOR, 
		/** Blend factor is (As, As, As, As).  */
		BLEND_SRCALPHA, 
		/** Blend factor is (1-As, 1-As, 1-As, 1-As).  */
		BLEND_INVSRCALPHA, 
		/** Blend factor is (Ad, Ad, Ad, Ad).  */
		BLEND_DESTALPHA, 
		/** Blend factor is (1-Ad, 1-Ad, 1-Ad, 1-Ad). */
		BLEND_INVDESTALPHA, 
		/** Blend factor is (Rd, Gd, Bd, Ad).  */
		BLEND_DESTCOLOR, 
		/** Blend factor is (1-Rd, 1-Gd, 1-Bd, 1-Ad). */
		BLEND_INVDESTCOLOR, 
		/** Blend factor is (f, f, f, 1), f = min(As, 1-Ad). */
		BLEND_SRCALPHASAT
	};

	/** 
	 * Material face culling mode.
	 */
	enum CullMode
	{
		/** Do not cull back faces */
		CULL_NONE,
		/** Cull faces with clockwise vertices */
		CULL_CW,
		/** Cull faces with counterclockwise vertices (default) */
		CULL_CCW
	};

	/** 
	 * Argument values for texture combine operations.
	 */
	enum TextureArgument
	{
		/** The texture argument is the diffuse color interpolated from vertex components during Gouraud shading. If the vertex does not contain a diffuse color, the default color is white. */
		TA_DIFFUSE,
		/** The texture argument is the result of the previous blending stage. */
		TA_CURRENT,
		/** The texture argument is the texture color for this texture stage.  */
		TA_TEXTURE,
		/** The texture argument is set to render state texture factor. */
		TA_TFACTOR,
		/** The texture argument is the specular color interpolated from vertex components during Gouraud shading. If the vertex does not contain a diffuse color, the default color is white. */
		TA_SPECULAR,
		/** Texture alpha replicated to color channels. */
		TA_TEXTUREALPHA
	};

	/** 
	 * Vertex texture coordinate data source. Use with texture coordinate transforms.
	 */
	enum TextureCoordinateSourceType
	{
		/** Use explicit texture coordinates specified in vertex data. */
		TCS_VERTEXDATA,
		/** Use the vertex normal, transformed to camera space, as the input texture coordinates. */
		TCS_CAMERASPACENORMAL,
		/** Use the vertex position, transformed to camera space, as the input texture coordinates. */
		TCS_CAMERASPACEPOSITION,
		/** Use the reflection vector, transformed to camera space, as the input texture coordinate. The reflection vector is computed from the input vertex position and normal vector. */
		TCS_CAMERASPACEREFLECTIONVECTOR
	};

	/** 
	 * Binary texture combine operation.
	 */
	enum TextureOperation
	{
		/** Disables output from this texture stage and all stages with a higher index. */
		TOP_DISABLE,
		/** Use this texture stage's first color or alpha argument, unmodified, as the output. */
		TOP_SELECTARG1,
		/** Use this texture stage's second color or alpha argument, unmodified, as the output. */
		TOP_SELECTARG2,

		/** Multiply the components of the arguments together. */
		TOP_MODULATE,
		/** Multiply the components of the arguments, and shift the products to the left 1 bit (effectively multiplying them by 2) for brightening. */
		TOP_MODULATE2X,
		/** Multiply the components of the arguments, and shift the products to the left 2 bits (effectively multiplying them by 4) for brightening.  */
		TOP_MODULATE4X,

		/** Add the components of the arguments */
		TOP_ADD,
		/** Add the components of the arguments with a -0.5 bias, making the effective range of values from -0.5 through 0.5.  */
		TOP_ADDSIGNED,
		/** Add the components of the arguments with a -0.5 bias, and shift the products to the left 1 bit.  */
		TOP_ADDSIGNED2X,
		/** Subtract the components of the second argument from those of the first argument.  */
		TOP_SUBTRACT,
		/** Add the first and second arguments, then subtract their product from the sum.  */
		TOP_ADDSMOOTH,

		/** Linearly blend this texture stage, using the interpolated alpha from each vertex. */
		TOP_BLENDDIFFUSEALPHA,
		/** Linearly blend this texture stage, using the interpolated alpha from texture. */
		TOP_BLENDTEXTUREALPHA,
		/** Linearly blend this texture stage, using the interpolated alpha from blend factor. */
		TOP_BLENDFACTORALPHA,
		/** Linearly blend a texture stage that uses a premultiplied alpha.  */
		TOP_BLENDTEXTUREALPHAPM,
		/** Linearly blend this texture stage, using the interpolated current alpha. */
		TOP_BLENDCURRENTALPHA,

		/** Modulate this texture stage with the next texture stage.  */
		TOP_PREMODULATE,  
		/** Modulate the color of the second argument, using the alpha of the first argument; then add the result to argument one. This operation is supported only for color operations. */
		TOP_MODULATEALPHA_ADDCOLOR,  
		/** Modulate the arguments; then add the alpha of the first argument. This operation is supported only for color operations. */
		TOP_MODULATECOLOR_ADDALPHA,  
		/** Similar to TOP_MODULATEALPHA_ADDCOLOR, but use the inverse of the alpha of the first argument. This operation is supported only for color operations. */
		TOP_MODULATEINVALPHA_ADDCOLOR,  
		/** Similar to TOP_MODULATECOLOR_ADDALPHA, but use the inverse of the color of the first argument. This operation is supported only for color operations. */
		TOP_MODULATEINVCOLOR_ADDALPHA,  

		/** Perform per-pixel bump mapping, using the environment map in the next texture stage (without luminance). This operation is supported only for color operations. */
		TOP_BUMPENVMAP, 
		/** Perform per-pixel bump mapping, using the environment map in the next texture stage (with luminance). This operation is supported only for color operations. */
		TOP_BUMPENVMAPLUMINANCE,
		/** Modulate the components of each argument (as signed components), add their products, then replicate the sum to all color channels, including alpha. */
		TOP_DOTPRODUCT3
	};

	/** 
	 * Texture coordinate data transformation mode.
	 *
	 * <i>TTFF_COUNTx</i> 
	 *
	 * <i>TTFF_COUNTx_PROJECTED</i> The texture coordinates are all divided by the last element before being passed to the rasterizer. For example, if TTFF_COUNT3_PROJECTED is used, the first and second texture coordinates will be divided by the third coordinate before being passed to the rasterizer. 
	 */
	enum TextureCoordinateTransformMode
	{
		/** Texture coordinates are passed directly to the rasterizer */
		TTFF_DISABLE,
		/** The rasterizer should expect 1-D texture coordinates. */
		TTFF_COUNT1,
		/** The rasterizer should expect 2-D texture coordinates. */
		TTFF_COUNT2,
		/** The rasterizer should expect 3-D texture coordinates. */
		TTFF_COUNT3,
		/** The rasterizer should expect 4-D texture coordinates. */
		TTFF_COUNT4,
		/** The rasterizer should expect 1-D texture coordinates which were generated from 2-D by dividing all elements with the last element. */
		TTFF_COUNT2_PROJECTED,
		/** The rasterizer should expect 2-D texture coordinates which were generated from 3-D by dividing all elements with the last element. */
		TTFF_COUNT3_PROJECTED,
		/** The rasterizer should expect 3-D texture coordinates which were generated from 4-D by dividing all elements with the last element. */
		TTFF_COUNT4_PROJECTED
	};

	/** 
	 * Defines vertex diffuse/specular/ambient color source used in rendering. 
	 */
	enum MaterialColorSource
	{
		/** Color source is material */
		MCS_MATERIAL,
		/** Color source is diffuse color component of the vertex */
		MCS_COLOR1,
		/** Color source is specular color component of the vertex */
		MCS_COLOR2
	};

	/** 
	 * Supported texture addressing modes. Addressing modes define
	 * how texture coordinates outside texture space [0,1] get handled.
	 */
	enum TextureAddressMode
	{
		/** Wrap texture coordinates on boundaries. For example 1.25 becomes 0.25. */
		TADDRESS_WRAP,
		/** Mirror texture coordinates on boundaries. For example 1.25 becomes 0.75. */
		TADDRESS_MIRROR,
		/** Clamp texture coordinates on boundaries. For example 1.25 becomes 1.00. */
		TADDRESS_CLAMP
	};

	/** Texture filter to be used when rendering the texture onto primitives. */
	enum TextureFilterType
	{
		/** Point filtering. The texel with coordinates nearest to the desired pixel value is used. */
		TEXF_POINT,
		/** Bilinear interpolation filtering. A weighted average of a 2×2 area of texels surrounding the desired pixel is used.  */
		TEXF_LINEAR,
		/** Anisotropic texture filtering. Compensates for distortion caused by the difference in angle between the texture polygon and the plane of the screen. */
		TEXF_ANISOTROPIC
	};

	/** Stencil buffer operation. */
	enum StencilOperation
	{
		/** Do not update the entry in the stencil buffer. */
		STENCILOP_KEEP,
		/** Set the stencil buffer entry to 0. */
		STENCILOP_ZERO,
		/** Replace the stencil buffer entry with reference value. */
		STENCILOP_REPLACE,
		/** Increment the stencil buffer entry, wrapping to zero. */
		STENCILOP_INCR,
		/** Decrement the stencil buffer entry, wrapping to the maximum. */
		STENCILOP_DECR,
		/** Invert the bits in the stencil buffer entry. */
		STENCILOP_INVERT,
	};

	///
	Material();

	/** Copy by value. */
	Material( const Material& other );

	///
	~Material();

	/** Copy by value. */
	Shader*		clone() const;

	/** Destroys the object. */
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

	/** Sets diffuse color. */
	void		setDiffuseColor( const pix::Colorf& color );

	/** Enables/disables specular highlights. */
	void		setSpecularEnabled( bool enabled );
	
	/** Sets specular color. */
	void		setSpecularColor( const pix::Colorf& color );
	
	/** Sets sharpness of specular highlight. */
	void		setSpecularExponent( float power );
	
	/** Sets ambient color. */
	void		setAmbientColor( const pix::Colorf& color );
	
	/** Sets emissive color. */
	void		setEmissiveColor( const pix::Colorf& color );

	/** 
	 * Sets source and destination alpha blending modes for the rendered surface. 
	 * If the destination blending mode is not BLEND_ZERO and
	 * rendering pass of this material is 1 then the pass is set to 
	 * DEFAULT_TRANSPARENCY_PASS.
	 */
	void		setBlend( BlendMode src, BlendMode dst );

	/** Enables/disables depth buffering. */
	void		setDepthEnabled( bool enabled );

	/** Enables/disables depth buffer writing. */
	void		setDepthWrite( bool enabled );
	
	/** Sets depth buffer compare function. */
	void		setDepthFunc( CmpFunc func );

	/** Sets material Z-bias. Valid range is [0,16]. */
	void		setZBias( int bias );

	/** Sets face culling mode. */
	void		setCull( CullMode mode );
	
	/** Enables/disables surface lighting. */
	void		setLighting( bool enabled );

	/** Enables/disables per vertex color component usage. */
	void		setVertexColor( bool enabled );

	/** Forces fog to be always disabled for this material. */
	void		setFogDisabled( bool disabled );

	/** Sets diffuse color source. */
	void		setDiffuseColorSource( MaterialColorSource source );
	
	/** Sets specular color source. */
	void		setSpecularColorSource( MaterialColorSource source );
	
	/** Sets ambient color source. */
	void		setAmbientColorSource( MaterialColorSource source );
	
	/** Sets emissive color source. */
	void		setEmissiveColorSource( MaterialColorSource source );

	/** Sets texture layer data source. */
	void		setTexture( int layerIndex, sg::BaseTexture* tex );
	
	/** Sets how texture color channel gets combined with previous texture layer color channel. */
	void		setTextureColorCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 );
	
	/** Sets how texture alpha channel gets combined with previous texture layer alpha channel. */
	void		setTextureAlphaCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 );

	/** Sets texture coordinate data transformation mode and matrix for texture layer. Default mode is TTFF_DISABLE. */
	void		setTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode mode, const math::Matrix4x4& transform );
	
	/** Sets used texture coordinate set for texture layer. Default equals to layer index. */
	void		setTextureCoordinateSet( int layerIndex, int coordinateSetIndex );
	
	/** Sets texture coordinate data source for texture layer. Default is TCS_VERTEXDATA. */
	void		setTextureCoordinateSource( int layerIndex, TextureCoordinateSourceType source );
	
	/** Sets texture addressing mode for texture layer. Default is TADDRESS_WRAP. */
	void		setTextureAddress( int layerIndex, TextureAddressMode mode );

	/** Sets texture filter to be used when rendering the texture onto primitives. */
	void		setTextureFilter( int layerIndex, TextureFilterType mode );

	/** 
	 * Disables output from this texture stage and all stages with a higher index.
	 * Disabling texture layer is equal to setting both color
	 * and alpha combine operations to TOP_DISABLE.
	 */
	void		disableTextureLayer( int layerIndex );

	/** 
	 * Sets alpha test state. 
	 * Texture alpha component is compared to reference value with alphaCompareFunc.
	 * If result is false then pixel is skipped (early).
	 */
	void		setAlphaTest( bool alphaTestEnabled, CmpFunc alphaCompareFunc, int alphaReferenceValue );

	/** Enables/disables stenciling. */
	void		setStencil( bool enabled );

	/** Sets stencil operation to perform if stencil test fails. */
	void		setStencilFail( StencilOperation sop );

	/** Sets stencil operation to perform if stencil test passes but depth test fails. */
	void		setStencilZFail( StencilOperation sop );

	/** Sets stencil operation to perform if both stencil test and depth test passes. */
	void		setStencilPass( StencilOperation sop );

	/** 
	 * Sets stencil comparison function. 
	 * The function is used to compare stencil buffer to the reference value.
	 */
	void		setStencilFunc( CmpFunc func );

	/** Sets stencil reference value. Default is 0. */
	void		setStencilRef( int value );

	/**
	 * Mask applied to the reference value and each stencil buffer 
	 * entry to determine the significant bits for the stencil test.
	 * Default is 0xFFFFFFFF.
	 */
	void		setStencilMask( int mask );

	/** Enables/disables per polygon depth sorting. */
	void		setPolygonSorting( bool enabled );

	/** Sets if the material is enabled (default). */
	void		setEnabled( bool enabled );

	/** Sets vertex format used by this material. */
	void		setVertexFormat( const VertexFormat& vf );
	
	/** Returns diffuse color. */
	const pix::Colorf&	diffuseColor() const;

	/** Returns true if specular highlights are enabled. */
	bool				specularEnabled() const;

	/** Returns specular color. */
	const pix::Colorf&	specularColor() const;

	/** Returns specular exponent. */
	float				specularExponent() const;

	/** Returns ambient color. */
	const pix::Colorf&	ambientColor() const;

	/** Returns emissive color. */
	const pix::Colorf&	emissiveColor() const;

	/** Returns source alpha blending mode. */
	BlendMode			sourceBlend() const;
	
	/** Returns destination alpha blending mode. */
	BlendMode			destinationBlend() const;

	/** Returns true if lighting is enabled for this material. */
	bool				lighting() const;

	/** Returns true if depth buffering is enabled for this material. */
	bool				depthEnabled() const;

	/** Returns true if Z-write is enabled for this material. */
	bool				depthWrite() const;

	/** Returns true if stenciling is enabled. */
	bool				stencil() const;

	/** Returns face culling mode. */
	CullMode			cull() const;

	/** Returns data source of specified texture layer. */
	sg::BaseTexture*	getTexture( int layerIndex ) const;

	/** 
	 * Returns true if specified texture layer is enabled. 
	 * Texture layer is considered to be enabled if either
	 * color or alpha combine operation is different from TOP_DISABLE.
	 */
	bool				isTextureLayerEnabled( int layerIndex ) const;

	/** 
	 * Returns information about how texture color channel gets combined with previous texture layer color channel. 
	 * @param arg1 [out] Receives the first texture color combine operation argument of the layer.
	 * @param op [out] Receives the color combine operation type of the layer.
	 * @param arg2 [out] Receives the second texture color combine operation argument of the layer.
	 */
	void				getTextureColorCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const;
	
	/** 
	 * Returns information about how texture alpha channel gets combined with previous texture layer alpha channel. 
	 * @param arg1 [out] Receives the first texture alpha combine operation argument of the layer.
	 * @param op [out] Receives the alpha combine operation type of the layer.
	 * @param arg2 [out] Receives the second texture alpha combine operation argument of the layer.
	 */
	void				getTextureAlphaCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const;
	
	/** 
	 * Returns texture coordinate data transformation mode and matrix for texture layer. Default mode is TTFF_DISABLE. 
	 * @param mode [out] Receives texture coordinate transform mode of the layer.
	 * @param transform [out] Receives texture coordinate transform matrix of the layer.
	 */
	void				getTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode* mode, math::Matrix4x4* transform ) const;

	/** Returns used texture coordinate set for texture layer. Default equals to layer index. */
	int					getTextureCoordinateSet( int layerIndex ) const;

	/** Returns texture coordinate data source for texture layer. Default is TCS_VERTEXDATA. */
	TextureCoordinateSourceType	getTextureCoordinateSource( int layerIndex ) const;

	/** Returns texture addressing mode for texture layer. Default is TADDRESS_WRAP. */
	TextureAddressMode	getTextureAddress( int layerIndex ) const;

	/** Returns true if per polygon depth sorting is enabled. */
	bool				polygonSorting() const;

	/** Returns true if the material is enabled (default). */
	bool				enabled() const;

	/** Returns vertex format used by this material. */
	VertexFormat		vertexFormat() const;

	/** Returns description. */
	lang::String		toString() const;

private:
	P(gd::Material)		m_mat;
	int					m_flags;
	P(BaseTexture)		m_layers[TEXTURE_LAYERS];

	Material& operator=( const Material& other );
};


#include "Material.inl"


} // sg


#endif // _SG_MATERIAL_H
