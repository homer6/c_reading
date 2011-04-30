#ifndef _IO_FILEINPUTSTREAM_H
#define _IO_FILEINPUTSTREAM_H


#include <io/InputStream.h>


namespace lang {
	class String;}


namespace io
{


/**
 * FileInputStream reads bytes from a file in a file system.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class FileInputStream :
	public InputStream
{
public:
	/** 
	 * Opens a file input stream. 
	 *
	 * @exception FileNotFoundException
	 */
	explicit FileInputStream( const lang::String& filename );

	///
	~FileInputStream();

	/**
	 * Closes the stream and releases associated resources.
	 * Closed stream cannot perform any operations and cannot be reopened.
	 */
	void	close();

	/**
	 * Tries to read specified number of bytes from the stream.
	 * Doesn't block the caller if specified number of bytes isn't available.
	 *
	 * @return Number of bytes actually read.
	 * @exception IOException
	 */
	long	read( void* data, long size );

	/** 
	 * Returns the number of bytes that can be read from the stream without blocking.
	 *
	 * @exception IOException
	 */
	long	available() const;

	/** Returns name of the file. */
	lang::String	toString() const;

private:
	class FileInputStreamImpl;
	P(FileInputStreamImpl) m_this;

	FileInputStream();
	FileInputStream( const FileInputStream& );
	FileInputStream& operator=( const FileInputStream& );
};


} // io


#endif // _IO_FILEINPUTSTREAM_H
