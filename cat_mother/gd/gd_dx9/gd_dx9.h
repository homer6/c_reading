// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GD_DX8_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GD_DX8_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifdef GD_DX9_EXPORTS
#define GD_DX9_API extern "C" __declspec(dllexport)
#else
#define GD_DX9_API extern "C" __declspec(dllimport)
#endif


namespace gd {
	class GraphicsDriver;}


GD_DX9_API	gd::GraphicsDriver*	createGraphicsDriver();
