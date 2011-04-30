#include "NumberReader.h"
#include <math.h>
#include <ctype.h>
#include "config.h"

//----------------------------------------------------------------------------

namespace lang
{


/**
 * Converts ASCII-7 digit character to number. 
 * @returns 0 if the digit cannot be converted.
 */
template <class T> static T digitToNumber( char ch )
{
	// not a digit?
	if ( !(ch >= char('0') && ch <= char('9')) )
		return 0;

	return T( ch - unsigned('0') );
}

/**
 * Returns true if the character is exponent identifier 'e' of floating point value.
 */
static bool expChar( char ch )
{
	// is 'E' or 'e'?
	return (ch == char('e') || ch == char('E'));
}

/**
 * Returns true if the character is ASCII-7 whitespace.
 */
static bool isAsciiSpace( char ch )
{
	return 0 != isspace(ch);
}

/**
 * Returns true if the character is ASCII-7 digit.
 */
static bool isAsciiDigit( char ch )
{
	return 0 != isdigit(ch);
}

//-----------------------------------------------------------------------------

template <> int NumberReader<double>::put( char ch )
{
	switch ( m_state )
	{
	case STATE_INIT:
		if ( isAsciiSpace(ch) )
			return 1;
		m_state			= STATE_SIGN;
		m_valid			= false; 
		m_sign			= 1;
		m_value			= 0;
		m_expSign		= 1;
		m_expValue		= 0;
	
	case STATE_SIGN:
		if ( char('+') == ch )
		{
			m_sign = 1;
			m_state = STATE_BODY;
			m_valid	= false; 
			return 1;
		}
		else if ( char('-') == ch )
		{
			m_sign = -1;
			m_state = STATE_BODY;
			m_valid	= false; 
			return 1;
		}
		m_state = STATE_BODY;
	
	case STATE_BODY:
		if ( char('.') == ch )
		{
			m_state = STATE_FRACTION;
			m_fractionScale = T(0.1);
			return 1;
		}
		else if ( expChar(ch) )
		{
			m_state = STATE_EXP;
			m_valid	= false; 
			return 1;
		}
		else if ( isAsciiDigit(ch) )
		{
			m_value *= T(10);
			m_value += digitToNumber<T>(ch);
			m_valid = true;
			return 1;
		}
		break;

	case STATE_FRACTION:
		if ( isAsciiDigit(ch) )
		{
			m_value += digitToNumber<T>(ch) * m_fractionScale;
			m_fractionScale *= T(0.1);
			m_valid = true;
			return 1;
		}
		else if ( expChar(ch) )
		{
			m_state = STATE_EXP;
			m_valid	= false; 
			return 1;
		}
		break;

	case STATE_EXP:
		if ( char('+') == ch )
		{
			m_expSign = 1;
			m_state = STATE_EXP_BODY;
			m_valid	= false; 
			return 1;
		}
		else if ( char('-') == ch )
		{
			m_expSign = -1;
			m_state = STATE_EXP_BODY;
			m_valid	= false; 
			return 1;
		}
		m_state = STATE_EXP_BODY;
	
	case STATE_EXP_BODY:
		if ( char('.') == ch )
		{
			m_state = STATE_EXP_FRACTION;
			m_fractionScale = T(0.1);
			return 1;
		}
		else if ( isAsciiDigit(ch) )
		{
			m_expValue *= T(10);
			m_expValue += digitToNumber<T>(ch);
			m_valid	= true; 
			return 1;
		}
		break;

	case STATE_EXP_FRACTION:
		if ( isAsciiDigit(ch) )
		{
			m_expValue += digitToNumber<T>(ch) * m_fractionScale;
			m_fractionScale *= T(0.1);
			m_valid	= true; 
			return 1;
		}
		break;
	}

	if ( !isAsciiSpace(ch) )
		m_valid = false;
	m_state = STATE_INIT;
	return 0;
}

template <> double NumberReader<double>::value() const
{
	return m_sign * m_value * (T)pow( 10.0, m_expSign*m_expValue );
}

template <> int NumberReader<long>::put( char ch )
{
	switch ( m_state )
	{
	case STATE_INIT:
		if ( isAsciiSpace(ch) )
			return 1;
		m_state			= STATE_SIGN;
		m_valid			= false; 
		m_sign			= 1;
		m_value			= 0;
	
	case STATE_SIGN:
		if ( char('+') == ch )
		{
			m_sign = 1;
			m_state = STATE_BODY;
			m_valid	= false; 
			return 1;
		}
		else if ( char('-') == ch )
		{
			m_sign = -1;
			m_state = STATE_BODY;
			m_valid	= false; 
			return 1;
		}
		m_state = STATE_BODY;
	
	case STATE_BODY:
		if ( isAsciiDigit(ch) )
		{
			m_value *= T(10);
			m_value += digitToNumber<T>(ch);
			m_valid = true;
			return 1;
		}
		break;
	}

	if ( !isAsciiSpace(ch) )
		m_valid = false;
	m_state = STATE_INIT;
	return 0;
}

template <> long NumberReader<long>::value() const
{
	return m_sign * m_value;
}

template <> int NumberReader<unsigned long>::put( char ch )
{
	switch ( m_state )
	{
	case STATE_INIT:
		if ( isAsciiSpace(ch) )
			return 1;
		m_state			= STATE_BODY;
		m_valid			= false; 
		m_value			= 0;
	
	case STATE_BODY:
		if ( isAsciiDigit(ch) )
		{
			m_value *= T(10);
			m_value += digitToNumber<T>(ch);
			m_valid = true;
			return 1;
		}
		break;
	}

	if ( !isAsciiSpace(ch) )
		m_valid = false;
	m_state = STATE_INIT;
	return 0;
}

template <> unsigned long NumberReader<unsigned long>::value() const
{
	return m_value;
}


} // lang

/*
#include "NumberReader.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>

int main( int argc, char* argv[] )
{
	// 1.23 -1.23 1. 1.e+002 .2e-10 123 3e3 -123
	if ( argc > 1 )
	{
		int k;

		printf( "NumberReader<double>:\n" );
		for ( k = 1 ; k < argc ; ++k )
		{
			NumberReader<float> rd;
			const char* str = argv[k];
			for ( int i = 0 ; str[i] && rd.put( str[i] ) ; ++i );
			if ( rd.valid() )
				printf( "value %s = %g\n", str, (double)rd.value() );
			else
				printf( "invalid double value (%s)\n", str );
		}

		printf( "NumberReader<int>:\n" );
		for ( k = 1 ; k < argc ; ++k )
		{
			NumberReader<int> rd;
			const char* str = argv[k];
			for ( int i = 0 ; str[i] && rd.put( str[i] ) ; ++i );
			if ( rd.valid() )
				printf( "value %s = %i\n", str, rd.value() );
			else
				printf( "invalid int value (%s)\n", str );
		}

		printf( "NumberReader<unsigned>:\n" );
		for ( k = 1 ; k < argc ; ++k )
		{
			NumberReader<unsigned> rd;
			const char* str = argv[k];
			for ( int i = 0 ; str[i] && rd.put( str[i] ) ; ++i );
			if ( rd.valid() )
				printf( "value %s = %u\n", str, rd.value() );
			else
				printf( "invalid unsigned int value (%s)\n", str );
		}
	}
	getchar();
	return 0;
}
*/
