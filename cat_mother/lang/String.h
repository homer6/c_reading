#ifndef _LANG_STRING_H
#define _LANG_STRING_H


#include <lang/Char.h>


namespace lang
{


/** 
 * Immutable Unicode character string.
 * Note about implementation:
 * Short strings are stored in preallocated buffer (and copied by value)
 * and longer strings are allocated to heap (and copied by reference).
 * Currently String class has only very basic Unicode support 
 * (for example toUpperCase simply uses toupper internally).
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class String
{
public:
	/** Creates an empty string. */
	String();

	/** 
	 * Creates a string from the null-terminated character sequence. 
	 */
	String( const Char* str );

	/** 
	 * Creates a string from the character sequence. 
	 *
	 * @param begin Pointer to the beginning (inclusive) of the sequence.
	 * @param count Number of characters in the sequence.
	 */
	String( const Char* begin, int count );

	/** 
	 * Creates a string from the null-terminated ASCII-7 char sequence.
	 */
	String( const char* str );

	/** 
	 * Creates a string from the ASCII-7 char sequence.
	 *
	 * @param begin Pointer to the beginning (inclusive) of char sequence.
	 * @param count Number of characters in the sequence.
	 */
	String( const char* begin, int count );

	/** 
	 * Creates a string from the byte sequence with specified encoding. 
	 * Ignores silently encoded characters which have invalid byte sequences.
	 *
	 * Supported encoding types are
	 *	<pre>
        ASCII-7         UTF-8           UTF-16BE
        UTF-16LE        UTF-32BE        UTF-32LE
		</pre>
	 *
	 * @exception UnsupportedEncodingException
	 */
	String( const void* data, int size, const char* encoding );

	/** 
	 * Creates a string from single character.
	 */
	explicit String( Char ch );

	/** Copy by reference. */
	String( const String& other );

	///
	~String();

	/** Copy by reference. */
	String&		operator=( const String& other );

	/** Returns number of characters in the string. */
	int			length() const;

	/** Returns character at specified index. */
	Char		charAt( int index ) const;

	/**
	 * Encodes string content to the buffer using specified encoding method.
	 * Always terminates encoded character string with zero byte.
	 *
	 * @param buffer Pointer to the destination buffer.
	 * @param bufferSize Size of the destination buffer.
	 * @param encoding Encoding type. See constructor String(data,size,encoding) for a list of supported encoding methods.
	 * @return Number of bytes needed to encode the string, not including trailing zero. Note that this might be larger than bufferSize.
	 * @exception UnsupportedEncodingException
	 */
	int			getBytes( void* buffer, int bufferSize, const char* encoding ) const;

	/**
	 * Copies characters from this string into the destination character array.
	 *
	 * @param begin Index to the beginning (inclusive) of the substring.
	 * @param end Index to the end (exclusive) of the substring.
	 */
	void		getChars( int begin, int end, Char* dest ) const;

	/**
	 * Returns true if the string ends with specified substring.
	 */
	bool		endsWith( const String& suffix ) const;

	/**
	 * Returns true if the string starts with specified substring.
	 */
	bool		startsWith( const String& prefix ) const;

	/**
	 * Returns hash code for this string.
	 */
	int			hashCode() const;

	/**
	 * Returns the first index within this string of the specified character.
	 *
	 * @param ch Character to find.
	 * @return Index of the found position or -1 if the character was not found from the string.
	 */
	int			indexOf( Char ch ) const											{return indexOf(ch,0);}

	/**
	 * Returns the first index within this string of the specified character.
	 * Starts the search from the specified position.
	 *
	 * @param ch Character to find.
	 * @param index The first position to search.
	 * @return Index of the found position or -1 if the character was not found from the string.
	 */
	int			indexOf( Char ch, int index ) const;

	/**
	 * Returns the first index within this string of the specified substring.
	 *
	 * @param str Substring to find.
	 * @return Index of the found position or -1 if the substring was not found from the string.
	 */
	int			indexOf( const String& str ) const									{return indexOf(str,0);}

	/**
	 * Returns the first index within this string of the specified substring.
	 * Starts the search from the specified position.
	 *
	 * @param str Substring to find.
	 * @param index The first position to search.
	 * @return Index of the found position or -1 if the substring was not found from the string.
	 */
	int			indexOf( const String& str, int index ) const;

	/**
	 * Returns the last index within this string of the specified character.
	 *
	 * @param ch Character to find.
	 * @return Index of the found position or -1 if the character was not found from the string.
	 */
	int			lastIndexOf( Char ch ) const;

	/**
	 * Returns the last index within this string of the specified character.
	 * Starts the search from the specified position.
	 *
	 * @param ch Character to find.
	 * @param index The last position to search.
	 * @return Index of the found position or -1 if the character was not found from the string.
	 */
	int			lastIndexOf( Char ch, int index ) const;

	/**
	 * Returns the last index within this string of the specified substring.
	 *
	 * @param str Substring to find.
	 * @return Index of the found position or -1 if the substring was not found from the string.
	 */
	int			lastIndexOf( const String& str ) const								{return lastIndexOf(str,length()-1);}

	/**
	 * Returns the last index within this string of the specified substring.
	 * Starts the search from the specified position.
	 *
	 * @param str Substring to find.
	 * @param index The last position to search.
	 * @return Index of the found position or -1 if the substring was not found from the string.
	 */
	int			lastIndexOf( const String& str, int index ) const;

	/**
	 * Tests if two string regions are equal.
	 *
	 * @param thisOffset Offset to this string.
	 * @param other The other string to compare.
	 * @param otherOffset Offset to other string.
	 * @param length Length of the sequences to compare.
	 */
	bool		regionMatches( int thisOffset, const String& other, int otherOffset, int length ) const;

	/**
	 * Returns a new string which has oldChar characters replaced with newChar.
	 */
	String		replace( Char oldChar, Char newChar ) const;

	/**
	 * Returns a new string that is a substring of this string.
	 *
	 * @param begin Index to the beginning (inclusive) of the substring.
	 * @param end Index to the end (exclusive) of the substring.
	 */
	String		substring( int begin, int end ) const;

	/**
	 * Returns a new string that is a substring of this string.
	 *
	 * @param begin Index to the beginning (inclusive) of the substring.
	 */
	String		substring( int begin ) const;

	/**
	 * Returns a new string that has all characters of this string converted to lowercase.
	 * Doesn't handle locale dependent special casing.
	 */
	String		toLowerCase() const;

	/**
	 * Returns a new string that has all characters of this string converted to uppercase.
	 * Doesn't handle locale dependent special casing.
	 */
	String		toUpperCase() const;

	/**
	 * Returns a new string that is otherwise identical to this string 
	 * but has whitespace removed from both ends of the string.
	 */
	String		trim() const;

	/**
	 * Bitwise lecigographical compare between this string and other string. 
	 * @return If this string is lexicographically before other then the return value is <0 and if this string is after other string then >0. If the strings are equal then the return value is 0.
	 */
	int			compareTo( const String& other ) const;

	/** Bitwise equality. */
	bool		operator==( const String& other ) const								{return 0==compareTo(other);}

	/** Bitwise inequality. */
	bool		operator!=( const String& other ) const								{return 0!=compareTo(other);}

	/** Bitwise lexicographical less than. */
	bool		operator<( const String& other ) const								{return compareTo(other)<0;}

	/** Bitwise lexicographical greater than. */
	bool		operator>( const String& other ) const								{return compareTo(other)>0;}

	/** Bitwise lexicographical less or equal. */
	bool		operator<=( const String& other ) const								{return compareTo(other)<=0;}
	
	/** Bitwise lexicographical greater or equal. */
	bool		operator>=( const String& other ) const								{return compareTo(other)>=0;}

	/** Concatenates this string and other string. */
	String		operator+( const String& other ) const;

	/** Returns string representation of specified value. */
	static String	valueOf( int value );

	/** Returns string representation of specified value. */
	static String	valueOf( float value );

private:
	Char*		m_this;
	char		m_buffer[16+60];	// StringRep (16 bytes) + buffer for short strings
};


} // lang


// Shortcut for concatenating 0-terminated ASCII-7 char sequence and String.
lang::String 
	operator+( const char* first, const lang::String& second );


#endif // _LANG_STRING_H
