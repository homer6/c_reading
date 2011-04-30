#ifndef _IO_STDIOEX_H
#define _IO_STDIOEX_H


#include <stdio.h>


namespace lang {
	class String;}


namespace io
{

	
/** 
 * fopen with Unicode file name support.
 *
 * The function supports wide character file names if the platform supports them.
 * Non-representable characters are skipped.
 *
 * @param filename Relative or absolute path of the file to be opened.
 * @param access Type of access permitted. Same as with Standard C fopen.
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
FILE*	
	fopen( const lang::String& filename, const char* access );


} // io


#endif // _IO_STDIOEX_H
