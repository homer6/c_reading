#ifndef _COMINITIALIZER_H
#define _COMINITIALIZER_H


#ifndef WIN32
#error Cannot use COM on non-Win32 platform
#endif


#include <lang/Exception.h>
#include <objbase.h>


namespace music
{


/** 
 * Initializes/deinitializes COM. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class COMInitializer
{
public:
	COMInitializer()
	{
		HRESULT hr = CoInitialize(0);
		if ( hr != S_OK )
			throw lang::Exception( lang::Format("Failed to initialized COM") );
	}

	~COMInitializer()
	{
		CoUninitialize();
	}

private:
	COMInitializer( const COMInitializer& );
	COMInitializer& operator=( const COMInitializer& );
};


} // music


#endif // _COMINITIALIZER_H
