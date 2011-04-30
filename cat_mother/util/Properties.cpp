#include "Properties.h"
#include <io/InputStream.h>
#include <io/OutputStream.h>
#include <io/InputStreamReader.h>
#include <io/OutputStreamWriter.h>
#include <io/IOException.h>
#include <lang/Character.h>
#include <util/Vector.h>
#include <algorithm>
#include <string.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

/** Number of supported escape sequences. */
static const int	CONVERSIONS = 6;

/** Escape sequence targets. -1 marks special sequence. */
static const Char	s_target[ CONVERSIONS ] =
{
	Char(0x10), 
	Char(0x9), 
	Char(0x5c), 
	Char(0x13), 
	Char(0x23), 
	Char(-1)
};

/** Escape sequence sources. */
static const char*	s_source[ CONVERSIONS ] =
{
	"\\n",	// new line
	"\\t",	// horizontal tab
	"\\\\",	// '\' character
	"\\r",	// carriage return
	"\\#",	// '#' character
	"\\u"	// Unicode literal
};

//-----------------------------------------------------------------------------

namespace util
{


Properties::Properties() :
	Hashtable<String,String>( Allocator< HashtablePair<String,String> >(__FILE__,__LINE__) )
{
}

void Properties::load( io::InputStream* in )
{
	InputStreamReader 	reader	( in );
	int 				line 	= 1;
	int					chbuf	= -1;
	
	while ( skipWhitespace(&reader,&chbuf,&line) )
	{
		if ( '#' == peekChar(&reader,&chbuf) )
		{
			// comment line
			readLine( &reader, &chbuf, &line );
		}
		else
		{
			// property name
			String name = readString( &reader, &chbuf, &line );

			// assignment operator
			skipWhitespace( &reader, &chbuf, &line );
			Char ch = readChar( &reader, &chbuf, &line );
			if ( ch != '=' )
				throw IOException( Format("{0}({1,#}): Missing assignment operator", in->toString(), line) );

			// property value
			String str = readLine( &reader, &chbuf, &line );

			// remove in-line comments
			for ( int i = 0 ; i < str.length() ; ++i )
			{
				if ( str.charAt(i) == '#' )
				{
					str = str.substring( 0, i );
					break;
				}
			}

			// insert to map
			put( name, str.trim() );
		}
	}
}

void Properties::store( OutputStream* out, const String& header )
{
	OutputStreamWriter writer( out, "UTF-8" );

	writeString( &writer, "# " );
	writeString( &writer, header );
	writeString( &writer, "\n\n" );

	Vector<String> keys( Allocator<String>(__FILE__,__LINE__) );
	for ( HashtableIterator<String,String> it = begin() ; it != end() ; ++it )
		keys.add( it.key() );
	std::sort( keys.begin(), keys.end() );

	for ( int i = 0 ; i < keys.size() ; ++i )
	{
		writeString( &writer, keys[i] );
		writeString( &writer, " = " );
		writeString( &writer, get(keys[i]) );
		writeString( &writer, "\n" );
	}
}

Char Properties::peekChar( Reader* reader, int* chbuf )
{
	if ( -1 == *chbuf )
	{
		// read new
		Char ch = 0;
		while ( -1 == *chbuf && reader->read(&ch,1) > 0 )
		{
			// ignore cr
			if ( 13 == ch )
			{
				ch = 0;
				continue;
			}
				
			*chbuf = ch;
		}
		return ch;
	}
	else
	{
		// return old
		return Char( *chbuf );
	}
}

Char Properties::readChar( Reader* reader, int* chbuf, int* line )
{
	// peek and pop
	Char ch = peekChar( reader, chbuf );
	*chbuf = -1;

	// update line counter	
	if ( 10 == ch )
		*line += 1;
	return ch;
}

bool Properties::skipWhitespace( Reader* reader, int* chbuf, int* line )
{
	// peek and pop until non-whitespace
	Char ch;
	while ( 0 != (ch=peekChar(reader,chbuf)) )
	{
		if ( !Character::isWhitespace(ch) )
			break;
		readChar( reader, chbuf, line );
	}
	return 0 != ch;
}

String Properties::readString( Reader* reader, int* chbuf, int* line )
{
	skipWhitespace( reader, chbuf, line );

	const int 	BUFLEN = 256;
	Char 		buf[BUFLEN];
	int 		len = 0;
	Char 		ch = 0;
	String 		str = "";
	
	while ( 0 != (ch=peekChar(reader, chbuf)) )
	{
		if ( Character::isWhitespace(ch) || '=' == ch )
			break;
		readChar( reader, chbuf, line );
		
		buf[len++] = ch;
		buf[len] = 0;
		
		if ( len > BUFLEN-2 )
		{
			str = str + buf;
			len = 0;
		}
	}
	
	if ( len > 0 )
		str = str + buf;
	if ( 0 == str.length() )
		throw IOException( Format("{0}({1,#}): Failed to read string", reader->toString(), *line) );
	return str;
}

String Properties::readLine( Reader* reader, int* chbuf, int* line )
{
	const int 	BUFLEN = 256;
	Char 		buf[BUFLEN];
	int 		len = 0;
	Char 		ch = 0;
	String 		str = "";
	bool		ws = true;
	
	while ( 0 != (ch=peekChar(reader, chbuf)) )
	{
		readChar( reader, chbuf, line );

		if ( 10 == ch )
			break;
		if ( ws && Character::isWhitespace(ch) )
			continue;
		ws = false;
		
		buf[len++] = ch;
		buf[len] = 0;
		
		if ( len > BUFLEN-2 )
		{
			str = str + buf;
			len = 0;
		}
	}
	
	if ( len > 0 )
		str = str + buf;
	return str;
}

void Properties::writeString( io::Writer* writer, const String& str )
{
	const int BUFSIZE = 32;
	Char buf[BUFSIZE];

	for ( int i = 0 ; i < str.length() ; )
	{
		int count = str.length() - i;
		if ( count > BUFSIZE )
			count = BUFSIZE;
			
		for ( int k = 0 ; k < count ; ++k )
			buf[k] = str.charAt(i+k);
		i += count;
			
		writer->write( buf, count );
	}
}

int Properties::hexToInt( Char ch, int* error ) const
{
	switch ( ch )
	{
	case '0':	return 0;
	case '1':	return 1;
	case '2':	return 2;
	case '3':	return 3;
	case '4':	return 4;
	case '5':	return 5;
	case '6':	return 6;
	case '7':	return 7;
	case '8':	return 8;
	case '9':	return 9;
	case 'a':	return 0xA;
	case 'b':	return 0xB;
	case 'c':	return 0xC;
	case 'd':	return 0xD;
	case 'e':	return 0xE;
	case 'f':	return 0xF;
	case 'A':	return 0xA;
	case 'B':	return 0xB;
	case 'C':	return 0xC;
	case 'D':	return 0xD;
	case 'E':	return 0xE;
	case 'F':	return 0xF;
	default:	*error = 1;
	}
	return 0;
}


} // util
