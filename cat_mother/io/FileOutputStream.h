#ifndef _IO_FILEOUTPUTSTREAM_H
#define _IO_FILEOUTPUTSTREAM_H


#include <io/OutputStream.h>
#include <lang/String.h>


namespace io
{


/**
 * FileOutputStream writes bytes to a file in a file system.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class FileOutputStream :
	public OutputStream
{
public:
	/** 
	 * Opens a file output stream. 
	 *
	 * @exception FileNotFoundException
	 */
	explicit FileOutputStream( const lang::String& filename );

	///
	~FileOutputStream();

	/**
	 * Closes the stream and releases associated resources.
	 * Closed stream cannot perform any operations and cannot be reopened.
	 */
	void	close();

	/**
	 * Forces any buffered output bytes to be written out.
	 *
	 * @exception IOException
	 */
	void	flush();

	/**
	 * Writes specified number of bytes to the stream.
	 *
	 * @exception IOException
	 */
	void	write( const void* data, long size );

	/** Returns name of the file. */
	lang::String	toString() const;

private:
	class FileOutputStreamImpl;
	P(FileOutputStreamImpl) m_this;

	FileOutputStream();
	FileOutputStream( const FileOutputStream& );
	FileOutputStream& operator=( const FileOutputStream& );
};


} // io


#endif // _IO_FILEOUTPUTSTREAM_H
