#include "Material.h"
#include "Texture.h"
#include "CubeTexture.h"
#include "Context.h"
#include "GdUtil.h"
#include <gd/BaseTexture.h>
#include <gd/Material.h>
#include <gd/GraphicsDriver.h>
#include <gd/GraphicsDevice.h>
#include <dev/Profile.h>
#include <pix/Colorf.h>
#include <lang/Debug.h>
#include <lang/Format.h>
#include <math/Matrix4x4.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace sg
{


inline static gd::Material::BlendMode togd( Material::BlendMode v )
{
	return (gd::Material::BlendMode)v;
}

inline static gd::Material::CmpFunc togd( Material::CmpFunc v )
{
	return (gd::Material::CmpFunc)v;
}

inline static gd::Material::CullMode togd( Material::CullMode v )
{
	return (gd::Material::CullMode)v;
}

inline static gd::Material::MaterialColorSource togd( Material::MaterialColorSource v )
{
	return (gd::Material::MaterialColorSource)v;
}

inline static gd::Material::TextureArgument togd( Material::TextureArgument v )
{
	return (gd::Material::TextureArgument)v;
}

inline static gd::Material::TextureOperation togd( Material::TextureOperation v )
{
	return (gd::Material::TextureOperation)v;
}

inline static gd::Material::TextureCoordinateTransformMode togd( Material::TextureCoordinateTransformMode v )
{
	return (gd::Material::TextureCoordinateTransformMode)v;
}

inline static gd::Material::TextureCoordinateSourceType togd( Material::TextureCoordinateSourceType v )
{
	return (gd::Material::TextureCoordinateSourceType)v;
}

inline static gd::Material::TextureAddressMode togd( Material::TextureAddressMode v )
{
	return (gd::Material::TextureAddressMode)v;
}

inline static gd::Material::TextureFilterType togd( Material::TextureFilterType v )
{
	return (gd::Material::TextureFilterType)v;
}

inline static gd::Material::StencilOperation togd( sg::Material::StencilOperation sop )
{
	return (gd::Material::StencilOperation)sop;
}

inline static sg::Material::BlendMode tosg( gd::Material::BlendMode v )
{
	return (sg::Material::BlendMode)v;
}

inline static sg::Material::CmpFunc tosg( gd::Material::CmpFunc v )
{
	return (sg::Material::CmpFunc)v;
}

inline static sg::Material::CullMode tosg( gd::Material::CullMode v )
{
	return (sg::Material::CullMode)v;
}

inline static sg::Material::MaterialColorSource tosg( gd::Material::MaterialColorSource v )
{
	return (sg::Material::MaterialColorSource)v;
}

inline static sg::Material::TextureArgument tosg( gd::Material::TextureArgument v )
{
	return (sg::Material::TextureArgument)v;
}

inline static sg::Material::TextureOperation tosg( gd::Material::TextureOperation v )
{
	return (sg::Material::TextureOperation)v;
}

inline static sg::Material::TextureCoordinateTransformMode tosg( gd::Material::TextureCoordinateTransformMode v )
{
	return (sg::Material::TextureCoordinateTransformMode)v;
}

inline static sg::Material::TextureCoordinateSourceType tosg( gd::Material::TextureCoordinateSourceType v )
{
	return (sg::Material::TextureCoordinateSourceType)v;
}

inline static sg::Material::TextureAddressMode tosg( gd::Material::TextureAddressMode v )
{
	return (sg::Material::TextureAddressMode)v;
}

inline static sg::Material::TextureFilterType tosg( gd::Material::TextureFilterType v )
{
	return (sg::Material::TextureFilterType)v;
}

//-----------------------------------------------------------------------------

Material::Material() :
	m_mat( 0 ),
	m_flags( FLAG_DEFAULT )
{
	m_mat = Context::driver()->createMaterial();
}

Material::Material( const Material& other ) :
	Shader( other ),
	m_mat( 0 ),
	m_flags( other.m_flags )
{
	m_mat = Context::driver()->createMaterial();
	m_mat->duplicate( other.m_mat );

	for ( int i = 0 ; i < TEXTURE_LAYERS ; ++i )
		m_layers[i] = other.m_layers[i];
}

Shader* Material::clone() const
{
	return new Material( *this );
}

Material::~Material()
{
	destroy();
}

void Material::destroy()
{
	m_mat = 0;
	Shader::destroy();
}

void Material::load()
{
	for ( int i = 0 ; i < TEXTURE_LAYERS ; ++i )
	{
		if ( m_layers[i] )
			m_layers[i]->load();
	}
}

void Material::unload()
{
	for ( int i = 0 ; i < TEXTURE_LAYERS ; ++i )
	{
		if ( m_layers[i] )
			m_layers[i]->unload();
	}
}

int	Material::begin()
{
	assert( m_mat );
	assert( !(m_flags & FLAG_INBEGIN) );

	m_flags |= FLAG_INBEGIN;
	if ( !(m_flags & FLAG_ENABLED) )
		return 0;

	int passes;
	m_mat->begin( Context::device(), &passes );
	return passes;
}

void Material::end()
{
	assert( m_flags & FLAG_INBEGIN );

	m_mat->end();
	m_flags &= ~FLAG_INBEGIN;
}

void Material::apply( int pass )
{
	assert( m_mat );

	//dev::Profile pr( "Mtl.apply" );

	gd::GraphicsDevice* dev = Context::device();
	if ( dev )
		m_mat->apply( pass );
}

void Material::setDiffuseColor( const pix::Colorf& color )
{
	assert( m_mat );
	m_mat->setDiffuseColor( color );
}

void Material::setSpecularEnabled( bool enabled )
{
	assert( m_mat );
	m_mat->setSpecularEnabled( enabled );
}

void Material::setSpecularColor( const pix::Colorf& color )
{
	assert( m_mat );
	m_mat->setSpecularColor( color );
}

void Material::setSpecularExponent( float power )
{
	assert( m_mat );
	m_mat->setSpecularExponent( power );
}

void Material::setAmbientColor( const pix::Colorf& color )
{
	assert( m_mat );
	m_mat->setAmbientColor( color );
}

void Material::setEmissiveColor( const pix::Colorf& color )
{
	assert( m_mat );
	m_mat->setEmissiveColor( color );
}

void Material::setBlend( BlendMode src, BlendMode dst )
{
	assert( m_mat );

	m_mat->setBlend( togd(src), togd(dst) );

	if ( dst != BLEND_ZERO && 1 == pass() )
		setPass( Material::DEFAULT_TRANSPARENCY_PASS );
}

void Material::setDepthWrite( bool enabled )
{
	assert( m_mat );
	m_mat->setDepthWrite( enabled );
}

void Material::setDepthEnabled( bool enabled )
{
	assert( m_mat );
	m_mat->setDepthEnabled( enabled );
}

void Material::setDepthFunc( CmpFunc func )
{
	assert( m_mat );
	m_mat->setDepthFunc( togd(func) );
}

void Material::setCull( CullMode mode )
{
	assert( m_mat );
	m_mat->setCull( togd(mode) );
}

void Material::setLighting( bool enabled )
{
	assert( m_mat );
	m_mat->setLighting( enabled );
}

void Material::setVertexColor( bool enabled )
{
	assert( m_mat );
	m_mat->setVertexColor( enabled );
}

void Material::setFogDisabled( bool disabled )
{
	assert( m_mat );
	m_mat->setFogDisabled( disabled );
}

void Material::setDiffuseColorSource( MaterialColorSource source )
{
	assert( m_mat );
	m_mat->setDiffuseColorSource( togd(source) );
}

void Material::setSpecularColorSource( MaterialColorSource source )
{
	assert( m_mat );
	m_mat->setSpecularColorSource( togd(source) );
}

void Material::setAmbientColorSource( MaterialColorSource source )
{
	assert( m_mat );
	m_mat->setAmbientColorSource( togd(source) );
}

void Material::setEmissiveColorSource( MaterialColorSource source )
{
	assert( m_mat );
	m_mat->setEmissiveColorSource( togd(source) );
}

void Material::setTexture( int layerIndex, sg::BaseTexture* tex )
{
	assert( layerIndex >= 0 && layerIndex < TEXTURE_LAYERS );
	assert( m_mat );
		
	m_mat->setTexture( layerIndex, tex->baseTexture() );
	m_layers[layerIndex] = tex;
	m_flags |= FLAG_DIRTY;
}

void Material::setTextureColorCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 )
{
	assert( m_mat );
	m_mat->setTextureColorCombine( layerIndex, togd(arg1), togd(op), togd(arg2) );
}

void Material::setTextureAlphaCombine( int layerIndex, TextureArgument arg1, TextureOperation op, TextureArgument arg2 )
{
	assert( m_mat );
	m_mat->setTextureAlphaCombine( layerIndex, togd(arg1), togd(op), togd(arg2) );
}

void Material::setTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode mode, const math::Matrix4x4& transform )
{
	assert( m_mat );
	m_mat->setTextureCoordinateTransform( layerIndex, togd(mode), transform );
}

void Material::setTextureCoordinateSet( int layerIndex, int coordinateSetIndex )
{
	assert( m_mat );
	m_mat->setTextureCoordinateSet( layerIndex, coordinateSetIndex );
}

void Material::setTextureCoordinateSource( int layerIndex, TextureCoordinateSourceType source )
{
	assert( m_mat );
	m_mat->setTextureCoordinateSource( layerIndex, togd(source) );
}

void Material::setTextureAddress( int layerIndex, TextureAddressMode mode )
{
	assert( m_mat );
	m_mat->setTextureAddress( layerIndex, togd(mode) );
}

void Material::setTextureFilter( int layerIndex, TextureFilterType mode )
{
	assert( m_mat );
	m_mat->setTextureFilter( layerIndex, togd(mode) );
}

void Material::disableTextureLayer( int layerIndex )
{
	assert( m_mat );
	m_mat->disableTextureLayer( layerIndex );
}

const pix::Colorf& Material::diffuseColor() const
{
	assert( m_mat );
	return m_mat->diffuseColor();
}

bool Material::specularEnabled() const
{
	assert( m_mat );
	return m_mat->specularEnabled();
}

const pix::Colorf& Material::specularColor() const
{
	assert( m_mat );
	return m_mat->specularColor();
}

float Material::specularExponent() const
{
	assert( m_mat );
	return m_mat->specularExponent();
}

const pix::Colorf& Material::ambientColor() const
{
	assert( m_mat );
	return m_mat->ambientColor();
}

const pix::Colorf& Material::emissiveColor() const
{
	assert( m_mat );
	return m_mat->emissiveColor();
}

Material::BlendMode Material::sourceBlend() const
{
	assert( m_mat );
	return tosg( m_mat->sourceBlend() );
}

Material::BlendMode Material::destinationBlend() const
{
	assert( m_mat );
	return tosg( m_mat->destinationBlend() );
}

bool Material::lighting() const
{
	assert( m_mat );
	return m_mat->lighting();
}

bool Material::depthWrite() const
{
	assert( m_mat );
	return m_mat->depthWrite();
}

bool Material::depthEnabled() const
{
	assert( m_mat );
	return m_mat->depthEnabled();
}

Material::CullMode Material::cull() const
{
	assert( m_mat );
	return tosg( m_mat->cull() );
}

sg::BaseTexture* Material::getTexture( int layerIndex ) const
{
	assert( m_mat );
	assert( layerIndex >= 0 && layerIndex < TEXTURE_LAYERS );
	return m_layers[layerIndex];
}

bool Material::isTextureLayerEnabled( int layerIndex ) const
{
	assert( m_mat );
	assert( layerIndex >= 0 && layerIndex < TEXTURE_LAYERS );
	return m_layers[layerIndex] != 0;
}

void Material::getTextureColorCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const
{
	assert( m_mat );

	gd::Material::TextureArgument gdArg1, gdArg2;
	gd::Material::TextureOperation gdOp;
	m_mat->getTextureColorCombine( layerIndex, &gdArg1, &gdOp, &gdArg2 );
	*arg1 = tosg(gdArg1);
	*arg2 = tosg(gdArg2);
	*op = tosg(gdOp);
}

void Material::getTextureAlphaCombine( int layerIndex, TextureArgument* arg1, TextureOperation* op, TextureArgument* arg2 ) const
{
	assert( m_mat );

	gd::Material::TextureArgument gdArg1, gdArg2;
	gd::Material::TextureOperation gdOp;
	m_mat->getTextureAlphaCombine( layerIndex, &gdArg1, &gdOp, &gdArg2 );
	*arg1 = tosg(gdArg1);
	*arg2 = tosg(gdArg2);
	*op = tosg(gdOp);
}

void Material::getTextureCoordinateTransform( int layerIndex, TextureCoordinateTransformMode* mode, math::Matrix4x4* transform ) const
{
	assert( m_mat );

	gd::Material::TextureCoordinateTransformMode gdMode;
	m_mat->getTextureCoordinateTransform( layerIndex, &gdMode, transform );
	*mode = tosg(gdMode);
}

int	Material::getTextureCoordinateSet( int layerIndex ) const
{
	assert( m_mat );
	return m_mat->getTextureCoordinateSet( layerIndex );
}

Material::TextureCoordinateSourceType	Material::getTextureCoordinateSource( int layerIndex ) const
{
	assert( m_mat );
	return tosg( m_mat->getTextureCoordinateSource( layerIndex ) );
}

Material::TextureAddressMode Material::getTextureAddress( int layerIndex ) const
{
	assert( m_mat );
	return tosg( m_mat->getTextureAddress( layerIndex ) );
}

void Material::setStencil( bool enabled )
{
	assert( m_mat );
	m_mat->setStencil( enabled );
}

void Material::setStencilFail( StencilOperation sop )
{
	assert( m_mat );
	m_mat->setStencilFail( togd(sop) );
}

void Material::setStencilZFail( StencilOperation sop )
{
	assert( m_mat );
	m_mat->setStencilZFail( togd(sop) );
}

void Material::setStencilPass( StencilOperation sop )
{
	assert( m_mat );
	m_mat->setStencilPass( togd(sop) );
}

void Material::setStencilFunc( CmpFunc func )
{
	assert( m_mat );
	m_mat->setStencilFunc( togd(func) );
}

void Material::setStencilRef( int value )
{
	assert( m_mat );
	m_mat->setStencilRef( value );
}

void Material::setStencilMask( int mask )
{
	assert( m_mat );
	m_mat->setStencilMask( mask );
}

void Material::setPolygonSorting( bool enabled )
{
	assert( m_mat );
	m_mat->setPolygonSorting( enabled );
}

bool Material::polygonSorting() const
{
	assert( m_mat );
	return m_mat->polygonSorting();
}

void Material::setEnabled( bool enabled )
{
	m_flags &= ~FLAG_ENABLED;
	if ( enabled )
		m_flags |= ~FLAG_ENABLED;
}

void Material::setZBias( int bias )
{
	assert( m_mat );
	m_mat->setZBias( bias );
}

bool Material::stencil() const
{
	assert( m_mat );
	return m_mat->stencil();
}

void Material::setVertexFormat( const VertexFormat& vf )
{
	assert( m_mat );

	gd::VertexFormat gdvf;
	GdUtil::togd( vf, &gdvf );
	
	m_mat->setVertexFormat( gdvf );
}

VertexFormat Material::vertexFormat() const
{
	assert( m_mat );

	VertexFormat vf;
	gd::VertexFormat gdvf = m_mat->vertexFormat();
	GdUtil::tosg( gdvf, &vf );
	
	return vf;
}

String Material::toString() const
{
	String litStr;
	if ( lighting() )
		litStr = "LIT";
	else
		litStr = "UNLIT";

	int texlayers = 0;
	for ( ; texlayers < TEXTURE_LAYERS ; ++texlayers )
		if ( !isTextureLayerEnabled(texlayers) )
			break;

	return Format( "TEX{0}_{1}", texlayers, litStr ).format();
}

void Material::setAlphaTest( bool alphaTestEnabled, CmpFunc alphaCompareFunc, int alphaReferenceValue )
{
	assert( m_mat );

	m_mat->setAlphaTest( alphaTestEnabled, togd(alphaCompareFunc), alphaReferenceValue );
}


} // sg
