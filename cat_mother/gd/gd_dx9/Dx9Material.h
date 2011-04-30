#ifndef _DX8MATERIAL_H
#define _DX8MATERIAL_H


#include "DrvObject.h"
#include <gd/Material.h>
#include <pix/Color.h>
#include <pix/Colorf.h>
#include <math/Matrix4x4.h>
#include <stdint.h>


class Dx9BaseTexture;
class Dx9GraphicsDevice;


/**
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Dx9Material :
	public gd::Material,
	public DrvObject
{
public:
	class TextureLayer
	{
	public:
		TextureArgument						cArg1;
		TextureOperation					cOp;
		TextureArgument						cArg2;
		TextureArgument						aArg1;
		TextureOperation					aOp;
		TextureArgument						aArg2;
		int8_t								coordinateSet;
		TextureCoordinateSourceType			coordinateSource;
		TextureCoordinateTransformMode		coordinateTransform;
		TextureAddressMode					addressMode;
		TextureFilterType					filter;
		math::Matrix4x4						coordinateTransformMatrix;

		TextureLayer();
		TextureLayer( const TextureLayer& other );
		~TextureLayer();

		TextureLayer&		operator=( const TextureLayer& other );

		/** Sets texture and increases texture reference count. */
		void				setTexture( gd::BaseTexture* tex );

		/** Returns layer texture, or 0 if not set. */
		gd::BaseTexture*	texture() const											{return m_tex;}

		/** Returns true if the layer is enabled. */
		bool				enabled() const;

	private:
		gd::BaseTexture*	m_tex;
	};

	class ReflectanceFactors
	{
	public:
		pix::Colorf		diffuseColor;
		pix::Colorf		ambientColor;
		pix::Colorf		specularColor;
		pix::Colorf		emissiveColor;
		float			specularExponent;

		bool operator==( const ReflectanceFactors& other ) const;
		bool operator!=( const ReflectanceFactors& other ) const;

		ReflectanceFactors();
	};

	Dx9Material();
	~Dx9Material();
	
	void				addReference();
	void				release();
	void				duplicate( const gd::Material* other );

	void				begin( gd::GraphicsDevice* device, int* passes );
	void				apply( int pass );
	void				end();
	bool				validate( gd::GraphicsDevice* device );

	void				setDiffuseColor( const pix::Colorf& color );
	void				setSpecularEnabled( bool enabled );
	void				setSpecularColor( const pix::Colorf& color );
	void				setSpecularExponent( float power );
	void				setAmbientColor( const pix::Colorf& color );
	void				setEmissiveColor( const pix::Colorf& color );
	void				setBlend( BlendMode src, BlendMode dst );
	void				setDepthEnabled( bool enabled );
	void				setDepthWrite( bool enabled );
	void				setDepthFunc( CmpFunc func );
	void				setZBias( int bias );
	void				setCull( CullMode mode );
	void				setLighting( bool enabled );
	void				setVertexColor( bool enabled );
	void				setFogDisabled( bool disabled );
	void				setDiffuseColorSource( MaterialColorSource source );
	void				setSpecularColorSource( MaterialColorSource source );
	void				setAmbientColorSource( MaterialColorSource source );
	void				setEmissiveColorSource( MaterialColorSource source );
	void				setTexture( int layerIndex, gd::BaseTexture* tex );
	void				setTextureColorCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 );
	void				setTextureAlphaCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 );
	void				setTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode mode, const math::Matrix4x4& transform );
	void				setTextureCoordinateSet( int layerIndex, int coordinateSetIndex );
	void				setTextureCoordinateSource( int layerIndex, TextureCoordinateSourceType source );
	void				setTextureAddress( int layerIndex, TextureAddressMode mode );
	void				setTextureFilter( int layerIndex, TextureFilterType mode );
	void				disableTextureLayer( int layerIndex );
	void				setStencil( bool enabled );
	void				setStencilFail( StencilOperation sop );
	void				setStencilZFail( StencilOperation sop );
	void				setStencilPass( StencilOperation sop );
	void				setStencilFunc( CmpFunc func );
	void				setStencilRef( int value );
	void				setStencilMask( int mask );
	void				setPolygonSorting( bool enabled );
	void				setVertexFormat( const gd::VertexFormat& vf );
	void				setAlphaTest( bool alphaTestEnabled, CmpFunc alphaCompareFunc, int alphaReferenceValue );

	gd::VertexFormat	vertexFormat() const;
	const pix::Colorf&	diffuseColor() const;
	bool				specularEnabled() const;
	const pix::Colorf&	specularColor() const;
	float				specularExponent() const;
	const pix::Colorf&	ambientColor() const;
	const pix::Colorf&	emissiveColor() const;
	BlendMode			sourceBlend() const;
	BlendMode			destinationBlend() const;
	bool				lighting() const;
	bool				depthWrite() const;
	bool				depthEnabled() const;
	CullMode			cull() const;
	bool				stencil() const;
	gd::BaseTexture*	getTexture( int layerIndex ) const;
	bool				isTextureLayerEnabled( int layerIndex ) const;
	void				getTextureColorCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const;
	void				getTextureAlphaCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const;
	void				getTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode* mode, math::Matrix4x4* transform ) const;
	int					getTextureCoordinateSet( int layerIndex ) const;
	bool				polygonSorting() const;

	TextureCoordinateSourceType	getTextureCoordinateSource( int layerIndex ) const;
	TextureAddressMode			getTextureAddress( int layerIndex ) const;

	/** Returns true if the material has been changed since last apply(). */
	bool	modified() const														{return m_changed;}

private:
	// char is used to store booleans to be able to invalidate them with -1
	long						m_refs;
	ReflectanceFactors			m_materialReflectance;
	BlendMode					m_srcBlend;
	BlendMode					m_dstBlend;
	CmpFunc						m_depthFunc;
	CullMode					m_cull;
	char						m_depthEnabled;
	char						m_depthWrite;
	char						m_specular;
	char						m_lighting;
	char						m_vertexColor;
	char						m_fogDisabled;
	MaterialColorSource			m_diffuseSource;
	MaterialColorSource			m_specularSource;
	MaterialColorSource			m_ambientSource;
	MaterialColorSource			m_emissiveSource;
	char						m_stencil;
	StencilOperation			m_stencilFail;
	StencilOperation			m_stencilZFail;
	StencilOperation			m_stencilPass;
	CmpFunc						m_stencilFunc;
	int							m_stencilRef;
	int							m_stencilMask;
	bool						m_sorting;
	int8_t						m_zbias;
	int8_t						m_alphaTestEnabled;
	CmpFunc						m_alphaCompareFunc;
	int							m_alphaReferenceValue;
	TextureLayer				m_textureLayers[TEXTURE_LAYERS];
	int							m_d3dfvf;
	gd::VertexFormat			m_vf;
	mutable bool				m_changed;
	Dx9GraphicsDevice*			m_dev;

	TextureLayer&				getLayer( int index );
	const TextureLayer&			getLayer( int index ) const;

	/** Applies the material to the device. */
	void	setMaterial( Dx9GraphicsDevice* dev );

	void	destroyDeviceObject();

	Dx9Material( const Dx9Material& );
	Dx9Material& operator=( const Dx9Material& );
};


#endif // _DX8MATERIAL_H
