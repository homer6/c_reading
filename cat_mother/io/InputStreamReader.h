#ifndef _IO_INPUTSTREAMREADER_H
#define _IO_INPUTSTREAMREADER_H


#include <io/Reader.h>


namespace io
{


class InputStream;


/**
 * Class for reading characters from byte stream.
 * Supported encodings:
 * <ul>
 * <li>ASCII-7
 * <li>UTF-8
 * <li>UTF-16BE
 * <li>UTF-16LE
 * <li>UTF-32BE
 * <li>UTF-32LE
 * </ul>
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class InputStreamReader :
	public Reader
{
public:
	/** 
	 * Starts reading characters from specified stream. 
	 * Tries to guess used encoding.
	 */
	explicit InputStreamReader( InputStream* in );

	/** 
	 * Starts reading characters from specified stream. 
	 * Uses specified encoding if no byte order mark present. 
	 * @exception IOException
	 * @exception UnsupportedEncodingException
	 */
	explicit InputStreamReader( InputStream* in, const char* encoding );

	///
	~InputStreamReader();

	/**
	 * Reads specified number of characters from the stream.
	 * Skips malformed byte sequences.
	 * @exception IOException
	 * @return Number of character actually read. -1 if end-of-stream occured.
	 */
	long	read( lang::Char* buffer, long count );

	/** Closes the stream. */
	void	close();

	/**
	 * Marks the current position in this input stream.
	 * Following calls to the reset method restore
	 * this stream to the last marked position.
	 * If marking is not supported for the stream then 
	 * markSupported() returns false and mark and reset do nothing.
	 * @param readlimit Maximum number of characters to peek.
	 * @exception IOException
	 */
	void	mark( int readlimit );

	/**
	 * Restore this stream to the last marked position.
	 * @see mark
	 * @exception IOException
	 */
	void	reset();

	/**
	 * Returns true as InputStreamReader supports marking.
	 * @see mark
	 * @exception IOException
	 */
	bool	markSupported() const;

	/**
	 * Returns true if the input stream is ready to be read.
	 * @exception IOException
	 */
	bool	ready() const;

	/** Returns name of the reader. */
	lang::String	toString() const;

private:
	class InputStreamReaderImpl;
	P(InputStreamReaderImpl)	m_this;

	InputStreamReader();
	InputStreamReader( const InputStreamReader& );
	InputStreamReader& operator=( const InputStreamReader& );
};


} // io


#endif // _IO_INPUTSTREAMREADER_H
