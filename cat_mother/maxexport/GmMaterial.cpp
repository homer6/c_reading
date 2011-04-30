#include "StdAfx.h"
#include "GmMaterial.h"
#include "ChunkUtil.h"
#include <io/IOException.h>
#include <io/FileInputStream.h>
#include <io/ChunkOutputStream.h>
#include <dev/Profile.h>
#include <pix/Image.h>
#include <lang/Debug.h>
#include <lang/String.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace pix;
using namespace lang;

//-----------------------------------------------------------------------------

enum CullMode
{
	/** Do not cull back faces */
	CULL_NONE,
	/** Cull faces with clockwise vertices */
	CULL_CW,
	/** Cull faces with counterclockwise vertices (default) */
	CULL_CCW
};

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

//-----------------------------------------------------------------------------

static String toString( GmMaterial::EnvType env )
{
	switch ( env )
	{
	case GmMaterial::ENV_NONE:			return "NONE";
	case GmMaterial::ENV_SPHERICAL:		return "SPHERICAL";
	case GmMaterial::ENV_CYLINDRICAL:	return "CYLINDRICAL";
	case GmMaterial::ENV_SHRINKWRAP:	return "SHRINKWRAP";
	case GmMaterial::ENV_SCREEN:		return "SCREEN";
	default:							return "INVALID";
	}
}

static void println( const String& name, const GmMaterial::TextureLayer& layer )
{
	if ( layer.filename.length() > 0 )
	{
		Debug::println( "    {0} layer:", name );
		Debug::println( "      filename = {0}", layer.filename );
		Debug::println( "      filter = {0}", layer.filter );
		Debug::println( "      env = {0}", toString(layer.env) );
		Debug::println( "      uoffs = {0}", layer.uoffs );
		Debug::println( "      voffs = {0}", layer.voffs );
		Debug::println( "      uscale = {0}", layer.uscale );
		Debug::println( "      vscale = {0}", layer.vscale );
	}
	else
	{
		Debug::println( "    (layer {0} is empty)", name );
	}
}

static String toString( GmMaterial::BlendType blend )
{
	switch ( blend )
	{
	case GmMaterial::BLEND_COPY:		return "COPY";
	case GmMaterial::BLEND_ADD:			return "ADD";
	case GmMaterial::BLEND_MULTIPLY:	return "MULTIPLY";
	default:							return "INVALID";
	}
}

static String stripPath( const String& str )
{
	int i1 = str.lastIndexOf("/");
	int i2 = str.lastIndexOf("\\");

	int i = i1;
	if ( i < i2 )
		i = i2;

	return str.substring( i+1 );
}

static Image::ImageType getImageType( const String& filename, bool throwExceptions=false )
{
	try
	{
		FileInputStream in( filename );
		Image img( &in );
		return img.type();
	}
	catch ( ... )
	{
		Debug::printlnWarning( "Failed to open {0} for image type check", filename );
		if ( !throwExceptions )
			return Image::TYPE_BITMAP;
		throw;
	}
}

static void writeTextureType( ChunkOutputStream* out, const lang::String& filename )
{
	if ( getImageType(filename) == Image::TYPE_CUBEMAP )
		ChunkUtil::writeStringChunk( out, "type", "cubemap" );
	else
		ChunkUtil::writeStringChunk( out, "type", "bitmap" );
}

static void writeTexOnly( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex )
{
	out->beginChunk( "tex_only" );

	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );

	out->endChunk();
}

static void writeEnvOnly( ChunkOutputStream* out, const GmMaterial::TextureLayer& env )
{
	out->beginChunk( "env_only" );
	
	writeTextureType( out, env.filename );
	ChunkUtil::writeStringChunk( out, "envmap", stripPath(env.filename) );

	out->endChunk();
}

static void writeTexAndEnv( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex,
	const GmMaterial::TextureLayer& env )
{
	out->beginChunk( "tex_and_env" );
	
	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );
	writeTextureType( out, env.filename );
	ChunkUtil::writeStringChunk( out, "envmap", stripPath(env.filename) );

	out->endChunk();
}

static void writeTexAndBump( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex,
	const GmMaterial::TextureLayer& bump )
{
	out->beginChunk( "tex_and_bump" );
	
	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );
	writeTextureType( out, bump.filename );
	ChunkUtil::writeStringChunk( out, "bumpmap", stripPath(bump.filename) );

	out->endChunk();
}

static void writeTexAndMaskedEnv( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex,
	const GmMaterial::TextureLayer& env )
{
	out->beginChunk( "tex_and_masked_env" );
	
	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );
	writeTextureType( out, env.filename );
	ChunkUtil::writeStringChunk( out, "envmap", stripPath(env.filename) );

	out->endChunk();
}

static void writeLightOnly( ChunkOutputStream* out, const GmMaterial::TextureLayer& light )
{
	out->beginChunk( "light_only" );
	
	writeTextureType( out, light.filename );
	ChunkUtil::writeStringChunk( out, "lightmap", stripPath(light.filename) );

	out->endChunk();
}

static void writeTexAndLight( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex,
	const GmMaterial::TextureLayer& light )
{
	out->beginChunk( "tex_and_light" );
	
	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );
	writeTextureType( out, light.filename );
	ChunkUtil::writeStringChunk( out, "lightmap", stripPath(light.filename) );

	out->endChunk();
}

static void writeEnvAndLight( ChunkOutputStream* out, const GmMaterial::TextureLayer& env,
	const GmMaterial::TextureLayer& light )
{
	out->beginChunk( "env_and_light" );
	
	writeTextureType( out, env.filename );
	ChunkUtil::writeStringChunk( out, "envmap", stripPath(env.filename) );
	writeTextureType( out, light.filename );
	ChunkUtil::writeStringChunk( out, "lightmap", stripPath(light.filename) );

	out->endChunk();
}

static void writeTexEnvAndLight( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex,
	const GmMaterial::TextureLayer& env, const GmMaterial::TextureLayer& light )
{
	out->beginChunk( "tex_env_and_light" );
	
	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );
	writeTextureType( out, env.filename );
	ChunkUtil::writeStringChunk( out, "envmap", stripPath(env.filename) );
	writeTextureType( out, light.filename );
	ChunkUtil::writeStringChunk( out, "lightmap", stripPath(light.filename) );

	out->endChunk();
}

static void writeTexBumpAndLight( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex,
	const GmMaterial::TextureLayer& bump, const GmMaterial::TextureLayer& light )
{
	out->beginChunk( "tex_bump_and_light" );
	
	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );
	writeTextureType( out, bump.filename );
	ChunkUtil::writeStringChunk( out, "bumpmap", stripPath(bump.filename) );
	writeTextureType( out, light.filename );
	ChunkUtil::writeStringChunk( out, "lightmap", stripPath(light.filename) );

	out->endChunk();
}

static void writeTexMaskedEnvAndLight( ChunkOutputStream* out, const GmMaterial::TextureLayer& tex,
	const GmMaterial::TextureLayer& env, const GmMaterial::TextureLayer& light )
{
	out->beginChunk( "tex_masked_env_and_light " );
	
	writeTextureType( out, tex.filename );
	ChunkUtil::writeStringChunk( out, "texmap", stripPath(tex.filename) );
	writeTextureType( out, env.filename );
	ChunkUtil::writeStringChunk( out, "envmap", stripPath(env.filename) );
	writeTextureType( out, light.filename );
	ChunkUtil::writeStringChunk( out, "lightmap", stripPath(light.filename) );

	out->endChunk();
}

static void writeEffectTextureParam( ChunkOutputStream* out, const String& name, const GmMaterial::TextureLayer& layer )
{
	Image::ImageType type = getImageType( layer.filename, true );
	String fname = stripPath( layer.filename );

	if ( type == Image::TYPE_CUBEMAP )
	{
		Debug::println( "    CubeTexture {0} = {1}", name, fname );
		out->beginChunk( "cubetexture" );
		out->writeString( name );
		out->writeString( fname );
		out->endChunk();
	}
	else
	{
		Debug::println( "    Texture {0} = {1}", name, fname );
		out->beginChunk( "texture" );
		out->writeString( name );
		out->writeString( fname );
		out->endChunk();
	}
}

//-----------------------------------------------------------------------------

GmMaterial::TextureLayer::TextureLayer()
{
	filename	= "";
	coordset	= 0;
	filter		= true;
	env			= ENV_NONE;
	uoffs		= 0.5f;
	voffs		= 0.5f;
	uscale		= 0.5f;
	vscale		= 0.5f;
}

bool GmMaterial::TextureLayer::operator==( const GmMaterial::TextureLayer& other ) const
{
	return 
		filename == other.filename &&
		coordset == other.coordset &&
		filter == other.filter &&
		env == other.env &&
		uoffs == other.uoffs &&
		voffs == other.voffs &&
		uscale == other.uscale &&
		vscale == other.vscale;
}

bool GmMaterial::TextureLayer::operator!=( const GmMaterial::TextureLayer& other ) const
{
	return !(*this == other);
}

//-----------------------------------------------------------------------------

GmMaterial::GmMaterial()
{
	defaults();
}

void GmMaterial::defaults()
{
	name				= "";
	opacity				= 1.f;
	selfIllum			= 0.f;
	twosided			= false;
	blend				= BLEND_COPY;
	diffuseColor		= Colorf(1,1,1,1);
	specular			= false;
	specularColor		= Colorf(1,1,1,1);
	specularExponent	= 0.f;
	diffuseLayer		= TextureLayer();
	opacityLayer		= TextureLayer();
	reflectionLayer		= TextureLayer();
	bumpLayer			= TextureLayer();
	specularColorLayer	= TextureLayer();
	specularLevelLayer	= TextureLayer();
}

void GmMaterial::write( ChunkOutputStream* out ) const
{
	Debug::println( "  Writing material {0}", name );
	Debug::println( "    blend = {0}", toString(blend) );
	Debug::println( "    selfIllum = {0}", selfIllum );
	Debug::println( "    twosided = {0}", twosided );
	Debug::println( "    opacity = {0,#.##}", opacity );
	Debug::println( "    diffuseColor = {0} {1} {2}", diffuseColor.red(), diffuseColor.green(), diffuseColor.blue() );
	Debug::println( "    specular = {0}", specular );
	Debug::println( "    specularColor = {0} {1} {2}", specularColor.red(), specularColor.green(), specularColor.blue() );
	Debug::println( "    specularExponent = {0}", specularExponent );
	println( "Diffuse", diffuseLayer );
	println( "Opacity", opacityLayer );
	println( "Reflection", reflectionLayer );
	println( "Glossiness", glossinessLayer );
	println( "Bump", bumpLayer );
	println( "LightMap", lightMapLayer );
	println( "SpecularColor", specularColorLayer );
	println( "SpecularLevel", specularLevelLayer );

	int fxIndex = name.indexOf("FX=");
	if ( fxIndex >= 0 && fxIndex+3 < name.length() )
	{
		String fxName = name.substring( fxIndex + 3 );

		int fxEndIndex = fxName.indexOf(' ');
		if ( fxEndIndex >= 0 )
			fxName = fxName.substring( 0, fxEndIndex );
		fxName = fxName + ".fx";

		writeEffect( out, fxName );
	}
	else
	{
		writeMaterial( out );
	}
}

void GmMaterial::writeEffect( ChunkOutputStream* out, const String& fxName ) const
{
	out->beginChunk( "effect" );

	Debug::println( "  Writing effect {0} using {1}", name, fxName );

	out->writeString( fxName );
	out->writeString( name );

	int flags = this->layerFlags();
	if ( flags & L_DIFF )
		writeEffectTextureParam( out, "tDiffuse", diffuseLayer );
	if ( flags & L_REFL )
		writeEffectTextureParam( out, "tReflection", reflectionLayer );
	if ( flags & L_BUMP )
		writeEffectTextureParam( out, "tBump", bumpLayer );
	if ( flags & L_LGHT )
		writeEffectTextureParam( out, "tLightMap", lightMapLayer );
	if ( flags & L_SPEC_COLOR )
		writeEffectTextureParam( out, "tSpecularColor", specularColorLayer );
	if ( flags & L_SPEC_LEVEL )
		writeEffectTextureParam( out, "tSpecularLevel", specularLevelLayer );

	out->endChunk();
}

void GmMaterial::writeMaterial( ChunkOutputStream* out ) const
{
	out->beginChunk( "material" );
	out->writeString( name );

	if ( BLEND_COPY != blend )
		ChunkUtil::writeIntChunk( out, "zwrite", false );

	switch ( blend )
	{
	case BLEND_ADD:			ChunkUtil::writeIntChunk2( out, "blend", BLEND_ONE, BLEND_ONE ); break;
	case BLEND_MULTIPLY:	ChunkUtil::writeIntChunk2( out, "blend", BLEND_SRCALPHA, BLEND_INVSRCALPHA ); break;
	}
	
	if ( twosided )
		ChunkUtil::writeIntChunk( out, "cull", (twosided ? CULL_NONE : CULL_CCW) );

	if ( selfIllum >= 0.99f || (this->layerFlags() & L_LGHT) )
		ChunkUtil::writeIntChunk( out, "lighting", false );

	if ( selfIllum > 0.f && selfIllum < 1.f )
		ChunkUtil::writeFloatChunk3( out, "emissive", selfIllum, selfIllum, selfIllum );

	if ( 0 == diffuseLayer.filename.length() && 0 == reflectionLayer.filename.length() )
		ChunkUtil::writeFloatChunk3( out, "diffuse", diffuseColor.red(), diffuseColor.green(), diffuseColor.blue() );

	if ( specular )
		ChunkUtil::writeFloatChunk4( out, "specular", specularColor.red(), specularColor.green(), specularColor.blue(), specularExponent );
	
	if ( opacity < 1.f )
		ChunkUtil::writeFloatChunk( out, "opacity", opacity );

	if ( name.indexOf("NOFOG") >= 0 )
		ChunkUtil::writeIntChunk( out, "nofog", 1 );

	// get layer textures
	int layerFlags = this->layerFlags();

	// write texture layers
	switch ( layerFlags )
	{
	case 0:
		break;

	case L_LGHT:
		writeLightOnly( out, lightMapLayer );
		break;

	case L_DIFF:
		writeTexOnly( out, diffuseLayer );
		break;

	case (L_DIFF|L_LGHT):
		writeTexAndLight( out, diffuseLayer, lightMapLayer );
		break;

	/*case L_REFL:
		writeEnvOnly( out, reflectionLayer );
		break;

	case (L_REFL|L_LGHT):
		writeEnvAndLight( out, reflectionLayer, lightMapLayer );
		break;

	case (L_DIFF|L_REFL):
		writeTexAndEnv( out, diffuseLayer, reflectionLayer );
		break;

	case (L_DIFF|L_REFL|L_LGHT):
		writeTexEnvAndLight( out, diffuseLayer, reflectionLayer, lightMapLayer );
		break;

	case (L_DIFF|L_BUMP):
		writeTexAndBump( out, diffuseLayer, bumpLayer );
		break;

	case (L_DIFF|L_BUMP|L_LGHT):
		writeTexBumpAndLight( out, diffuseLayer, bumpLayer, lightMapLayer );
		break;

	case (L_DIFF|L_REFL|L_GLOS):	
		writeTexAndMaskedEnv( out, diffuseLayer, reflectionLayer );
		break;

	case (L_DIFF|L_REFL|L_GLOS|L_LGHT):	
		writeTexMaskedEnvAndLight( out, diffuseLayer, reflectionLayer, lightMapLayer );
		break;*/
	
	default:
		throw Exception( Format("Material {0} has invalid combination of map channels (or shader tag missing in the name)\nOnly 'diffuse' and 'diffuse*self-illumination' texture combinations are supported without using shaders.", name) );
		//require( false );
		break;
	}

	out->endChunk();
}

int GmMaterial::layerFlags() const
{
	int flags = 0;
	if ( diffuseLayer.filename.length() > 0 )
		flags |= L_DIFF;
	if ( reflectionLayer.filename.length() > 0 )
		flags |= L_REFL;
	if ( glossinessLayer.filename.length() > 0 )
		flags |= L_GLOS;
	if ( bumpLayer.filename.length() > 0 )
		flags |= L_BUMP;
	if ( lightMapLayer.filename.length() > 0 )
		flags |= L_LGHT;
	if ( specularColorLayer.filename.length() > 0 )
		flags |= L_SPEC_COLOR;
	if ( specularLevelLayer.filename.length() > 0 )
		flags |= L_SPEC_LEVEL;
	return flags;
}

bool GmMaterial::operator==( const GmMaterial& other ) const
{
	return
		name == other.name &&
		opacity == other.opacity &&			
		selfIllum == other.selfIllum &&		
		twosided == other.twosided &&		
		blend == other.blend &&			
		diffuseColor == other.diffuseColor &&	
		specular == other.specular &&		
		specularColor == other.specularColor &&	
		specularExponent == other.specularExponent &&
		diffuseLayer == other.diffuseLayer &&	
		opacityLayer == other.opacityLayer &&	
		reflectionLayer == other.reflectionLayer;
}

bool GmMaterial::operator!=( const GmMaterial& other ) const
{
	return !(*this == other);
}
