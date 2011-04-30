#include "CommandReader.h"
#include <io/Reader.h>
#include <io/IOException.h>
#include <lang/String.h>
#include <lang/Character.h>
#include <lang/NumberReader.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


CommandReader::CommandReader( Reader* reader, const String& name )
{
	m_reader	= reader;
	m_line		= 1;
	m_lineMark	= 1;
	m_name		= name;
}

void CommandReader::skipWhitespace()
{
	mark1();
	Char ch;
	while ( readChar(&ch) )
	{
		if ( !Character::isWhitespace(ch) )
		{
			reset1();
			break;
		}
		mark1();
	}
}

bool CommandReader::readString( String& str )
{
	skipWhitespace();

	str = "";
	Char sz[64];
	Char ch;
	int i = 0;
	mark1();
	while ( readChar(&ch) )
	{
		if ( Character::isWhitespace(ch) )
		{
			reset1();
			break;
		}

		sz[i] = ch;
		sz[++i] = 0;

		if ( i > 60 )
		{
			str = str + sz;
			i = 0;
		}

		mark1();
	}
	if ( i > 0 )
		str = str + sz;
	
	return str.length() > 0;
}

bool CommandReader::readLine( String& str )
{
	skipWhitespace();

	str = "";
	Char sz[64];
	Char ch;
	int i = 0;
	while ( readChar(&ch) )
	{
		if ( 13 == ch || 10 == ch )
		{
			// skip line feeds and carriage returns
			mark1();
			while ( readChar(&ch) )
			{
				if ( 13 != ch && 10 != ch )
				{
					reset1();
					break;
				}
				mark1();
			}
			break;
		}

		sz[i] = ch;
		sz[++i] = 0;

		if ( i > 60 )
		{
			str = str + sz;
			i = 0;
		}
	}
	if ( i > 0 )
		str = str + sz;
	
	return str.length() > 0;
}

long CommandReader::readLong()
{
	skipWhitespace();

	NumberReader<long> nr;
	Char ch;
	mark1();
	while ( readChar(&ch) )
	{
		if ( ch >= 0x80 || 0 == nr.put((char)ch) )
		{
			reset1();
			break;
		}
		mark1();
	}

	if ( !nr.valid() )
		throw IOException( Format("Failed to parse an integer: {0}({1})", m_name, m_line) );
	return nr.value();
}

int	CommandReader::readInt()
{
	return (int)readLong();
}

float CommandReader::readFloat()
{
	skipWhitespace();

	NumberReader<float> nr;
	Char ch;
	mark1();
	while ( readChar(&ch) )
	{
		if ( ch >= 0x80 || 0 == nr.put((char)ch) )
		{
			reset1();
			break;
		}
		mark1();
	}

	if ( !nr.valid() )
		throw IOException( Format("Failed to parse a float: {0}({1})", m_name, m_line) );
	return nr.value();
}

int CommandReader::readHex()
{
	skipWhitespace();

	Char ch;
	if ( !readChar(&ch) )
		throw IOException( Format("Failed to read a hex digit: {0}({1})", m_name, m_line) );

	Char hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	int i;
	for ( i = 0 ; i < 16 ; ++i )
	{
		if ( hex[i] == ch )
			break;
	}
	if ( i >= 16 )
		throw IOException( Format("Failed to read a hex digit: {0}({1})", m_name, m_line) );
	
	return i;
}

Char CommandReader::peekChar()
{
	mark1();
	Char ch;
	if ( !readChar(&ch) )
		throw IOException( Format("Failed to peek a character: {0}({1})", m_name, m_line) );
	reset1();
	return ch;
}

const String& CommandReader::name() const
{
	return m_name;
}

int CommandReader::line() const
{
	return m_line;
}

bool CommandReader::readChar( Char* ch )
{
	bool ok = ( 1 == m_reader->read(ch,1) );
	if ( ok && *ch == 10 )
		++m_line;
	return ok;
}

void CommandReader::mark1()
{
	m_reader->mark( 1 );
	m_lineMark = m_line;
}

void CommandReader::reset1()
{
	m_reader->reset();
	m_line = m_lineMark;
}


} // io
