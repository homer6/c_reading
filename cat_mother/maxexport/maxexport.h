#ifdef MAXEXPORT_EXPORTS
#define MAXEXPORT_API extern "C" __declspec(dllexport) 
#else
#define MAXEXPORT_API extern "C" __declspec(dllimport)
#endif


MAXEXPORT_API int			LibNumberClasses();
MAXEXPORT_API ClassDesc*	LibClassDesc( int i );
MAXEXPORT_API const TCHAR*	LibDescription();
MAXEXPORT_API ULONG			LibVersion();
