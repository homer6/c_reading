#include "StdAfx.h"
#include "id_dx8.h"
#include "Dx8InputDriver.h"
#include "config.h"

HANDLE g_module;

//-----------------------------------------------------------------------------

BOOL APIENTRY DllMain( HANDLE module, 
                       DWORD  reason, 
                       LPVOID )
{
	g_module = module;

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

ID_DX8_API	id::InputDriver* createInputDriver()
{
	return new Dx8InputDriver;
}

ID_DX8_API int getInputDriverVersion()
{
	return Dx8InputDriver::VERSION;
}

