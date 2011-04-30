#include "StdAfx.h"
#include "sd_dx8.h"
#include "Dx8SoundDriver.h"
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

SD_DX8_API sd::SoundDriver* createSoundDriver()
{
	return new Dx8SoundDriver;
}

SD_DX8_API int getSoundDriverVersion()
{
	return Dx8SoundDriver::VERSION;
}
