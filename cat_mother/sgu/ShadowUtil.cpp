#include "ShadowUtil.h"
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/Material.h>
#include <sg/VertexLock.h>
#include <sg/ShadowVolume.h>
#include <sg/ShadowShader.h>
#include <sg/VertexFormat.h>
#include <sg/TriangleList.h>
#include <math/Vector4.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace pix;
using namespace math;

//-----------------------------------------------------------------------------

namespace sgu
{


P(Primitive) ShadowUtil::createShadowFiller( const Color& color, 
	float width, float height )
{
	VertexFormat fillvf;
	fillvf.addRHW().addDiffuse();

	// material
	P(Material) fillmat = new Material;
	fillmat->setDepthEnabled( false );
	fillmat->setDepthWrite( false );
	fillmat->setBlend( Material::BLEND_SRCALPHA, Material::BLEND_INVSRCALPHA );
	fillmat->setTextureColorCombine( 0, Material::TA_DIFFUSE, Material::TOP_SELECTARG1, Material::TextureArgument() );
	fillmat->setTextureAlphaCombine( 0, Material::TA_DIFFUSE, Material::TOP_SELECTARG1, Material::TextureArgument() );
	fillmat->setLighting( false );
	fillmat->setVertexColor( true );
	fillmat->setStencil( true );
	fillmat->setStencilRef( 1 );
	fillmat->setStencilFunc( Material::CMP_LESSEQUAL );
	fillmat->setStencilPass( Material::STENCILOP_KEEP );
	fillmat->setPass( ShadowShader::DEFAULT_SHADOW_FILL_PASS );
	fillmat->setCull( Material::CULL_NONE );
	fillmat->setFogDisabled( true );
	fillmat->setVertexFormat( fillvf );

	// geometry
	P(TriangleList) fillgeom = new TriangleList( 6, fillvf );
	fillgeom->setShader( fillmat );
	{VertexLock<TriangleList> lock( fillgeom, TriangleList::LOCK_WRITE );
	Vector4 fillpos[4] = { Vector4(0,0,0,1), Vector4(width,0,0,1), Vector4(width,height,0,1), Vector4(0,height,0,1) };
	Color fillcolr[4] = { color, color, color, color };
	int fillind[6] = { 0,1,2, 0,2,3 };
	for ( int k = 0 ; k < 6 ; ++k )
	{
		fillgeom->setVertexPositionsRHW( k, &fillpos[ fillind[k] ], 1 );
		fillgeom->setVertexDiffuseColors( k, &fillcolr[ fillind[k] ], 1 );
	}}
	
	return fillgeom;
}

void ShadowUtil::setShadowVolumeShaders( Node* scene, Shader* shadowShader )
{
	for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );

		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				ShadowVolume* shadow = dynamic_cast<ShadowVolume*>( mesh->getPrimitive(i) );
				if ( shadow )
					shadow->setShader( shadowShader );
			}
		}
	}
}

void ShadowUtil::updateDynamicShadow( sg::Node* root, sg::Mesh* shadowMesh, 
	const lang::String& shadowName, const math::Vector3& lightWorld, float shadowLength, float shadowViewOffset )
{
	assert( root );

	root->unlink();
	shadowMesh->unlink();
	shadowMesh->removePrimitives();

	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* root = dynamic_cast<Mesh*>( node );
		if ( root )
		{
			for ( int i = 0 ; i < root->primitives() ; ++i )
			{
				P(ShadowVolume) shadow = dynamic_cast<ShadowVolume*>( root->getPrimitive(i) );
				if ( shadow )
					root->removePrimitive( i-- );
			}

			if ( shadowName == root->name() )
			{
				shadowMesh->setBones( root );

				for ( int i = 0 ; i < root->primitives() ; ++i )
				{
					P(Model) model = dynamic_cast<Model*>( root->getPrimitive(i) );
					if ( model )
					{
						P(ShadowVolume) shadow = new ShadowVolume( model, lightWorld, shadowLength );
						shadow->setViewOffset( shadowViewOffset );
						shadowMesh->addPrimitive( shadow );
					}
				}

				shadowMesh->linkTo( root );
				break;
			}
		}
	}
}

void ShadowUtil::setShadowRenderPass( sg::Node* root, int passShadow )
{
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* root = dynamic_cast<Mesh*>( node );
		if ( root )
		{
			for ( int i = 0 ; i < root->primitives() ; ++i )
			{
				ShadowVolume* shadow = dynamic_cast<ShadowVolume*>( root->getPrimitive(i) );
				if ( shadow )
				{
					Shader* shader = shadow->shader();
					if ( shader )
						shader->setPass( passShadow );
				}
			}
		}
	}
}


} // sgu
