#include "StdAfx.h"
#include "toString.h"
#include "config.h"

//-----------------------------------------------------------------------------

const char* toString( D3DRENDERSTATETYPE state )
{
	switch ( state )
	{
    case D3DRS_ZENABLE: return "ZENABLE";                   
    case D3DRS_FILLMODE: return "FILLMODE";                  
    case D3DRS_SHADEMODE: return "SHADEMODE";                 
    //case D3DRS_LINEPATTERN: return "LINEPATTERN";               
    case D3DRS_ZWRITEENABLE: return "ZWRITEENABLE";              
    case D3DRS_ALPHATESTENABLE: return "ALPHATESTENABLE";           
    case D3DRS_LASTPIXEL: return "LASTPIXEL";                 
    case D3DRS_SRCBLEND: return "SRCBLEND";                  
    case D3DRS_DESTBLEND: return "DESTBLEND";                 
    case D3DRS_CULLMODE: return "CULLMODE";                  
    case D3DRS_ZFUNC: return "ZFUNC";                     
    case D3DRS_ALPHAREF: return "ALPHAREF";                  
    case D3DRS_ALPHAFUNC: return "ALPHAFUNC";                 
    case D3DRS_DITHERENABLE: return "DITHERENABLE";              
    case D3DRS_ALPHABLENDENABLE: return "ALPHABLENDENABLE";          
    case D3DRS_FOGENABLE: return "FOGENABLE";                 
    case D3DRS_SPECULARENABLE: return "SPECULARENABLE";            
    //case D3DRS_ZVISIBLE: return "ZVISIBLE";                  
    case D3DRS_FOGCOLOR: return "FOGCOLOR";                  
    case D3DRS_FOGTABLEMODE: return "FOGTABLEMODE";              
    case D3DRS_FOGSTART: return "FOGSTART";                  
    case D3DRS_FOGEND: return "FOGEND";                    
    case D3DRS_FOGDENSITY: return "FOGDENSITY";                
    //case D3DRS_EDGEANTIALIAS: return "EDGEANTIALIAS";             
    //case D3DRS_ZBIAS: return "ZBIAS";                     
    case D3DRS_RANGEFOGENABLE: return "RANGEFOGENABLE";            
    case D3DRS_STENCILENABLE: return "STENCILENABLE";             
    case D3DRS_STENCILFAIL: return "STENCILFAIL";               
    case D3DRS_STENCILZFAIL: return "STENCILZFAIL";              
    case D3DRS_STENCILPASS: return "STENCILPASS";               
    case D3DRS_STENCILFUNC: return "STENCILFUNC";               
    case D3DRS_STENCILREF: return "STENCILREF";                
    case D3DRS_STENCILMASK: return "STENCILMASK";               
    case D3DRS_STENCILWRITEMASK: return "STENCILWRITEMASK";          
    case D3DRS_TEXTUREFACTOR: return "TEXTUREFACTOR";             
    case D3DRS_WRAP0: return "WRAP0";                     
    case D3DRS_WRAP1: return "WRAP1";                     
    case D3DRS_WRAP2: return "WRAP2";                     
    case D3DRS_WRAP3: return "WRAP3";                     
    case D3DRS_WRAP4: return "WRAP4";                     
    case D3DRS_WRAP5: return "WRAP5";                     
    case D3DRS_WRAP6: return "WRAP6";                     
    case D3DRS_WRAP7: return "WRAP7";                     
    case D3DRS_CLIPPING: return "CLIPPING";                  
    case D3DRS_LIGHTING: return "LIGHTING";                  
    case D3DRS_AMBIENT: return "AMBIENT";                   
    case D3DRS_FOGVERTEXMODE: return "FOGVERTEXMODE";             
    case D3DRS_COLORVERTEX: return "COLORVERTEX";               
    case D3DRS_LOCALVIEWER: return "LOCALVIEWER";               
    case D3DRS_NORMALIZENORMALS: return "NORMALIZENORMALS";          
    case D3DRS_DIFFUSEMATERIALSOURCE: return "DIFFUSEMATERIALSOURCE";     
    case D3DRS_SPECULARMATERIALSOURCE: return "SPECULARMATERIALSOURCE";    
    case D3DRS_AMBIENTMATERIALSOURCE: return "AMBIENTMATERIALSOURCE";     
    case D3DRS_EMISSIVEMATERIALSOURCE: return "EMISSIVEMATERIALSOURCE";    
    case D3DRS_VERTEXBLEND: return "VERTEXBLEND";               
    case D3DRS_CLIPPLANEENABLE: return "CLIPPLANEENABLE";           
    //case D3DRS_SOFTWAREVERTEXPROCESSING: return "SOFTWAREVERTEXPROCESSING";  
    case D3DRS_POINTSIZE: return "POINTSIZE";                 
    case D3DRS_POINTSIZE_MIN: return "POINTSIZE_MIN";             
    case D3DRS_POINTSPRITEENABLE: return "POINTSPRITEENABLE";         
    case D3DRS_POINTSCALEENABLE: return "POINTSCALEENABLE";          
    case D3DRS_POINTSCALE_A: return "POINTSCALE_A";              
    case D3DRS_POINTSCALE_B: return "POINTSCALE_B";              
    case D3DRS_POINTSCALE_C: return "POINTSCALE_C";              
    case D3DRS_MULTISAMPLEANTIALIAS: return "MULTISAMPLEANTIALIAS";      
    case D3DRS_MULTISAMPLEMASK: return "MULTISAMPLEMASK";           
    case D3DRS_PATCHEDGESTYLE: return "PATCHEDGESTYLE";            
    //case D3DRS_PATCHSEGMENTS: return "PATCHSEGMENTS";             
    case D3DRS_DEBUGMONITORTOKEN: return "DEBUGMONITORTOKEN";         
    case D3DRS_POINTSIZE_MAX: return "POINTSIZE_MAX";             
    case D3DRS_INDEXEDVERTEXBLENDENABLE: return "INDEXEDVERTEXBLENDENABLE";  
    case D3DRS_COLORWRITEENABLE: return "COLORWRITEENABLE";          
    case D3DRS_TWEENFACTOR: return "TWEENFACTOR";               
    case D3DRS_BLENDOP: return "BLENDOP";                  
	default: return "UNKNOWN";
	}
}

const char*	toString( D3DFORMAT fmt )
{
	switch ( fmt )
	{
    case D3DFMT_R8G8B8:		return "D3DFMT_R8G8B8";               
    case D3DFMT_A8R8G8B8:	return "D3DFMT_A8R8G8B8";             
    case D3DFMT_X8R8G8B8:	return "D3DFMT_X8R8G8B8";             
    case D3DFMT_R5G6B5:		return "D3DFMT_R5G6B5";               
    case D3DFMT_X1R5G5B5:	return "D3DFMT_X1R5G5B5";             
    case D3DFMT_A1R5G5B5:	return "D3DFMT_A1R5G5B5";             
    case D3DFMT_A4R4G4B4:	return "D3DFMT_A4R4G4B4";             
    case D3DFMT_R3G3B2:		return "D3DFMT_R3G3B2";               
    case D3DFMT_A8:			return "D3DFMT_A8";                   
    case D3DFMT_A8R3G3B2:	return "D3DFMT_A8R3G3B2";             
    case D3DFMT_X4R4G4B4:	return "D3DFMT_X4R4G4B4";             
    case D3DFMT_P8:			return "D3DFMT_P8";                   
    case D3DFMT_VERTEXDATA: return "D3DFMT_VERTEXDATA";           
    case D3DFMT_INDEX16:	return "D3DFMT_INDEX16";              
    case D3DFMT_INDEX32:	return "D3DFMT_INDEX32";              
	case D3DFMT_A8P8:		return "D3DFMT_A8P8";
    case D3DFMT_L8:			return "D3DFMT_L8";                   
    case D3DFMT_A8L8:		return "D3DFMT_A8L8";                 
    case D3DFMT_A4L4:		return "D3DFMT_A4L4";                 
    case D3DFMT_V8U8:		return "D3DFMT_V8U8";                
    case D3DFMT_L6V5U5:		return "D3DFMT_L6V5U5";               
    case D3DFMT_X8L8V8U8:	return "D3DFMT_X8L8V8U8";             
    case D3DFMT_Q8W8V8U8:	return "D3DFMT_Q8W8V8U8";             
	case D3DFMT_V16U16:		return "D3DFMT_V16U16";               
    //case D3DFMT_W11V11U10:	return "D3DFMT_W11V11U10";            
    case D3DFMT_UYVY:		return "D3DFMT_UYVY";                 
    case D3DFMT_YUY2:		return "D3DFMT_YUY2";                 
	case D3DFMT_DXT1:		return "D3DFMT_DXT1";                 
    case D3DFMT_DXT2:		return "D3DFMT_DXT2";                 
    case D3DFMT_DXT3:		return "D3DFMT_DXT3";                 
    case D3DFMT_DXT4:		return "D3DFMT_DXT4";                 
    case D3DFMT_DXT5:		return "D3DFMT_DXT5";                 
    case D3DFMT_D16_LOCKABLE: return "D3DFMT_D16_LOCKABLE";       
    case D3DFMT_D32:		return "D3DFMT_D32";                  
    case D3DFMT_D15S1:		return "D3DFMT_D15S1";                
    case D3DFMT_D24S8:		return "D3DFMT_D24S8";                
    case D3DFMT_D16:		return "D3DFMT_D16";                  
    case D3DFMT_D24X8:		return "D3DFMT_D24X8";                
    case D3DFMT_D24X4S4:	return "D3DFMT_D24X4S4";              
    case D3DFMT_UNKNOWN:              
	default:				return "D3DFMT_UNKNOWN";
	}
}

const char*	toString( HRESULT hr )
{
	switch ( hr )
	{
	case D3D_OK: return "D3D_OK"; 
	case D3DERR_CONFLICTINGRENDERSTATE: return "D3DERR_CONFLICTINGRENDERSTATE"; 
	case D3DERR_CONFLICTINGTEXTUREFILTER: return "D3DERR_CONFLICTINGTEXTUREFILTER"; 
	case D3DERR_CONFLICTINGTEXTUREPALETTE: return "D3DERR_CONFLICTINGTEXTUREPALETTE"; 
	case D3DERR_DEVICELOST: return "D3DERR_DEVICELOST"; 
	case D3DERR_DEVICENOTRESET: return "D3DERR_DEVICENOTRESET"; 
	case D3DERR_DRIVERINTERNALERROR: return "D3DERR_DRIVERINTERNALERROR"; 
	case D3DERR_INVALIDCALL: return "D3DERR_INVALIDCALL"; 
	case D3DERR_INVALIDDEVICE: return "D3DERR_INVALIDDEVICE"; 
	case D3DERR_MOREDATA: return "D3DERR_MOREDATA"; 
	case D3DERR_NOTAVAILABLE: return "D3DERR_NOTAVAILABLE"; 
	case D3DERR_NOTFOUND: return "D3DERR_NOTFOUND"; 
	case D3DERR_OUTOFVIDEOMEMORY: return "D3DERR_OUTOFVIDEOMEMORY"; 
	case D3DERR_TOOMANYOPERATIONS: return "D3DERR_TOOMANYOPERATIONS"; 
	case D3DERR_UNSUPPORTEDALPHAARG: return "D3DERR_UNSUPPORTEDALPHAARG"; 
	case D3DERR_UNSUPPORTEDALPHAOPERATION: return "D3DERR_UNSUPPORTEDALPHAOPERATION"; 
	case D3DERR_UNSUPPORTEDCOLORARG: return "D3DERR_UNSUPPORTEDCOLORARG"; 
	case D3DERR_UNSUPPORTEDCOLOROPERATION: return "D3DERR_UNSUPPORTEDCOLOROPERATION"; 
	case D3DERR_UNSUPPORTEDFACTORVALUE: return "D3DERR_UNSUPPORTEDFACTORVALUE"; 
	case D3DERR_UNSUPPORTEDTEXTUREFILTER: return "D3DERR_UNSUPPORTEDTEXTUREFILTER"; 
	case D3DERR_WRONGTEXTUREFORMAT: return "D3DERR_WRONGTEXTUREFORMAT"; 
	case E_FAIL: return "E_FAIL";
	case E_INVALIDARG: return "E_INVALIDARG";
	case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
	default: return "D3DERR_UNKNOWN";
	}
}
