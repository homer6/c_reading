#include "DynamicLinkLibrary.h"
#include <lang/Array.h>
#include <lang/Exception.h>
#include <assert.h>
#include "config.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

//-----------------------------------------------------------------------------

namespace lang
{


class DynamicLinkLibrary::DynamicLinkLibraryImpl :
	public Object
{
public:
	DynamicLinkLibraryImpl( const char* name );
	~DynamicLinkLibraryImpl();

	void	close();
	void*	getProcAddress( const char* name ) const;

private:
	#ifdef WIN32
	HINSTANCE	m_dll;
	#else
	void*		m_dll;
	#endif

	DynamicLinkLibraryImpl();
	DynamicLinkLibraryImpl( const DynamicLinkLibraryImpl& );
	DynamicLinkLibraryImpl& operator=( const DynamicLinkLibraryImpl& );
};

//-----------------------------------------------------------------------------

DynamicLinkLibrary::DynamicLinkLibraryImpl::DynamicLinkLibraryImpl( const char* name )
{
#ifdef WIN32

	char fname[1024];
	strncpy( fname, name, 1000 );
	fname[sizeof(fname)-10] = 0;
	#ifdef _DEBUG
	strcat( fname, "d" );
	#endif
	strcat( fname, ".dll" );

	m_dll = LoadLibrary( fname );
	if ( !m_dll )
		throw Exception( Format("DLL loading failed: {0}", fname) );

#else

	char fname[1024];
	memset( fname, 0, sizeof(fname) );
	strncpy( fname, name, 1000 );
	fname[sizeof(fname)-10] = 0;
	strcat( fname, ".so" );

	m_dll = dlopen( fname, RTLD_NOW );
	if ( !m_dll )
		throw Exception( Format("Shared object loading failed: {0}", fname) );

#endif
}

DynamicLinkLibrary::DynamicLinkLibraryImpl::~DynamicLinkLibraryImpl()
{
	close();
}

void DynamicLinkLibrary::DynamicLinkLibraryImpl::close()
{
#ifdef WIN32

	if ( m_dll )
	{
		FreeLibrary( m_dll );
		m_dll = 0;
	}

#else

	if ( m_dll )
	{
		dlclose( m_dll );
		m_dll = 0;
	}

#endif
}

void* DynamicLinkLibrary::DynamicLinkLibraryImpl::getProcAddress( const char* name ) const
{
#ifdef WIN32

	void* addr = 0;
	if ( m_dll )
		addr = GetProcAddress( m_dll, name );
	return addr;

#else

	void* addr = 0;
	if ( m_dll )
		addr = dlsym( m_dll, name );
	return addr;

#endif
}

//-----------------------------------------------------------------------------

DynamicLinkLibrary::DynamicLinkLibrary( const String& name )
{
	Array<char,512> sz( name.length()+1 );
	name.getBytes( sz.begin(), sz.size(), "ASCII-7" );
	m_this = new DynamicLinkLibraryImpl( sz.begin() );
}

DynamicLinkLibrary::~DynamicLinkLibrary()
{
}

void* DynamicLinkLibrary::getProcAddress( const lang::String& name ) const
{
	Array<char,512> sz( name.length()+1 );
	name.getBytes( sz.begin(), sz.size(), "ASCII-7" );
	return m_this->getProcAddress( sz.begin() );
}

void DynamicLinkLibrary::close()
{
	m_this->close();
}


} // lang
