#ifndef _IO_CHUNKWRITER_H
#define _IO_CHUNKWRITER_H


#include <io/FilterOutputStream.h>
#include <lang/Object.h>
#include <lang/String.h>


namespace io
{


class OutputStream;


/** 
 * Binary data chunk writer. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ChunkOutputStream :
	public FilterOutputStream
{
public:
	///
	explicit ChunkOutputStream( OutputStream* out );

	///
	~ChunkOutputStream();

	/** 
	 * Begins writing a new chunk. 
	 * @param name The chunk name.
	 * @exception IOException
	 */
	void			beginChunk( const lang::String& name );

	/** 
	 * Ends writing the chunk.
	 * @exception IOException
	 */
	void			endChunk();

	/**
	 * Writes a byte to the chunk.
	 * @exception IOException
	 */
	void			writeByte( int v );

	/**
	 * Write a variable-length integer to the chunk.
	 * @exception IOException
	 */
	void			writeInt( int v );

	/**
	 * Write a float to the chunk.
	 * @exception IOException
	 */
	void			writeFloat( float v );

	/**
	 * Write a UTF-encoded string to the chunk.
	 * @exception IOException
	 */
	void			writeString( const lang::String& v );

	/** Writes pending chunks to the output stream. */
	void			close();

	/** 
	 * Returns number of bytes written to the stream. 
	 * @exception IOException
	 */
	long			size() const;

private:
	class ChunkOutputStreamImpl;
	P(ChunkOutputStreamImpl)	m_this;

	ChunkOutputStream( const ChunkOutputStream& );
	ChunkOutputStream& operator=( const ChunkOutputStream& );
};


} // io


#endif // _IO_CHUNKWRITER_H
