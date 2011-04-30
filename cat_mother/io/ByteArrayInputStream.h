#ifndef _IO_BYTEARRAYINPUTSTREAM_H
#define _IO_BYTEARRAYINPUTSTREAM_H


#include <io/InputStream.h>


namespace io
{


/**
 * ByteArrayInputStream reads bytes from a memory buffer.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ByteArrayInputStream :
	public InputStream
{
public:
	/** 
	 * Creates an input stream from specified memory buffer.
	 * Note that the contents of the buffer is duplicated so the buffer
	 * can be freed immediately after ByteArrayInputStream constructor returns.
	 */
	ByteArrayInputStream( const void* data, long size );

	///
	~ByteArrayInputStream();

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

	/** Returns byte array identifier. */
	lang::String	toString() const;

private:
	char*	m_data;
	long	m_size;
	long	m_index;
	char	m_defaultBuffer[20];

	ByteArrayInputStream();
	ByteArrayInputStream( const ByteArrayInputStream& );
	ByteArrayInputStream& operator=( const ByteArrayInputStream& );
};


} // io


#endif // _IO_BYTEARRAYINPUTSTREAM_H
