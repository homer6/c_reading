#ifndef _IO_OUTPUTSTREAMWRITER_H
#define _IO_OUTPUTSTREAMWRITER_H


#include <io/Writer.h>


namespace io
{


class OutputStream;


/**
 * Class for writing characters to byte stream.
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
class OutputStreamWriter :
	public Writer
{
public:
	/** 
	 * Starts writing characters to specified stream. 
	 * Tries to guess used encoding.
	 */
	explicit OutputStreamWriter( OutputStream* out );

	/** 
	 * Starts writing characters to specified stream. 
	 * Uses specified encoding. 
	 * @exception IOException
	 * @exception UnsupportedEncodingException
	 */
	explicit OutputStreamWriter( OutputStream* out, const char* encoding );

	///
	~OutputStreamWriter();

	/**
	 * Writes specified number of characters to the stream.
	 * Ignores invalid character pairs.
	 * @exception IOException
	 */
	void	write( const lang::Char* buffer, long count );

	/**
	 * Flush the stream.
	 * @exception IOException
	 */
	void	flush();

	/** Closes the stream. */
	void	close();

	/** Returns name of the writer. */
	lang::String	toString() const;

private:
	class OutputStreamWriterImpl;
	P(OutputStreamWriterImpl)	m_this;

	OutputStreamWriter();
	OutputStreamWriter( const OutputStreamWriter& );
	OutputStreamWriter& operator=( const OutputStreamWriter& );
};


} // io


#endif // _IO_OUTPUTSTREAMWRITER_H
