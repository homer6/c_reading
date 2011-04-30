// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SD_DX8_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GD_D3D8_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifdef SD_DX8_EXPORTS
#define SD_DX8_API extern "C" __declspec(dllexport)
#else
#define SD_DX8_API extern "C" __declspec(dllimport)
#endif


namespace sd {
	class SoundDriver;}


SD_DX8_API sd::SoundDriver* createSoundDriver();
