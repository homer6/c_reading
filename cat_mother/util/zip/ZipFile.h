#ifndef _UTIL_ZIP_ZIPFILE_H
#define _UTIL_ZIP_ZIPFILE_H


#include <io/InputStreamArchive.h>


namespace io {
	class InputStream;}

namespace lang {
	class String;}


namespace util 
{ 
namespace zip
{


class ZipEntry;


/** 
 * Zip file archive. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ZipFile :
	public io::InputStreamArchive
{
public:
	/** Opens a zip file. */
	explicit ZipFile( const lang::String& name );

	/** Closes the zip file. */
	void				close();

	/** Returns input stream to specified entry. */
	io::InputStream*	getInputStream( const lang::String& name );

	/** Returns input stream to ith entry. */
	io::InputStream*	getInputStream( int index );

	/** Returns ith entry. */
	lang::String		getEntry( int index ) const;

	/** Returns number of entries in the archive. */
	int					size() const;

	/** Returns name of the archive. */
	lang::String		toString() const;

private:
	class ZipFileImpl;
	P(ZipFileImpl) m_this;

	ZipFile( const ZipFile& );
	ZipFile& operator=( const ZipFile& );
};


} // zip
} // util


#endif // _UTIL_ZIP_ZIPFILE_H
