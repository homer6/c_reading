#include "Formattable.h"
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


static void parseNumberPattern( 
	const String& pattern, int pos,
	int& minIntDigits, int& maxIntDigits,
	int& minFracDigits, int& maxFracDigits,
	bool& fraction, bool& grouping, char& hex )
{
	const int absMaxIntDigits = 20;

	minIntDigits		= 0;
	maxIntDigits		= 0;
	minFracDigits		= 0;
	maxFracDigits		= 0;
	fraction			= false;
	grouping			= false;
	hex					= 0;

	int patternEnd = pos;
	for ( ; patternEnd < pattern.length() ; ++patternEnd )
		if ( pattern.charAt(patternEnd) == Char('}') )
			break;

	for ( int i = pos ; i < patternEnd ; ++i )
	{
		Char ch = pattern.charAt(i);

		if ( ch == 'x' || ch == 'X' )
		{
			hex = (char)ch;
			break;
		}
		else if ( ch == Char('.') )
		{
			fraction = true;
		}
		else if ( ch == Char(',') )
		{
			grouping = true;
		}
		else if ( ch == Char('0') )
		{
			// compulsory digit
			if ( fraction )
				++minFracDigits, ++maxFracDigits;
			else
				++minIntDigits, maxIntDigits=absMaxIntDigits;
		}
		else if ( ch == Char('#') )
		{
			// optional digit
			if ( fraction )
				++maxFracDigits;
			else
				maxIntDigits=absMaxIntDigits;
		}
	}
}

//-----------------------------------------------------------------------------

Formattable::Formattable()
{
	m_type = VALUE_NONE;
}

Formattable::Formattable( double value )
{
	m_type = VALUE_DOUBLE;
	m_dbl = value;
}

Formattable::Formattable( const String& value )
{
	m_type = VALUE_STRING;
	m_str = value;
}

Formattable::Formattable( const char* value )
{
	m_type = VALUE_STRING;
	m_str = value;
}

Formattable::ValueType Formattable::type() const
{
	return m_type;
}

double Formattable::doubleValue() const
{
	assert( m_type == VALUE_DOUBLE );
	return m_dbl;
}

String Formattable::stringValue() const
{
	assert( m_type == VALUE_STRING );
	return m_str;
}

int	Formattable::format( Char* buffer, int size, const String& pattern, int pos ) const
{
	if ( VALUE_DOUBLE == m_type )
	{
		int minIntDigits, maxIntDigits;
		int minFracDigits, maxFracDigits;
		bool fraction, grouping;
		char hex;
		parseNumberPattern( pattern, pos, minIntDigits, maxIntDigits, minFracDigits, maxFracDigits, fraction, grouping, hex );

		bool chopInt = false;
		char fmt[32];
		if ( hex )
			sprintf( fmt, "%%%c", hex ), chopInt = true;
		else if ( 0 == maxFracDigits && 0 == maxIntDigits )
			sprintf( fmt, "%%g" );
		else if ( minIntDigits > 0 )
			sprintf( fmt, "%%0%i.%if", minIntDigits, maxFracDigits );
		else
			sprintf( fmt, "%%.%if", maxFracDigits );

		char buff[32];
		if ( chopInt )
			sprintf( buff, fmt, (int)m_dbl );
		else
			sprintf( buff, fmt, m_dbl );

		int needed = strlen(buff);
		int count = needed;
		if ( count > size ) 
			count = size;
		for ( int i = 0 ; i < count ; ++i )
			buffer[i] = (Char)(unsigned char)buff[i];
		if ( needed < size )
			buffer[needed] = 0;
		else if ( size > 0 )
			buffer[size-1] = 0;
		return needed;
	}
	else if ( VALUE_STRING == m_type )
	{
		int needed = m_str.length();
		int count = needed;
		if ( count > size ) 
			count = size;
		for ( int i = 0 ; i < count ; ++i )
			buffer[i] = m_str.charAt(i);
		if ( needed < size )
			buffer[needed] = 0;
		else if ( size > 0 )
			buffer[size-1] = 0;
		return needed;
	}
	return 0;
}


} // lang
