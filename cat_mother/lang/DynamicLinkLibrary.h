#ifndef _LANG_DYNAMICLINKLIBRARY_H
#define _LANG_DYNAMICLINKLIBRARY_H


#include <lang/Object.h>
#include <lang/String.h>


namespace lang
{


/** 
 * Platform independent interface to dynamic link library functionality. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DynamicLinkLibrary :
	public lang::Object
{
public:
	/**  
	 * Loads a dynamic link library. Doesnt throw exceptions, check
	 * error() to see if the library loading failed.
	 * @param name Name of the library without debug build identifier (d) and file extension (.dll in Win32)
	 * @exception Exception
	 */
	explicit DynamicLinkLibrary( const lang::String& name );

	///
	~DynamicLinkLibrary();

	/** 
	 * Closes the library. 
	 * The library cannot be used after calling this method. 
	 */
	void	close();

	/** 
	 * Returns procedure address by name or 0 if not found. 
	 * If the library loading failed (error() returns true)
	 * then the function will return always 0.
	 */
	void*	getProcAddress( const lang::String& name ) const;

private:
	class DynamicLinkLibraryImpl;
	P(DynamicLinkLibraryImpl) m_this;

	DynamicLinkLibrary();
	DynamicLinkLibrary( const DynamicLinkLibrary& other );
	DynamicLinkLibrary& operator=( const DynamicLinkLibrary& other );
};


} // lang


#endif // _LANG_DYNAMICLINKLIBRARY_H
