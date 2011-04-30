#include "StdAfx.h"
#include "gd_dx9.h"
#include "Dx9GraphicsDriver.h"
#include "config.h"

//-----------------------------------------------------------------------------

BOOL APIENTRY DllMain( HANDLE /*module*/, 
                       DWORD  reason, 
                       LPVOID )
{
    switch ( reason )
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

GD_DX9_API	gd::GraphicsDriver*	createGraphicsDriver()
{
	return new Dx9GraphicsDriver;
}

GD_DX9_API int getGraphicsDriverVersion()
{
	return Dx9GraphicsDriver::VERSION;
}
