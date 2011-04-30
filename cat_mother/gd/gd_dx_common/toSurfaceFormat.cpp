#include "StdAfx.h"
#include "toSurfaceFormat.h"
#include "config.h"

//-----------------------------------------------------------------------------

using namespace pix;

//-----------------------------------------------------------------------------

pix::SurfaceFormat toSurfaceFormat( D3DFORMAT d3dfmt )
{
	switch ( d3dfmt )
	{
    case D3DFMT_R8G8B8:               return SurfaceFormat::SURFACE_R8G8B8;
    case D3DFMT_A8R8G8B8:             return SurfaceFormat::SURFACE_A8R8G8B8;
    case D3DFMT_X8R8G8B8:             return SurfaceFormat::SURFACE_X8R8G8B8;
    case D3DFMT_R5G6B5:               return SurfaceFormat::SURFACE_R5G6B5;
    case D3DFMT_X1R5G5B5:             return SurfaceFormat::SURFACE_R5G5B5;
    case D3DFMT_A1R5G5B5:             return SurfaceFormat::SURFACE_A1R5G5B5;
    case D3DFMT_A4R4G4B4:             return SurfaceFormat::SURFACE_A4R4G4B4;
    case D3DFMT_R3G3B2:               return SurfaceFormat::SURFACE_R3G2B3;
    case D3DFMT_A8:                   return SurfaceFormat::SURFACE_A8;
    case D3DFMT_A8R3G3B2:             return SurfaceFormat::SURFACE_A8R3G3B2;
    case D3DFMT_X4R4G4B4:             return SurfaceFormat::SURFACE_X4R4G4B4;
    case D3DFMT_P8:                   return SurfaceFormat::SURFACE_P8;

	case D3DFMT_DXT1:                 return SurfaceFormat::SURFACE_DXT1;
    //case D3DFMT_DXT2:                 return SurfaceFormat::SURFACE_DXT2;
    case D3DFMT_DXT3:                 return SurfaceFormat::SURFACE_DXT3;
    //case D3DFMT_DXT4:                 return SurfaceFormat::SURFACE_DXT4;
    case D3DFMT_DXT5:                 return SurfaceFormat::SURFACE_DXT5;

    case D3DFMT_D32:                  return SurfaceFormat::SURFACE_D32;
    case D3DFMT_D16:                  return SurfaceFormat::SURFACE_D16;
    case D3DFMT_D24S8:                return SurfaceFormat::SURFACE_D24S8;

    case D3DFMT_VERTEXDATA:           
    case D3DFMT_INDEX16:              
    case D3DFMT_INDEX32:              
    
	case D3DFMT_A8P8:
    case D3DFMT_L8:                   
    case D3DFMT_A8L8:                 
    case D3DFMT_A4L4:                 
    case D3DFMT_V8U8:                
    case D3DFMT_L6V5U5:               
    case D3DFMT_X8L8V8U8:             
    case D3DFMT_Q8W8V8U8:             
	case D3DFMT_V16U16:               
    //case D3DFMT_W11V11U10:            
    case D3DFMT_UYVY:                 
    case D3DFMT_YUY2:                 
    
    case D3DFMT_D16_LOCKABLE:         
    case D3DFMT_D15S1:                
    case D3DFMT_D24X8:                
    case D3DFMT_D24X4S4:              

    case D3DFMT_UNKNOWN:              
	default:							return SurfaceFormat::SURFACE_UNKNOWN;
	}
}
