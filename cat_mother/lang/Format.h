#ifndef _LANG_FORMAT_H
#define _LANG_FORMAT_H


#include <lang/Formattable.h>


namespace lang
{


/**
 * Class for constructing messages that keep formatting information.
 * (to be used for example in translation)
 *
 * Syntax of format string (=first parameter of every constructor):
 * <ul>
 * <li>{0} = replaced with 1st argument formatted to string.
 * <li>{1} = replaced with 2nd argument formatted to string.
 * <li>{n} = replaced with nth argument formatted to string.
 * <li>{0,x} = replaced with 1st argument formatted to hex number.
 * <li>{0,#.##} = replaced with 1st argument formatted to number using max 2 digits, e.g. 1.2=1.2 but 1.211=1.21.
 * <li>{0,#.00} = replaced with 1st argument formatted to number using exactly 2 digits, e.g. 1.2=1.20 and 1.211=1.21.
 * <li>{0,000} = replaced with 1st argument formatted to number using at least 3 numbers, padded with zeros, e.g. 3=003.
 * </ul>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Format
{
public:
	/** Creates an empty message. */
	Format();

	/** Creates a message with no arguments. */
	Format( const String& pattern );

	/** Creates a message with 1 argument. */
	Format( const String& pattern, const Formattable& arg0 );

	/** Creates a message with 2 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1 );

	/** Creates a message with 3 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2 );

	/** Creates a message with 4 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3 );

	/** Creates a message with 5 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4 );

	/** Creates a message with 6 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5 );

	/** Creates a message with 7 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6 );

	/** Creates a message with 8 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7 );

	/** Creates a message with 9 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8 );

	/** Creates a message with 10 arguments. */
	Format( const String& pattern, const Formattable& arg0, const Formattable& arg1, const Formattable& arg2, const Formattable& arg3, const Formattable& arg4, const Formattable& arg5, const Formattable& arg6, const Formattable& arg7, const Formattable& arg8, const Formattable& arg9 );

	/** Creates a message with n arguments. */
	Format( const String& pattern, int argc, Formattable* argv );

	/** Returns format pattern. */
	const String&		pattern() const;

	/** Returns number of arguments. */
	int					arguments() const;

	/** Returns ith argument. */
	const Formattable&	getArgument( int i ) const;

	/** Returns message formatted using English/US locale. */
	String				format() const;

	/** 
	 * Formats the message to the buffer using English/US locale.
	 * The buffer is always 0-terminated even if the whole message
	 * cannot be stored.
	 * @return Number of characters needed to store the whole message.
	 */
	int					format( Char* buffer, int size ) const;

private:
	String				m_pattern;
	int					m_args;
	Formattable			m_argv[10];
};


} // lang


#endif // _LANG_FORMAT_H
