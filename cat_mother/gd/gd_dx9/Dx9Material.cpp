#include "StdAfx.h"
#include "Dx9Material.h"
#include "Dx9Texture.h"
#include "Dx9CubeTexture.h"
#include "toDx9.h"
#include "Dx9GraphicsDevice.h"
#include "error.h"
#include <assert.h>
#include "config.h"

//----------------------------------------------------------------------------

#define INVALID_STATEBLOCK DWORD(-1)

//-----------------------------------------------------------------------------

using namespace gd;

//-----------------------------------------------------------------------------

Dx9Material::Dx9Material() :
	m_refs(0)
{
	m_changed = true;

	setBlend( BLEND_ONE, BLEND_ZERO );
	setDepthEnabled( true );
	setDepthWrite( true );
	setDepthFunc( CMP_LESSEQUAL );
	setZBias( 0 );
	setCull( CULL_CCW );
	setSpecularEnabled( false );
	setLighting( true );
	setVertexColor( false );
	setFogDisabled( false );
	setDiffuseColorSource( MCS_MATERIAL );
	setSpecularColorSource( MCS_MATERIAL );
	setAmbientColorSource( MCS_MATERIAL );
	setEmissiveColorSource( MCS_MATERIAL );

	setTextureColorCombine( 0, TA_TEXTURE, TOP_MODULATE, TA_DIFFUSE );
	setTextureAlphaCombine( 0, TA_TEXTURE, TOP_SELECTARG1, TA_CURRENT );

	setStencil( false );
	setStencilFail( STENCILOP_KEEP );
	setStencilZFail( STENCILOP_KEEP );
	setStencilPass( STENCILOP_KEEP );
	setStencilFunc( CMP_ALWAYS );
	setStencilRef( 1 );
	setStencilMask( 0xFFFFFFFF );

	setAlphaTest( false, CMP_GREATEREQUAL, 1 );

	setPolygonSorting( false );

	m_vf = gd::VertexFormat();
	m_d3dfvf = -1;
	m_dev = 0;

	// used coordinate set must be equal to layer index in fixed pipeline
	for ( int i = 0 ; i < TEXTURE_LAYERS ; ++i )
		m_textureLayers[i].coordinateSet = (int8_t)i;
}

Dx9Material::~Dx9Material()
{
	destroyDeviceObject();
}

void Dx9Material::addReference()
{
	InterlockedIncrement( &m_refs );
}

void Dx9Material::release()
{
	if ( 0 == InterlockedDecrement( &m_refs ) )
		delete this;
}

void Dx9Material::duplicate( const Material* otherInterface )
{
	const Dx9Material& other = static_cast<const Dx9Material&>( *otherInterface );

	destroyDeviceObject();
	
	m_materialReflectance = other.m_materialReflectance;
	m_srcBlend = other.m_srcBlend;
	m_dstBlend = other.m_dstBlend;
	m_depthFunc = other.m_depthFunc;
	m_cull = other.m_cull;
	m_depthEnabled = other.m_depthEnabled;
	m_depthWrite = other.m_depthWrite;
	m_specular = other.m_specular;
	m_lighting = other.m_lighting;
	m_vertexColor = other.m_vertexColor;
	m_fogDisabled = other.m_fogDisabled;
	m_diffuseSource = other.m_diffuseSource;
	m_specularSource = other.m_specularSource;
	m_ambientSource = other.m_ambientSource;
	m_emissiveSource = other.m_emissiveSource;
	m_stencil = other.m_stencil;
	m_stencilFail = other.m_stencilFail;
	m_stencilZFail = other.m_stencilZFail;
	m_stencilPass = other.m_stencilPass;
	m_stencilFunc = other.m_stencilFunc;
	m_stencilRef = other.m_stencilRef;
	m_stencilMask = other.m_stencilMask;
	m_sorting = other.m_sorting;
	m_zbias = other.m_zbias;
	m_alphaTestEnabled = other.m_alphaTestEnabled;
	m_alphaCompareFunc = other.m_alphaCompareFunc;
	m_alphaReferenceValue = other.m_alphaReferenceValue;
	m_d3dfvf = other.m_d3dfvf;
	m_vf = other.m_vf;

	for ( int i = 0 ; i < TEXTURE_LAYERS ; ++i )
		m_textureLayers[i] = other.m_textureLayers[i];

	m_changed = true;
}

void Dx9Material::setAlphaTest( bool alphaTestEnabled, CmpFunc alphaCompareFunc, int alphaReferenceValue )
{
	m_changed = true;
	m_alphaTestEnabled = (int8_t)(alphaTestEnabled ? 1 : 0);
	m_alphaCompareFunc = alphaCompareFunc;
	m_alphaReferenceValue = alphaReferenceValue;
}

void Dx9Material::setDiffuseColor( const pix::Colorf& color )							
{
	m_changed = true; 
	m_materialReflectance.diffuseColor = color;
}

void Dx9Material::setAmbientColor( const pix::Colorf& color )							
{
	m_changed = true; 
	m_materialReflectance.ambientColor = color;
}

void Dx9Material::setSpecularColor( const pix::Colorf& color )							
{
	m_changed = true; 
	m_materialReflectance.specularColor = color;
}

void Dx9Material::setSpecularExponent( float power )								
{
	m_changed = true; 
	m_materialReflectance.specularExponent = power;
}

void Dx9Material::setEmissiveColor( const pix::Colorf& color )							
{
	m_changed = true; 
	m_materialReflectance.emissiveColor = color;
}

void Dx9Material::setBlend( BlendMode src, BlendMode dst )						
{
	m_changed = true; 
	m_srcBlend = src; m_dstBlend = dst;
}

void Dx9Material::setDepthFunc( CmpFunc func )										
{
	m_changed = true; 
	m_depthFunc = func;
}

void Dx9Material::setDepthWrite( bool enabled )										
{
	m_changed = true; 
	m_depthWrite = enabled;
}

void Dx9Material::setDepthEnabled( bool enabled )										
{
	m_changed = true; 
	m_depthEnabled = enabled;
}

void Dx9Material::setZBias( int ) 
{
}

void Dx9Material::setCull( CullMode mode )										
{
	m_changed = true; 
	m_cull = mode;
}

void Dx9Material::setSpecularEnabled( bool enabled )								
{
	m_changed = true; 
	m_specular = enabled;
}

void Dx9Material::setLighting( bool enabled )										
{
	m_changed = true; 
	m_lighting = enabled;
}

void Dx9Material::setVertexColor( bool enabled )									
{
	m_changed = true; 
	m_vertexColor = enabled;
}

void Dx9Material::setFogDisabled( bool disabled )									
{
	m_changed = true; 
	m_fogDisabled = disabled;
}

void Dx9Material::setDiffuseColorSource( MaterialColorSource source )				
{
	m_changed = true; 
	m_diffuseSource = source;
}

void Dx9Material::setSpecularColorSource( MaterialColorSource source )			
{
	m_changed = true; 
	m_specularSource = source;
}

void Dx9Material::setAmbientColorSource( MaterialColorSource source )				
{
	m_changed = true; 
	m_ambientSource = source;
}

void Dx9Material::setEmissiveColorSource( MaterialColorSource source )			
{
	m_changed = true; 
	m_emissiveSource = source;
}

void Dx9Material::setTextureCoordinateTransform( int textureLayer, TextureCoordinateTransformMode mode, const math::Matrix4x4& transform )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.coordinateTransform = mode;
	L.coordinateTransformMatrix = transform;
}

void Dx9Material::setTextureFilter( int textureLayer, Material::TextureFilterType mode )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.filter = mode;
}

void Dx9Material::getTextureColorCombine( int textureLayer, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const
{
	const TextureLayer& layer = getLayer(textureLayer);
	*arg1	= layer.cArg1;
	*op		= layer.cOp;
	*arg2	= layer.cArg2;
}

void Dx9Material::getTextureAlphaCombine( int textureLayer, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const
{
	const TextureLayer& layer = getLayer(textureLayer);
	*arg1	= layer.aArg1;
	*op		= layer.aOp;
	*arg2	= layer.aArg2;
}

void Dx9Material::getTextureCoordinateTransform( int textureLayer, TextureCoordinateTransformMode* mode, math::Matrix4x4* transform ) const
{
	const TextureLayer& layer = getLayer(textureLayer);
	*mode		= layer.coordinateTransform;
	*transform	= layer.coordinateTransformMatrix;
}

int Dx9Material::getTextureCoordinateSet( int textureLayer ) const
{
	const TextureLayer& layer = getLayer(textureLayer);
	return layer.coordinateSet;
}

Material::TextureCoordinateSourceType Dx9Material::getTextureCoordinateSource( int textureLayer ) const
{
	const TextureLayer& layer = getLayer(textureLayer);
	return layer.coordinateSource;
}

Material::TextureAddressMode Dx9Material::getTextureAddress( int textureLayer ) const
{
	const TextureLayer& layer = getLayer(textureLayer);
	return layer.addressMode;
}

void Dx9Material::setTexture( int textureLayer, gd::BaseTexture* tex )
{
	m_changed = true;
	getLayer(textureLayer).setTexture( tex );
}

void Dx9Material::setTextureColorCombine( int textureLayer, 
	TextureArgument arg1, TextureOperation op, TextureArgument arg2 )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.cArg1	= arg1; 
	L.cOp	= op; 
	L.cArg2	= arg2;
}

void Dx9Material::setTextureAlphaCombine( int textureLayer, 
	TextureArgument arg1, TextureOperation op, TextureArgument arg2 )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.aArg1	= arg1; 
	L.aOp	= op; 
	L.aArg2	= arg2;
}

void Dx9Material::setTextureCoordinateSet( int textureLayer, int coordinateSetIndex )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.coordinateSet = (int8_t)coordinateSetIndex;
}

void Dx9Material::setTextureCoordinateSource( int textureLayer, TextureCoordinateSourceType source )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.coordinateSource = source;
}

void Dx9Material::setTextureAddress( int textureLayer, TextureAddressMode mode )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.addressMode = mode;
}

void Dx9Material::disableTextureLayer( int textureLayer )
{
	m_changed = true;
	TextureLayer& L = getLayer(textureLayer); 
	L.aOp = L.cOp = Material::TOP_DISABLE;
}

Dx9Material::BlendMode Dx9Material::sourceBlend() const									
{
	return m_srcBlend;
}

Dx9Material::BlendMode Dx9Material::destinationBlend() const								
{
	return m_dstBlend;
}

const pix::Colorf& Dx9Material::diffuseColor() const									
{
	return m_materialReflectance.diffuseColor;
}

bool Dx9Material::specularEnabled() const								
{
	return 0 != m_specular;
}

const pix::Colorf& Dx9Material::specularColor() const									
{
	return m_materialReflectance.specularColor;
}

float Dx9Material::specularExponent() const								
{
	return m_materialReflectance.specularExponent;
}

const pix::Colorf& Dx9Material::ambientColor() const									
{
	return m_materialReflectance.ambientColor;
}

const pix::Colorf& Dx9Material::emissiveColor() const									
{
	return m_materialReflectance.emissiveColor;
}

bool Dx9Material::lighting() const										
{
	return 0 != m_lighting;
}

bool Dx9Material::depthWrite() const											
{
	return 0 != m_depthWrite;
}

bool Dx9Material::depthEnabled() const
{
	return 0 != m_depthEnabled;
}

Dx9Material::CullMode Dx9Material::cull() const											
{
	return m_cull;
}

gd::BaseTexture* Dx9Material::getTexture( int textureLayer ) const
{
	assert(textureLayer>=0 && textureLayer<Material::TEXTURE_LAYERS); 
	return getLayer(textureLayer).texture();
}

bool Dx9Material::isTextureLayerEnabled( int textureLayer ) const		
{
	assert(textureLayer>=0 && textureLayer<Material::TEXTURE_LAYERS); 
	return getLayer(textureLayer).enabled();
}

void Dx9Material::setMaterial( Dx9GraphicsDevice* dev )
{
	const D3DCAPS9& caps = dev->caps();

	if ( !m_d3dfvf )
		m_d3dfvf = dev->getDeviceFVF( m_vf );

	Dx9RenderingState rs;
	rs.mixedVP = (int8_t)(m_vf.weights() > 0 ? 1 : 0);
	rs.d3dfvf = m_d3dfvf;
	rs.vertexWeights = m_vf.weights();
	rs.refl = m_materialReflectance;
	rs.srcBlend = m_srcBlend;
	rs.dstBlend = m_dstBlend;
	rs.depthEnabled = m_depthEnabled;
	rs.depthWrite = m_depthWrite;
	rs.depthFunc = m_depthFunc;
	rs.cull = m_cull;
	rs.specularEnabled = m_specular;
	rs.lighting = m_lighting;
	rs.vertexColor = m_vertexColor;
	rs.fogDisabled = m_fogDisabled;
	rs.diffuseSource = m_diffuseSource;
	rs.specularSource = m_specularSource;
	rs.ambientSource = m_ambientSource;
	rs.emissiveSource = m_emissiveSource;

	const int maxLayers	= TEXTURE_LAYERS < caps.MaxTextureBlendStages ? TEXTURE_LAYERS : caps.MaxTextureBlendStages;
	int i = 0;
	for ( ; i+1 < maxLayers && m_textureLayers[i].enabled() ; ++i )
		rs.textureLayers[i] = m_textureLayers[i];
	rs.textureLayers[i].aOp = Material::TOP_DISABLE;
	rs.textureLayers[i].cOp = Material::TOP_DISABLE;

	rs.stencil = m_stencil;
	rs.stencilFail = m_stencilFail;
	rs.stencilZFail = m_stencilZFail;
	rs.stencilPass = m_stencilPass;
	rs.stencilFunc = m_stencilFunc;
	rs.stencilRef = m_stencilRef;
	rs.stencilMask = m_stencilMask;

	rs.alphaTestEnabled = m_alphaTestEnabled;
	rs.alphaCompareFunc = m_alphaCompareFunc;
	rs.alphaReferenceValue = m_alphaReferenceValue;

	rs.polygonSorting = (uint8_t)(m_sorting ? 1 : 0);

	dev->setRenderState( rs );

	m_changed = false;
}

void Dx9Material::destroyDeviceObject()
{
}

Dx9Material::TextureLayer& Dx9Material::getLayer( int index )										
{
	assert(index>=0&&index<TEXTURE_LAYERS); 
	return m_textureLayers[index];
}

const Dx9Material::TextureLayer& Dx9Material::getLayer( int index ) const								
{
	assert(index>=0&&index<TEXTURE_LAYERS); 
	return m_textureLayers[index];
}

void Dx9Material::setStencil( bool enabled )
{
	m_changed = true; 
	m_stencil = enabled;
}

void Dx9Material::setStencilFail( StencilOperation sop )
{
	m_changed = true; 
	m_stencilFail = sop;
}

void Dx9Material::setStencilZFail( StencilOperation sop )
{
	m_changed = true; 
	m_stencilZFail = sop;
}

void Dx9Material::setStencilPass( StencilOperation sop )
{
	m_changed = true; 
	m_stencilPass = sop;
}

void Dx9Material::setStencilFunc( CmpFunc func )
{
	m_changed = true; 
	m_stencilFunc = func;
}

void Dx9Material::setStencilRef( int value )
{
	m_changed = true; 
	m_stencilRef = value;
}

void Dx9Material::setStencilMask( int mask )
{
	m_changed = true; 
	m_stencilMask = mask;
}

bool Dx9Material::stencil() const
{
	return 0 != m_stencil;
}

void Dx9Material::setPolygonSorting( bool enabled )
{
	m_changed = true; 
	m_sorting = enabled;
}

bool Dx9Material::polygonSorting() const
{
	return m_sorting;
}

void Dx9Material::setVertexFormat( const gd::VertexFormat& vf )
{
	m_vf = vf;
	m_d3dfvf = 0;
}

gd::VertexFormat Dx9Material::vertexFormat() const
{
	return m_vf;
}

void Dx9Material::begin( gd::GraphicsDevice* device, int* passes )
{
	assert( passes );
	assert( !m_dev );

	m_dev = static_cast<Dx9GraphicsDevice*>( device );
	*passes = 1;
}

void Dx9Material::apply( int /*pass*/ )
{
	assert( m_dev );

	setMaterial( m_dev );
}

void Dx9Material::end()
{
	assert( m_dev );

	m_dev->setDefaultRenderState();
	m_dev = 0;
}

bool Dx9Material::validate( gd::GraphicsDevice* device )
{
	Dx9GraphicsDevice* dev = static_cast<Dx9GraphicsDevice*>( device );
	int passes;
	begin( device, &passes );
	apply(0);
	bool valid = dev->validate();
	end();
	return valid;
}

//----------------------------------------------------------------------------

Dx9Material::TextureLayer::TextureLayer() : 
	m_tex(0)
{
	cArg1						= Material::TA_TEXTURE;
	cOp							= Material::TOP_DISABLE;
	cArg2						= Material::TA_TEXTURE;
	aArg1						= Material::TA_TEXTURE;
	aOp							= Material::TOP_DISABLE;
	aArg2						= Material::TA_TEXTURE;
	coordinateSet				= 0;
	coordinateSource			= Material::TCS_VERTEXDATA;
	coordinateTransform			= Material::TTFF_DISABLE;
	coordinateTransformMatrix	= math::Matrix4x4(1.f);
	addressMode					= Material::TADDRESS_WRAP;
	filter						= Material::TEXF_LINEAR;
}

Dx9Material::TextureLayer::TextureLayer( const Dx9Material::TextureLayer& other ) : 
	m_tex(0)
{
	*this = other;
}

Dx9Material::TextureLayer::~TextureLayer()
{
	setTexture( 0 );
}

Dx9Material::TextureLayer& Dx9Material::TextureLayer::operator=( const Dx9Material::TextureLayer& other )
{
	setTexture( other.m_tex );

	cArg1						= other.cArg1;
	cOp							= other.cOp;
	cArg2						= other.cArg2;
	aArg1						= other.aArg1;
	aOp							= other.aOp;
	aArg2						= other.aArg2;
	coordinateSet				= other.coordinateSet;
	coordinateSource			= other.coordinateSource;
	coordinateTransform			= other.coordinateTransform;
	coordinateTransformMatrix	= other.coordinateTransformMatrix;
	addressMode					= other.addressMode;
	filter						= other.filter;

	return *this;
}

void Dx9Material::TextureLayer::setTexture( gd::BaseTexture* tex )
{
	if ( tex )
		tex->addReference();

	if ( this->m_tex )
		this->m_tex->release();

	this->m_tex = tex;
}

bool Dx9Material::TextureLayer::enabled() const												
{
	return Material::TOP_DISABLE != cOp || Material::TOP_DISABLE != aOp;
}

//-----------------------------------------------------------------------------

Dx9Material::ReflectanceFactors::ReflectanceFactors()
{
	diffuseColor		= pix::Colorf(1,1,1);
	ambientColor		= pix::Colorf(1,1,1);
	specularColor		= pix::Colorf(0,0,0);
	emissiveColor		= pix::Colorf(0,0,0);
	specularExponent	= 0.f;
}

bool Dx9Material::ReflectanceFactors::operator==( const ReflectanceFactors& other ) const
{
	return 
		diffuseColor == other.diffuseColor &&
		ambientColor == other.ambientColor &&
		specularColor == other.specularColor &&
		emissiveColor == other.emissiveColor &&
		specularExponent == other.specularExponent;
}

bool Dx9Material::ReflectanceFactors::operator!=( const ReflectanceFactors& other ) const
{
	return 
		diffuseColor != other.diffuseColor ||
		ambientColor != other.ambientColor ||
		specularColor != other.specularColor ||
		emissiveColor != other.emissiveColor ||
		specularExponent != other.specularExponent;
}

