#include "StdAfx.h"
#include "maxexport.h"
#include "MySceneExportClassDesc.h"

//-----------------------------------------------------------------------------

static bool						s_controlsInit				= false;
static MySceneExportClassDesc	s_classDescMySceneExport;

//-----------------------------------------------------------------------------

BOOL APIENTRY DllMain( HINSTANCE instance, DWORD reason, void* /*reserved*/ )
{
	s_classDescMySceneExport.instance = instance;
	
	if ( !s_controlsInit )
	{
		s_controlsInit = true;
		InitCustomControls( instance );
		InitCommonControls();
	}

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

MAXEXPORT_API int LibNumberClasses()
{
	return 1;
}

MAXEXPORT_API ClassDesc* LibClassDesc( int i )
{
	switch ( i )
	{
	case 0:		return &s_classDescMySceneExport;
	default:	return 0;
	}
}

MAXEXPORT_API const TCHAR* LibDescription()
{
	return _T("sgexport");
}

MAXEXPORT_API ULONG	LibVersion()
{
	return VERSION_3DSMAX;
}
