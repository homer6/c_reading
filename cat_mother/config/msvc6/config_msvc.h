#ifdef _MSC_VER

	// Disable some warnings

	// unreferenced inline function has been removed
	#pragma warning( disable : 4514 )
	// debug identifier was truncated to 'number' characters in the debug information
	#pragma warning( disable : 4786 )
	// decorated name length exceeded, name was truncated
	#pragma warning( disable : 4503 )
	// variable may be used without having been initialized (some incorrect warnings)
	#pragma warning( disable : 4701 )
	// unreachable code (incorrect warnings when throwing exceptions)
	#pragma warning( disable : 4702 )
	// function not inlined (incorrect warnings with templates as template parameters)
	#pragma warning( disable : 4710 )
	// identifier was truncated to '255' characters in the debug information
	#pragma warning( disable : 4786 )

	// Fix old style for loop scope
	#ifdef __cplusplus
	inline static int forMSVC() {return 0;}
	#else
	static __inline int forMSVC() {return 0;}
	#endif
	#define for if (forMSVC()) {} else for

	// Enable debug global memory allocation if available
	#ifdef LANG_DEBUG_NEW
	#define new LANG_DEBUG_NEW
	#endif

#endif // _MSC_VER
