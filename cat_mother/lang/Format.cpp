#include "Format.h"
#include <lang/Character.h>
#include <assert.h>
#include "config.h"


namespace lang
{


Format::Format()
{
	m_args = 0;
}

Format::Format( const String& pattern )
{
	m_pattern = pattern;
	m_args = 0;
}

Format::Format( const String& pattern, const Formattable& arg0 )
{
	m_pattern = pattern;
	m_args	= 1;
	m_argv[0] = arg0;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1 )
{
	m_pattern = pattern;
	m_args	= 2;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2 )
{
	m_pattern = pattern;
	m_args	= 3;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3 )
{
	m_pattern = pattern;
	m_args	= 4;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
	m_argv[3] = arg3;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4 )
{
	m_pattern = pattern;
	m_args	= 5;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
	m_argv[3] = arg3;
	m_argv[4] = arg4;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5 )
{
	m_pattern = pattern;
	m_args	= 6;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
	m_argv[3] = arg3;
	m_argv[4] = arg4;
	m_argv[5] = arg5;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6 )
{
	m_pattern = pattern;
	m_args	= 7;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
	m_argv[3] = arg3;
	m_argv[4] = arg4;
	m_argv[5] = arg5;
	m_argv[6] = arg6;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7 )
{
	m_pattern = pattern;
	m_args	= 8;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
	m_argv[3] = arg3;
	m_argv[4] = arg4;
	m_argv[5] = arg5;
	m_argv[6] = arg6;
	m_argv[7] = arg7;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8 )
{
	m_pattern = pattern;
	m_args	= 9;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
	m_argv[3] = arg3;
	m_argv[4] = arg4;
	m_argv[5] = arg5;
	m_argv[6] = arg6;
	m_argv[7] = arg7;
	m_argv[8] = arg8;
}

Format::Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8, const Formattable& arg9 )
{
	m_pattern = pattern;
	m_args	= 10;
	m_argv[0] = arg0;
	m_argv[1] = arg1;
	m_argv[2] = arg2;
	m_argv[3] = arg3;
	m_argv[4] = arg4;
	m_argv[5] = arg5;
	m_argv[6] = arg6;
	m_argv[7] = arg7;
	m_argv[8] = arg8;
	m_argv[9] = arg9;
}

Format::Format( const String& pattern, int argc, Formattable* argv )
{
	assert( argc <= 10 );

	if ( argc < 10 )
		argc = 10;

	m_pattern = pattern;
	m_args	= argc;
	for ( int i = 0 ; i < argc ; ++i )
		m_argv[i] = argv[i];
}

const String& Format::pattern() const
{
	return m_pattern;
}

int Format::arguments() const
{
	return m_args;
}

const Formattable& Format::getArgument( int i ) const
{
	assert( i >= 0 && i < arguments() );
	return m_argv[i];
}

int Format::format( Char* buffer, int size ) const
{
	int		d		= 0;
	int		s		= 0;
	bool	skip	= false;

	while ( d < size && s < (int)m_pattern.length() )
	{
		Char ch = m_pattern.charAt( s++ );

		if ( ch == '{' && !skip && s < m_pattern.length() )
		{
			// <index><,opt>}
			int end = m_pattern.indexOf( '}', s );
			if ( -1 != end && end-s > 0 )
			{
				Char digit = m_pattern.charAt( s++ );
				if ( Character::isDigit(digit) )
				{
					// <,opt>}
					int arg = digit - '0';
					assert( arg >= 0 && arg < m_args );
					if ( arg >= 0 && arg < m_args )
					{
						int left = size - d;
						if ( left < 0 )
							left = 0;
						if ( s < m_pattern.length() && m_pattern.charAt(s) == ',' )
							++s;
						
						assert( arg >= 0 && arg < m_args );
						d += m_argv[arg].format( buffer+d, left, m_pattern, s );
						s = end+1;
					}
					else
					{
						break;
					}
				}
			}
		}
		else if ( ch == '\\' && !skip )
		{
			skip = true;
		}
		else
		{
			if ( d < size )
				buffer[d] = ch;
			++d;
			skip = false;
		}
	}

	int end = d;
	if ( end >= size )
		end = size-1;
	if ( end >= 0 )
		buffer[end] = 0;
	return d+1;
}

String Format::format() const
{
	Char buf[256];
	int len = format( buf, 256 );
	String str;
	if ( len > 256 )
	{
		Char* buf = new Char[len+2];
		format( buf, len+1 );
		str = buf;
		delete[] buf;
	}
	else
	{
		str = buf;
	}
	return str;
}


} // lang
