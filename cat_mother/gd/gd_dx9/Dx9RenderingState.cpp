#include "StdAfx.h"
#include "Dx9RenderingState.h"
#include "zero.h"
#include <memory.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;

//-----------------------------------------------------------------------------

Dx9RenderingState::Dx9RenderingState()
{
	invalidate();

	srcBlend			= Dx9Material::BLEND_ONE;
	dstBlend			= Dx9Material::BLEND_ZERO;
	depthEnabled		= 1;
	depthWrite			= 1;
	depthFunc			= Dx9Material::CMP_LESS;
	cull				= Dx9Material::CULL_CCW;
	lighting			= true;
	specularEnabled		= false;
	zero( refl );
	vertexColor			= 0;
	fogDisabled			= 0;
	diffuseSource		= Dx9Material::MCS_MATERIAL;
	specularSource		= Dx9Material::MCS_MATERIAL;
	ambientSource		= Dx9Material::MCS_MATERIAL;
	emissiveSource		= Dx9Material::MCS_MATERIAL;

	for ( int i = 0 ; i < Dx9Material::TEXTURE_LAYERS ; ++i )
	{
		Dx9Material::TextureLayer& l = textureLayers[i];
		l.setTexture( 0 );
		l.cArg1							= Dx9Material::TA_TEXTURE;
		l.cOp							= Dx9Material::TOP_DISABLE;
		l.cArg2							= Dx9Material::TA_DIFFUSE;
		l.aArg1							= Dx9Material::TA_TEXTURE;
		l.aOp							= Dx9Material::TOP_DISABLE;
		l.aArg2							= Dx9Material::TA_DIFFUSE;
		l.coordinateSet					= 0;
		l.coordinateSource				= Dx9Material::TCS_VERTEXDATA;
		l.coordinateTransform			= Dx9Material::TTFF_DISABLE;
		l.coordinateTransformMatrix		= math::Matrix4x4(0.f);
		l.addressMode					= Dx9Material::TADDRESS_WRAP;
		l.filter						= Dx9Material::TEXF_LINEAR;
	}

	// default for layer[0]: c=tex*dif, a=tex
	Dx9Material::TextureLayer& l = textureLayers[0];
	l.cOp				= Dx9Material::TOP_MODULATE;
	l.aOp				= Dx9Material::TOP_SELECTARG1;

	stencil				= 0;
	vertexWeights		= 0;
	mixedVP				= 0;
	polygonSorting		= 0;
	d3dfvf				= D3DFVF_XYZ;
	stencilFail			= Dx9Material::STENCILOP_KEEP;
	stencilZFail		= Dx9Material::STENCILOP_KEEP;
	stencilPass			= Dx9Material::STENCILOP_KEEP;
	stencilFunc			= Dx9Material::CMP_ALWAYS;
	stencilRef			= 1;
	stencilMask			= -1;
}

void Dx9RenderingState::invalidate()
{
	srcBlend		= (Dx9Material::BlendMode)(-1);
	dstBlend		= (Dx9Material::BlendMode)(-1);
	depthEnabled	= -1;
	depthWrite		= -1;
	depthFunc		= (Dx9Material::CmpFunc)(-1);
	cull			= (Dx9Material::CullMode)(-1);
	lighting		= -1;
	specularEnabled	= -1;
	memset( &refl, 0xFF, sizeof(refl) );
	vertexColor		= -1;
	fogDisabled		= -1;
	diffuseSource	= (Dx9Material::MaterialColorSource)(-1);
	specularSource	= (Dx9Material::MaterialColorSource)(-1);
	ambientSource	= (Dx9Material::MaterialColorSource)(-1);
	emissiveSource	= (Dx9Material::MaterialColorSource)(-1);

	for ( int i = 0 ; i < Dx9Material::TEXTURE_LAYERS ; ++i )
	{
		Dx9Material::TextureLayer& l = textureLayers[i];
		l.setTexture( 0 );
		l.cArg1							= (Dx9Material::TextureArgument)(-1);
		l.cOp							= (Dx9Material::TextureOperation)(-1);
		l.cArg2							= (Dx9Material::TextureArgument)(-1);
		l.aArg1							= (Dx9Material::TextureArgument)(-1);
		l.aOp							= (Dx9Material::TextureOperation)(-1);
		l.aArg2							= (Dx9Material::TextureArgument)(-1);
		l.coordinateSet					= -1;
		l.coordinateSource				= (Dx9Material::TextureCoordinateSourceType)(-1);
		l.coordinateTransform			= (Dx9Material::TextureCoordinateTransformMode)(-1);
		l.coordinateTransformMatrix		= math::Matrix4x4(0.f);
		l.addressMode					= (Dx9Material::TextureAddressMode)(-1);
		l.filter						= (Dx9Material::TextureFilterType)(-1);
	}

	stencil			= -1;
	vertexWeights	= -1;
	mixedVP			= -1;
	polygonSorting	= -1;
	d3dfvf			= -1;
	stencilFail		= (Dx9Material::StencilOperation)-1;
	stencilZFail	= (Dx9Material::StencilOperation)-1;
	stencilPass		= (Dx9Material::StencilOperation)-1;
	stencilFunc		= (Dx9Material::CmpFunc)-1;
	stencilRef		= 0;
	stencilMask		= 0;

	alphaTestEnabled = -1;
	alphaCompareFunc = (Dx9Material::CmpFunc)-1;
	alphaReferenceValue = -1;
}
