#ifndef _IO_DATAOUTPUTSTREAM_H
#define _IO_DATAOUTPUTSTREAM_H


#include <io/FilterOutputStream.h>
#include <lang/String.h>


namespace io
{


/**
 * Class for writing primitive types to the output stream in portable way.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DataOutputStream :
	public FilterOutputStream
{
public:
	///
	explicit DataOutputStream( OutputStream* out );

	/**
	 * Writes boolean to the stream.
	 *
	 * @exception IOException
	 */
	void writeBoolean( bool value );

	/**
	 * Writes 8-bit signed integer to the stream.
	 *
	 * @exception IOException
	 */
	void writeByte( int value );

	/**
	 * Writes character to the stream.
	 *
	 * @exception IOException
	 */
	void writeChar( lang::Char value );

	/**
	 * Writes character sequence to the stream.
	 *
	 * @exception IOException
	 */
	void writeChars( const lang::String& value );

	/**
	 * Writes double to the stream.
	 *
	 * @exception IOException
	 */
	void writeDouble( double value );

	/**
	 * Writes float to the stream.
	 *
	 * @exception IOException
	 */
	void writeFloat( float value );

	/**
	 * Writes 32-bit signed integer to the stream.
	 *
	 * @exception IOException
	 */
	void writeInt( int value );

	/**
	 * Writes 64-bit signed integer to the stream.
	 *
	 * @exception IOException
	 */
	void writeLong( long value );

	/**
	 * Writes 16-bit signed integer to the stream.
	 *
	 * @exception IOException
	 */
	void writeShort( int value );

	/**
	 * Writes string encoded in UTF-8 to the stream.
	 *
	 * @exception IOException
	 */
	void writeUTF( const lang::String& value );

	/**
	 * Returns number of bytes written with this DataOutputStream.
	 */
	long	size() const;

private:
	long	m_size;

	DataOutputStream();
	DataOutputStream( const DataOutputStream& );
	DataOutputStream& operator=( const DataOutputStream& );
};


} // io


#endif // _IO_DATAOUTPUTSTREAM_H
