#ifndef _IO_BYTEARRAYOUTPUTSTREAM_H
#define _IO_BYTEARRAYOUTPUTSTREAM_H


#include <io/OutputStream.h>


namespace io
{


/**
 * ByteArrayOutputStream writes bytes to a memory buffer.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ByteArrayOutputStream :
	public OutputStream
{
public:
	/** 
	 * Creates an output stream to a memory buffer of specified initial capacity.
	 */
	explicit ByteArrayOutputStream( long size=0 );

	///
	~ByteArrayOutputStream();

	/** Discards all written bytes. */
	void	reset();

	/**
	 * Writes specified number of bytes to the stream.
	 *
	 * @exception IOException
	 */
	void	write( const void* data, long size );

	/**
	 * Writes contents of the stream to the specified array.
	 *
	 * @param data Pointer to the array to receive the stream contents.
	 * @param size Size of the array to receive the stream contents. Must be at least the size of the stream.
	 */
	void	toByteArray( void* data, long size ) const;

	/**
	 * Returns pointer to the memory buffer used by the stream.
	 * Any write operation to the stream can invalidate the returned pointer.
	 */
	const void*	toByteArray() const													{return m_data;}

	/**
	 * Returns number of bytes written to the stream.
	 */
	long	size() const;

	/** Returns byte array identifier. */
	lang::String	toString() const;

private:
	char*	m_data;
	long	m_size;
	long	m_capacity;
	char	m_defaultBuffer[20];

	ByteArrayOutputStream( const ByteArrayOutputStream& );
	ByteArrayOutputStream& operator=( const ByteArrayOutputStream& );
};


} // io


#endif // _IO_BYTEARRAYOUTPUTSTREAM_H
