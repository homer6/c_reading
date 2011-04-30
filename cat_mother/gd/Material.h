#ifndef _GD_MATERIAL_H
#define _GD_MATERIAL_H


#include <gd/VertexFormat.h>


namespace math {
	class Matrix4x4;}

namespace pix {
	class Colorf;
	class Color;}


namespace gd
{


class BaseTexture;
class GraphicsDevice;


/**
 * The rendering state of the graphics device.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Material
{
public:
	/** Generic material constants. */
	enum Constants 
	{ 
		/** Maximum number of texture layers per material. */
		TEXTURE_LAYERS	= 8,
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
		STENCILOP_INVERT
	};

	/** Increments reference count by one. */
	virtual void		addReference() = 0;

	/** Decrements reference count by one and releases the object if no more references left. */
	virtual void		release() = 0;

	/** Copy by value. */
	virtual void		duplicate( const Material* other ) = 0;

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

	/** Returns true if material is compatible with the device. */
	virtual bool		validate( gd::GraphicsDevice* device ) = 0;

	/** Sets diffuse color of the rendered surface. */
	virtual void		setDiffuseColor( const pix::Colorf& color ) = 0;

	/** Enables/disables specular highlights. */
	virtual void		setSpecularEnabled( bool enabled ) = 0;
	
	/** Sets specular color of the rendered surface. */
	virtual void		setSpecularColor( const pix::Colorf& color ) = 0;
	
	/** Sets sharpness of specular highlight of the rendered surface. */
	virtual void		setSpecularExponent( float power ) = 0;
	
	/** Sets ambient color of the rendered surface. */
	virtual void		setAmbientColor( const pix::Colorf& color ) = 0;
	
	/** Sets emissive color of the rendered surface. */
	virtual void		setEmissiveColor( const pix::Colorf& color ) = 0;

	/** Sets source and destination alpha blending modes for the rendered surface. */
	virtual void		setBlend( BlendMode src, BlendMode dst ) = 0;

	/** Enables/disables depth buffering. */
	virtual void		setDepthEnabled( bool enabled ) = 0;

	/** Enables/disables depth buffer writing. */
	virtual void		setDepthWrite( bool enabled ) = 0;
	
	/** Sets depth buffer compare function. */
	virtual void		setDepthFunc( CmpFunc func ) = 0;

	/** Sets material Z-bias. Valid range is [0,16]. */
	virtual void		setZBias( int bias ) = 0;
	
	/** Sets face culling mode. */
	virtual void		setCull( CullMode mode ) = 0;
	
	/** Enables/disables surface lighting. */
	virtual void		setLighting( bool enabled ) = 0;

	/** Enables/disables per vertex color component usage. */
	virtual void		setVertexColor( bool enabled ) = 0;

	/** Forces fog to be always disabled for this material. */
	virtual void		setFogDisabled( bool disabled ) = 0;
	
	/** Sets diffuse color source of the rendered surface. */
	virtual void		setDiffuseColorSource( MaterialColorSource source ) = 0;
	
	/** Sets specular color source of the rendered surface. */
	virtual void		setSpecularColorSource( MaterialColorSource source ) = 0;
	
	/** Sets ambient color source of the rendered surface. */
	virtual void		setAmbientColorSource( MaterialColorSource source ) = 0;
	
	/** Sets emissive color source of the rendered surface. */
	virtual void		setEmissiveColorSource( MaterialColorSource source ) = 0;

	/** 
	 * Sets texture layer data source. 
	 * Increments reference count of the texture object.
	 * Pass 0 to disable texture. 
	 */
	virtual void		setTexture( int layerIndex, gd::BaseTexture* tex ) = 0;
	
	/** Sets how texture color channel gets combined with previous texture layer color channel. */
	virtual void		setTextureColorCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 ) = 0;
	
	/** Sets how texture alpha channel gets combined with previous texture layer alpha channel. */
	virtual void		setTextureAlphaCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 ) = 0;

	/** Sets texture coordinate data transformation mode and matrix for texture layer. Default mode is TTFF_DISABLE. */
	virtual void		setTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode mode, const math::Matrix4x4& transform ) = 0;
	
	/** Sets used texture coordinate set for texture layer. Default equals to layer index. */
	virtual void		setTextureCoordinateSet( int layerIndex, int coordinateSetIndex ) = 0;
	
	/** Sets texture coordinate data source for texture layer. Default is TCS_VERTEXDATA. */
	virtual void		setTextureCoordinateSource( int layerIndex, TextureCoordinateSourceType source ) = 0;
	
	/** Sets texture addressing mode for texture layer. Default is TADDRESS_WRAP. */
	virtual void		setTextureAddress( int layerIndex, TextureAddressMode mode ) = 0;

	/** Texture filter to be used when rendering the texture onto primitives. */
	virtual void		setTextureFilter( int layerIndex, TextureFilterType mode ) = 0;

	/** 
	 * Disables output from this texture stage and all stages with a higher index.
	 * Disabling texture layer is equal to setting both color
	 * and alpha combine operations to TOP_DISABLE.
	 */
	virtual void		disableTextureLayer( int layerIndex ) = 0;

	/** 
	 * Sets alpha test state. 
	 * Texture alpha component is compared to reference value with alphaCompareFunc.
	 * If result is false then pixel is skipped (early).
	 */
	virtual void		setAlphaTest( bool alphaTestEnabled, CmpFunc alphaCompareFunc, int alphaReferenceValue ) = 0;

	/** Enables/disables stenciling. */
	virtual void		setStencil( bool enabled ) = 0;

	/** Sets stencil operation to perform if stencil test fails. */
	virtual void		setStencilFail( StencilOperation sop ) = 0;

	/** Sets stencil operation to perform if stencil test passes but depth test fails. */
	virtual void		setStencilZFail( StencilOperation sop ) = 0;

	/** Sets stencil operation to perform if both stencil test and depth test passes. */
	virtual void		setStencilPass( StencilOperation sop ) = 0;

	/** 
	 * Sets stencil comparison function. 
	 * The function is used to compare stencil buffer to the reference value.
	 */
	virtual void		setStencilFunc( CmpFunc func ) = 0;

	/** Sets stencil reference value. Default is 0. */
	virtual void		setStencilRef( int value ) = 0;

	/**
	 * Mask applied to the reference value and each stencil buffer 
	 * entry to determine the significant bits for the stencil test.
	 * Default is 0xFFFFFFFF.
	 */
	virtual void		setStencilMask( int mask ) = 0;

	/** Enables/disables per polygon depth sorting. */
	virtual void		setPolygonSorting( bool enabled ) = 0;

	/** Sets vertex format used with this material. */
	virtual void		setVertexFormat( const gd::VertexFormat& vf ) = 0;

	/** Returns vertex format used with this material. */
	virtual gd::VertexFormat	vertexFormat() const = 0;

	/** Returns diffuse color of the rendered surface. */
	virtual const pix::Colorf&	diffuseColor() const = 0;

	/** Returns true if specular highlights are enabled. */
	virtual bool				specularEnabled() const = 0;

	/** Returns specular color of the rendered surface. */
	virtual const pix::Colorf&	specularColor() const = 0;

	/** Returns specular exponent of the rendered surface. */
	virtual float				specularExponent() const = 0;

	/** Returns ambient color of the rendered surface. */
	virtual const pix::Colorf&	ambientColor() const = 0;

	/** Returns emissive color of the rendered surface. */
	virtual const pix::Colorf&	emissiveColor() const = 0;

	/** Returns source alpha blending mode. */
	virtual BlendMode			sourceBlend() const = 0;
	
	/** Returns destination alpha blending mode. */
	virtual BlendMode			destinationBlend() const = 0;

	/** Returns true if lighting is enabled for this material. */
	virtual bool				lighting() const = 0;

	/** Returns true if depth buffering is enabled for this material. */
	virtual bool				depthEnabled() const = 0;

	/** Returns true if depth buffer write is enabled for this material. */
	virtual bool				depthWrite() const = 0;

	/** Returns face culling mode. */
	virtual CullMode			cull() const = 0;

	/** 
	 * Returns data source of specified texture layer or 0 if none. 
	 * Does not increment texture reference count.
	 */
	virtual gd::BaseTexture*	getTexture( int layerIndex ) const = 0;

	/** 
	 * Returns true if specified texture layer is enabled. 
	 * Texture layer is considered to be enabled if either
	 * color or alpha combine operation is different from TOP_DISABLE.
	 */
	virtual bool				isTextureLayerEnabled( int layerIndex ) const = 0;

	/** 
	 * Returns information about how texture color channel gets combined with previous texture layer color channel. 
	 * @param arg1 [out] Receives the first texture color combine operation argument of the layer.
	 * @param op [out] Receives the color combine operation type of the layer.
	 * @param arg2 [out] Receives the second texture color combine operation argument of the layer.
	 */
	virtual void				getTextureColorCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const = 0;
	
	/** 
	 * Returns information about how texture alpha channel gets combined with previous texture layer alpha channel. 
	 * @param arg1 [out] Receives the first texture alpha combine operation argument of the layer.
	 * @param op [out] Receives the alpha combine operation type of the layer.
	 * @param arg2 [out] Receives the second texture alpha combine operation argument of the layer.
	 */
	virtual void				getTextureAlphaCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const = 0;
	
	/** 
	 * Returns texture coordinate data transformation mode and matrix for texture layer. Default mode is TTFF_DISABLE. 
	 * @param mode [out] Receives texture coordinate transform mode of the layer.
	 * @param transform [out] Receives texture coordinate transform matrix of the layer.
	 */
	virtual void				getTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode* mode, math::Matrix4x4* transform ) const = 0;

	/** Returns used texture coordinate set for texture layer. Default equals to layer index. */
	virtual int					getTextureCoordinateSet( int layerIndex ) const = 0;

	/** Returns texture coordinate data source for texture layer. Default is TCS_VERTEXDATA. */
	virtual TextureCoordinateSourceType	getTextureCoordinateSource( int layerIndex ) const = 0;

	/** Returns texture addressing mode for texture layer. Default is TADDRESS_WRAP. */
	virtual TextureAddressMode	getTextureAddress( int layerIndex ) const = 0;

	/** Returns true if stenciling is enabled. */
	virtual bool				stencil() const = 0;

	/** Returns true if per polygon depth sorting is enabled. */
	virtual bool				polygonSorting() const = 0;

protected:
	Material() {}
	virtual ~Material() {}

private:
	Material( const Material& );
	Material& operator=( const Material& );
};


} // gd


#endif // _GD_MATERIAL_H
