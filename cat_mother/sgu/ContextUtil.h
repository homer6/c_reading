#ifndef _SGU_CONTEXTUTIL_H
#define _SGU_CONTEXTUTIL_H


#include <sg/Context.h>


namespace lang {
	class String;}


namespace sgu
{


/** 
 * Rendering device context related utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ContextUtil
{
public:
	/** 
	 * Grabs screen image to the current working directory. 
	 * @exception IOException
	 * @exception GraphicsDeviceException
	 */
	static void grabScreen( const lang::String& ext );

	/** 
	 * Converts string to Context::TextureFilterType.
	 * @exception Exception If string does not match any filter type.
	 */
	static sg::Context::TextureFilterType	toTextureFilterType( const lang::String& str );
	
};


} // sgu


#endif // _SGU_CONTEXTUTIL_H
