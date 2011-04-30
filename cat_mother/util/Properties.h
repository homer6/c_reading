#ifndef _UTIL_PROPERTIES_H
#define _UTIL_PROPERTIES_H


#include <lang/String.h>
#include <util/Hashtable.h>


namespace io {
	class Reader;
	class Writer;
	class OutputStream;
	class InputStream;}


namespace util
{


/** 
 * The Properties class represents a persistent set of properties. 
 * The Properties can be saved to a stream or loaded from a stream. 
 * A property file entry consists of the property name, assignment operator, and the value.
 * Property files can contain comment lines starting with '#'.
 *
 * Property file example:
 * <pre>
 * # this is a comment
 * MyProperty = 1.23
 * AnotherProperty = Hello, world
 * </pre>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Properties :
	public Hashtable< lang::String , lang::String >
{
public:
	/** Creates an emptry property list. */
	Properties();

	/** 
	 * Reads property list from the stream. 
	 * @exception IOException
	 */
	void	load( io::InputStream* in );

	/**
	 * Writes property list to the stream.
	 * @exception IOException
	 */
	void	store( io::OutputStream* out, const lang::String& header );
	
private:
	lang::Char		peekChar( io::Reader* reader, int* chbuf );
	lang::Char 		readChar( io::Reader* reader, int* chbuf, int* line );
	lang::String	readLine( io::Reader* reader, int* chbuf, int* line );
	lang::String	readString( io::Reader* reader, int* chbuf, int* line );
	bool			skipWhitespace( io::Reader* reader, int* chbuf, int* line );
	void			writeString( io::Writer* writer, const lang::String& str );
	int				hexToInt( lang::Char ch, int* err ) const;
};


} // util


#endif // _UTIL_PROPERTIES_H
