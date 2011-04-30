#include "StdAfx.h"
#include "toSurfaceFormat.h"
#include "toDx9.h"
#include "zero.h"
#include <math/Vector4.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace gd;
using namespace pix;

//-----------------------------------------------------------------------------

void toDx9( const pix::Colorf& s, D3DCOLORVALUE& d )
{
	d.r = s.red();
	d.g = s.green();
	d.b = s.blue();
	d.a = s.alpha();
}

void toDx9( const math::Vector3& s, D3DVECTOR& d )
{
	d.x = s.x;
	d.y = s.y;
	d.z = s.z;
}

void toDx9( const LightState& s, D3DLIGHT9& d )
{
	zero( d );

	switch ( s.type )
	{
	case LightState::LIGHT_SPOT:			d.Type = D3DLIGHT_SPOT; break;
	case LightState::LIGHT_POINT:			d.Type = D3DLIGHT_POINT; break;
	case LightState::LIGHT_DIRECT:			d.Type = D3DLIGHT_DIRECTIONAL; break;
	}

	toDx9( s.diffuse, d.Diffuse );
	toDx9( s.specular, d.Specular );
	toDx9( s.ambient, d.Ambient );

	toDx9( s.position, d.Position );
	toDx9( s.direction, d.Direction );

	d.Range = s.range;
	d.Falloff = s.falloff;
	d.Attenuation0 = s.attenuation0;
	d.Attenuation1 = s.attenuation1;
	d.Attenuation2 = s.attenuation2;
	d.Theta = s.theta;
	d.Phi = s.phi;
}

void toDx9( const math::Matrix4x4& s, D3DMATRIX& d )
{
	for ( int j = 0 ; j < 4 ; ++j )
		for ( int i = 0 ; i < 4 ; ++i )
			d.m[i][j] = s(j,i);
}

void fromDx9( const D3DMATRIX& s, math::Matrix4x4& d )
{
	for ( int j = 0 ; j < 4 ; ++j )
		for ( int i = 0 ; i < 4 ; ++i )
			d(i,j) = s.m[j][i];
}

D3DFORMAT toDx9( const pix::SurfaceFormat& fmt )
{
	switch ( fmt.type() )
	{
    case SurfaceFormat::SURFACE_R8G8B8:				return D3DFMT_R8G8B8;        
    case SurfaceFormat::SURFACE_A8R8G8B8:			return D3DFMT_A8R8G8B8;      
    case SurfaceFormat::SURFACE_X8R8G8B8:			return D3DFMT_X8R8G8B8;      
    case SurfaceFormat::SURFACE_R5G6B5:				return D3DFMT_R5G6B5;        
    case SurfaceFormat::SURFACE_R5G5B5:				return D3DFMT_X1R5G5B5;
    case SurfaceFormat::SURFACE_A1R5G5B5:			return D3DFMT_A1R5G5B5;      
    case SurfaceFormat::SURFACE_A4R4G4B4:			return D3DFMT_A4R4G4B4;      
    case SurfaceFormat::SURFACE_R3G2B3:				return D3DFMT_R3G3B2;        
    case SurfaceFormat::SURFACE_A8:					return D3DFMT_A8;            
    case SurfaceFormat::SURFACE_A8R3G3B2:			return D3DFMT_A8R3G3B2;      
    case SurfaceFormat::SURFACE_X4R4G4B4:			return D3DFMT_X4R4G4B4;      
    case SurfaceFormat::SURFACE_P8:					return D3DFMT_P8;
	case SurfaceFormat::SURFACE_DXT1:				return D3DFMT_DXT1;
	case SurfaceFormat::SURFACE_DXT3:				return D3DFMT_DXT3;
	case SurfaceFormat::SURFACE_DXT5:				return D3DFMT_DXT5;
	case SurfaceFormat::SURFACE_D32:				return D3DFMT_D32;
	case SurfaceFormat::SURFACE_D16:				return D3DFMT_D16;
	case SurfaceFormat::SURFACE_D24S8:				return D3DFMT_D24S8;
	default:										return D3DFMT_UNKNOWN;
	}
}

pix::SurfaceFormat fromDx9( D3DFORMAT fmt )
{
	return toSurfaceFormat( fmt );
}

D3DDEVTYPE toDx9( GraphicsDevice::RasterizerType rz )
{
	switch ( rz )
	{
	case GraphicsDevice::RASTERIZER_SW:		return D3DDEVTYPE_REF;
	case GraphicsDevice::RASTERIZER_HW:		
	default:								return D3DDEVTYPE_HAL;
	}
}

D3DFOGMODE toDx9( GraphicsDevice::FogMode mode )
{
	switch ( mode )
	{
	case GraphicsDevice::FOG_NONE:		return D3DFOG_NONE;
	case GraphicsDevice::FOG_LINEAR:	return D3DFOG_LINEAR;
	case GraphicsDevice::FOG_EXP:		return D3DFOG_EXP;
	case GraphicsDevice::FOG_EXP2:		return D3DFOG_EXP2;
	}
	return D3DFOG_NONE;
}

D3DSTENCILOP toDx9( Material::StencilOperation sop )
{
	switch ( sop )
	{
	case Material::STENCILOP_KEEP:		return D3DSTENCILOP_KEEP;
	case Material::STENCILOP_ZERO:		return D3DSTENCILOP_ZERO;
	case Material::STENCILOP_REPLACE:	return D3DSTENCILOP_REPLACE;
	case Material::STENCILOP_INCR:		return D3DSTENCILOP_INCR;
	case Material::STENCILOP_DECR:		return D3DSTENCILOP_DECR;
	case Material::STENCILOP_INVERT:	return D3DSTENCILOP_INVERT;
	default:							return (D3DSTENCILOP)(-1);
	}
}

D3DTEXTUREADDRESS toDx9( Material::TextureAddressMode v )
{
	switch ( v )
	{
	case Material::TADDRESS_WRAP:		return D3DTADDRESS_WRAP;
	case Material::TADDRESS_MIRROR:		return D3DTADDRESS_MIRROR;
	case Material::TADDRESS_CLAMP:		return D3DTADDRESS_CLAMP;
	default:							return D3DTADDRESS_WRAP;
	}
}

D3DTEXTUREFILTERTYPE toDx9( Material::TextureFilterType v )
{
	switch ( v )
	{
	case Material::TEXF_POINT:				return D3DTEXF_POINT;
	case Material::TEXF_LINEAR:				return D3DTEXF_LINEAR;
	case Material::TEXF_ANISOTROPIC:		return D3DTEXF_ANISOTROPIC;
	default:								return D3DTEXF_POINT;
	}
}

D3DTEXTUREFILTERTYPE toDx9( GraphicsDevice::TextureFilterType v )
{
	switch ( v )
	{
	case GraphicsDevice::TEXF_NONE:			return D3DTEXF_NONE;
	case GraphicsDevice::TEXF_POINT:		return D3DTEXF_POINT;
	case GraphicsDevice::TEXF_LINEAR:		return D3DTEXF_LINEAR;
	case GraphicsDevice::TEXF_ANISOTROPIC:	return D3DTEXF_ANISOTROPIC;
	default:								return D3DTEXF_POINT;
	}
}

D3DTEXTUREOP toDx9( Material::TextureOperation v )
{
	switch ( v )
	{
	case Material::TOP_DISABLE:						return D3DTOP_DISABLE;                   
	case Material::TOP_SELECTARG1:					return D3DTOP_SELECTARG1;                
	case Material::TOP_SELECTARG2:					return D3DTOP_SELECTARG2;                
	case Material::TOP_MODULATE:					return D3DTOP_MODULATE;                  
	case Material::TOP_MODULATE2X:					return D3DTOP_MODULATE2X;                
	case Material::TOP_MODULATE4X:					return D3DTOP_MODULATE4X;                
	case Material::TOP_ADD:							return D3DTOP_ADD;                       
	case Material::TOP_ADDSIGNED:					return D3DTOP_ADDSIGNED;                 
	case Material::TOP_ADDSIGNED2X:					return D3DTOP_ADDSIGNED2X;               
	case Material::TOP_SUBTRACT:					return D3DTOP_SUBTRACT;                  
	case Material::TOP_ADDSMOOTH:					return D3DTOP_ADDSMOOTH;                 
	case Material::TOP_BLENDDIFFUSEALPHA:			return D3DTOP_BLENDDIFFUSEALPHA;         
	case Material::TOP_BLENDTEXTUREALPHA:			return D3DTOP_BLENDTEXTUREALPHA;         
	case Material::TOP_BLENDFACTORALPHA:			return D3DTOP_BLENDFACTORALPHA;          
	case Material::TOP_BLENDTEXTUREALPHAPM:			return D3DTOP_BLENDTEXTUREALPHAPM;       
	case Material::TOP_BLENDCURRENTALPHA:			return D3DTOP_BLENDCURRENTALPHA;         
	case Material::TOP_PREMODULATE:					return D3DTOP_PREMODULATE;               
	case Material::TOP_MODULATEALPHA_ADDCOLOR:		return D3DTOP_MODULATEALPHA_ADDCOLOR;    
	case Material::TOP_MODULATECOLOR_ADDALPHA:		return D3DTOP_MODULATECOLOR_ADDALPHA;    
	case Material::TOP_MODULATEINVALPHA_ADDCOLOR:	return D3DTOP_MODULATEINVALPHA_ADDCOLOR; 
	case Material::TOP_MODULATEINVCOLOR_ADDALPHA:	return D3DTOP_MODULATEINVCOLOR_ADDALPHA; 
	case Material::TOP_BUMPENVMAP:					return D3DTOP_BUMPENVMAP;                
	case Material::TOP_BUMPENVMAPLUMINANCE:			return D3DTOP_BUMPENVMAPLUMINANCE;       
	case Material::TOP_DOTPRODUCT3:					return D3DTOP_DOTPRODUCT3;               
	default:										return D3DTOP_DISABLE;
	}
}

DWORD toDx9( Material::TextureArgument v )
{
	switch ( v )
	{
	case Material::TA_DIFFUSE:		return D3DTA_DIFFUSE;
	case Material::TA_CURRENT:		return D3DTA_CURRENT;
	case Material::TA_TEXTURE:		return D3DTA_TEXTURE;
	case Material::TA_TFACTOR:		return D3DTA_TFACTOR;
	case Material::TA_SPECULAR:		return D3DTA_SPECULAR;
	case Material::TA_TEXTUREALPHA:	return D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE;
	default:						return D3DTA_DIFFUSE;
	}
}

DWORD toDx9( const Material::TextureCoordinateTransformMode v )
{
	DWORD f = 0;
	
	switch ( unsigned(v) & 255 )
	{
	case Material::TTFF_DISABLE:			f |= D3DTTFF_DISABLE; break;
	case Material::TTFF_COUNT1:				f |= D3DTTFF_COUNT1; break;
	case Material::TTFF_COUNT2:				f |= D3DTTFF_COUNT2; break;
	case Material::TTFF_COUNT3:				f |= D3DTTFF_COUNT3; break;
	case Material::TTFF_COUNT4:				f |= D3DTTFF_COUNT4; break;
	case Material::TTFF_COUNT2_PROJECTED:	f |= (D3DTTFF_COUNT2|D3DTTFF_PROJECTED); break;
	case Material::TTFF_COUNT3_PROJECTED:	f |= (D3DTTFF_COUNT3|D3DTTFF_PROJECTED); break;
	case Material::TTFF_COUNT4_PROJECTED:	f |= (D3DTTFF_COUNT4|D3DTTFF_PROJECTED); break;
	}

	return f;
}

DWORD toDx9( const Material::TextureCoordinateSourceType v )
{
	switch ( v )
	{
	case Material::TCS_VERTEXDATA:					return D3DTSS_TCI_PASSTHRU;
	case Material::TCS_CAMERASPACENORMAL:			return D3DTSS_TCI_CAMERASPACENORMAL;
	case Material::TCS_CAMERASPACEPOSITION:			return D3DTSS_TCI_CAMERASPACEPOSITION;
	case Material::TCS_CAMERASPACEREFLECTIONVECTOR:	return D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
	default:										return D3DTSS_TCI_PASSTHRU;
	}
}

void toDx9( const Dx9Material::ReflectanceFactors& refl, D3DMATERIAL9& d3dmat )
{
	toDx9( refl.diffuseColor, d3dmat.Diffuse );
	toDx9( refl.ambientColor, d3dmat.Ambient );
	toDx9( refl.emissiveColor, d3dmat.Emissive );
	toDx9( refl.specularColor, d3dmat.Specular );
	d3dmat.Power = refl.specularExponent;
}

D3DBLEND toDx9( Material::BlendMode v )
{
	switch ( v )
	{
		case Material::BLEND_ZERO:			return D3DBLEND_ZERO;
		case Material::BLEND_ONE:			return D3DBLEND_ONE; 
		case Material::BLEND_SRCCOLOR:		return D3DBLEND_SRCCOLOR; 
		case Material::BLEND_INVSRCCOLOR:	return D3DBLEND_INVSRCCOLOR; 
		case Material::BLEND_SRCALPHA:		return D3DBLEND_SRCALPHA; 
		case Material::BLEND_INVSRCALPHA:	return D3DBLEND_INVSRCALPHA; 
		case Material::BLEND_DESTALPHA:		return D3DBLEND_DESTALPHA; 
		case Material::BLEND_INVDESTALPHA:	return D3DBLEND_INVDESTALPHA; 
		case Material::BLEND_DESTCOLOR:		return D3DBLEND_DESTCOLOR; 
		case Material::BLEND_INVDESTCOLOR:	return D3DBLEND_INVDESTCOLOR; 
		case Material::BLEND_SRCALPHASAT:	return D3DBLEND_SRCALPHASAT;
		default:							return D3DBLEND_ZERO;
	}
}

D3DCMPFUNC toDx9( Material::CmpFunc v )
{
	switch ( v )
	{
		case Material::CMP_NEVER:			return D3DCMP_NEVER;                
		case Material::CMP_LESS:			return D3DCMP_LESS;                 
		case Material::CMP_EQUAL:			return D3DCMP_EQUAL;                
		case Material::CMP_LESSEQUAL:		return D3DCMP_LESSEQUAL;            
		case Material::CMP_GREATER:			return D3DCMP_GREATER;              
		case Material::CMP_NOTEQUAL:		return D3DCMP_NOTEQUAL;             
		case Material::CMP_GREATEREQUAL:	return D3DCMP_GREATEREQUAL;         
		case Material::CMP_ALWAYS:			return D3DCMP_ALWAYS;               
		default:							return D3DCMP_NEVER;
	}
}

D3DCULL toDx9( Material::CullMode v )
{
	switch ( v )
	{
    case Material::CULL_NONE:	return D3DCULL_NONE;
    case Material::CULL_CW:		return D3DCULL_CW;
    case Material::CULL_CCW:	return D3DCULL_CCW;
	default:					return D3DCULL_NONE;
	}
}

D3DMATERIALCOLORSOURCE toDx9( Material::MaterialColorSource v )
{
	switch ( v )
	{
	case Material::MCS_MATERIAL:	return D3DMCS_MATERIAL;
	case Material::MCS_COLOR1:		return D3DMCS_COLOR1;
	case Material::MCS_COLOR2:		return D3DMCS_COLOR2;
	default:						return D3DMCS_MATERIAL;
	}
}
