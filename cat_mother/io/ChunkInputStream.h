#ifndef _IO_CHUNKREADER_H
#define _IO_CHUNKREADER_H


#include <io/DataInputStream.h>
#include <lang/String.h>


namespace io
{


class InputStream;
class DataInputStream;


/** 
 * Binary data chunk reader. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ChunkInputStream :
	public FilterInputStream
{
public:
	///
	explicit ChunkInputStream( InputStream* in );

	/** 
	 * Begins reading a new chunk. 
	 * @param name [out] Buffer for the chunk name.
	 * @param end [out] Receives the end (byte offset) of the chunk data.
	 * @exception EOFException
	 * @exception IOException
	 */
	void			beginChunk( lang::String* name, long* end );

	/** 
	 * Ends reading the chunk. Skips remaining bytes.
	 * @param end The end (byte offset) of the chunk data as returned by beginChunk().
	 * @exception EOFException
	 * @exception IOException
	 */
	void			endChunk( long end );

	/**
	 * Reads a byte from the chunk.
	 * @exception EOFException
	 * @exception IOException
	 */
	uint8_t			readByte();

	/**
	 * Read a variable-length integer from the chunk.
	 * @exception EOFException
	 * @exception IOException
	 */
	int				readInt();

	/**
	 * Read a float from the chunk.
	 * @exception EOFException
	 * @exception IOException
	 */
	float			readFloat();

	/**
	 * Read a float array from the chunk.
	 * @exception EOFException
	 * @exception IOException
	 */
	void			readFloatArray( float* array, int count );

	/**
	 * Read a UTF-encoded string from the chunk.
	 * @exception EOFException
	 * @exception IOException
	 */
	lang::String	readString();

	/** 
	 * Returns number of bytes read from the stream. 
	 * @exception IOException
	 */
	long			size() const;

private:
	DataInputStream m_in;

	ChunkInputStream( const ChunkInputStream& );
	ChunkInputStream& operator=( const ChunkInputStream& );
};


} // io


#endif // _IO_CHUNKREADER_H
