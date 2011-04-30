#ifndef _IO_DATAINPUTSTREAM_H
#define _IO_DATAINPUTSTREAM_H


#include <io/FilterInputStream.h>
#include <lang/String.h>
#include <stdint.h>


namespace io
{


/**
 * Class for reading primitive types from the input stream in portable way.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DataInputStream :
	public FilterInputStream
{
public:
	///
	explicit DataInputStream( InputStream* in );

	///
	~DataInputStream();

	/**
	 * Tries to skip over n bytes from the stream.
	 *
	 * @return Number of bytes actually skipped.
	 * @exception IOException
	 */
	long skip( long n );

	/**
	 * Tries to read specified number of bytes from the source stream.
	 * Doesn't block the caller if specified number of bytes isn't available.
	 *
	 * @return Number of bytes actually read.
	 * @exception IOException
	 */
	long read( void* data, long size );

	/**
	 * Reads specified number of bytes from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	void readFully( void* data, long size );

	/**
	 * Reads boolean from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	bool readBoolean();

	/**
	 * Reads 8-bit signed integer from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	uint8_t readByte();

	/**
	 * Reads character from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	lang::Char readChar();

	/**
	 * Reads character sequence from the stream.
	 *
	 * @param n Number of characters to read.
	 * @exception EOFException
	 * @exception IOException
	 */
	lang::String readChars( int n );

	/**
	 * Reads double value from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	double readDouble();

	/**
	 * Reads float from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	float readFloat();

	/**
	 * Reads 32-bit signed integer from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	int readInt();

	/**
	 * Reads 64-bit signed integer from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	long readLong();

	/**
	 * Reads 16-bit signed integer from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	int readShort();

	/**
	 * Reads string encoded in UTF-8 from the stream.
	 *
	 * @exception EOFException
	 * @exception IOException
	 */
	lang::String readUTF();

	/**
	 * Returns number of bytes read with this DataInputStream.
	 */
	long	size() const;

private:
	void*	m_inBuffer;
	int		m_inBufferSize;
	long	m_size;

	DataInputStream();
	DataInputStream( const DataInputStream& );
	DataInputStream& operator=( const DataInputStream& );
};


} // io


#endif // _IO_DATAINPUTSTREAM_H
