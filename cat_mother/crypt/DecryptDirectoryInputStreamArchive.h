#ifndef _CRYPT_DECRYPTDIRECTORYINPUTSTREAMARCHIVE_H
#define _CRYPT_DECRYPTDIRECTORYINPUTSTREAMARCHIVE_H


#include <io/InputStreamArchive.h>


namespace crypt
{


/** 
 * Class for decrypting input streams from multiple directories. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DecryptDirectoryInputStreamArchive :
	public io::InputStreamArchive
{
public:
	///
	DecryptDirectoryInputStreamArchive();

	///
	~DecryptDirectoryInputStreamArchive();
	
	/** Adds a directory to the list of searched paths. */
	void				addPath( const lang::String& path );

	/** Adds a directory and all subdirs to the list of searched paths. */
	void				addPaths( const lang::String& path );

	/** Removes all search paths. */
	void				removePaths();

	/** Closes the archive. */
	void				close();

	/** 
	 * Returns input stream to specified entry. 
	 * @exception IOException
	 */
	io::InputStream*	getInputStream( const lang::String& name );

	/** 
	 * Returns input stream to ith entry. 
	 * @exception IOException
	 */
	io::InputStream*	getInputStream( int index );

	/** 
	 * Returns ith entry name from the archive. 
	 * @exception IOException
	 */
	lang::String		getEntry( int index ) const;

	/** 
	 * Returns number of entries in the archive. 
	 * @exception IOException
	 */
	int					size() const;

	/** Returns name of the archive. */
	lang::String		toString() const;

private:
	class DecryptDirectoryInputStreamArchiveImpl;
	P(DecryptDirectoryInputStreamArchiveImpl) m_this;

	DecryptDirectoryInputStreamArchive( const DecryptDirectoryInputStreamArchive& );
	DecryptDirectoryInputStreamArchive& operator=( const DecryptDirectoryInputStreamArchive& );
};


} // crypt


#endif // _CRYPT_DECRYPTDIRECTORYINPUTSTREAMARCHIVE_H
