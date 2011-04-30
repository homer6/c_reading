#ifndef _IO_COMMANDREADER_H
#define _IO_COMMANDREADER_H


#include <lang/Object.h>
#include <lang/String.h>


namespace io {
	class Reader;}


namespace io
{


/** 
 * Class for reading simple text file commands and parameters. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class CommandReader :
	public lang::Object
{
public:
	///
	explicit CommandReader( io::Reader* reader, const lang::String& name );

	/** 
	 * Skips whitespace characters. 
	 * @exception IOException
	 */
	void		skipWhitespace();
	
	/**
	 * Reads a whitespace-delimited string from the stream. 
	 * @return false if end-of-file occured.
	 * @exception IOException
	 */
	bool		readString( lang::String& str );

	/**
	 * Reads a line from the stream. 
	 * Line is delimited by \n or \r or \n\r.
	 * @return false if end-of-file occured.
	 * @exception IOException
	 */
	bool		readLine( lang::String& str );

	/**
	 * Reads a long integer using US/English locale.
	 * @exception IOException
	 */
	long		readLong();

	/**
	 * Reads an integer using US/English locale.
	 * @exception IOException
	 */
	int			readInt();

	/**
	 * Reads a floating point value using US/English locale.
	 * @exception IOException
	 */
	float		readFloat();

	/**
	 * Reads a single hex digit.
	 * @exception IOException
	 */ 
	int			readHex();

	/**
	 * Single character look-ahead.
	 * @exception IOException
	 */
	lang::Char	peekChar();

	/** Returns name of the command stream. */
	const lang::String&	name() const;

	/** Returns line number of the current read position. */
	int					line() const;

private:
	io::Reader*			m_reader;
	int					m_line;
	int					m_lineMark;
	lang::String		m_name;

	bool readChar( lang::Char* ch );
	void mark1();
	void reset1();
	
	CommandReader( const CommandReader& );
	CommandReader& operator=( const CommandReader& );
};


} // io


#endif // _IO_COMMANDREADER_H
