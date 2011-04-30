#include "String.h"
#include "Mutex.h"
#include "UTFConverter.h"
#include <mem/raw.h>
#include <dev/Profile.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace lang
{


/**
 * Representation of the string object.
 */
struct StringRep
{
	long	refs;		/// Reference count
	int		size;		/// Number of UTF-16 code units in the string.
	int		capacity;	/// Maximum number of UTF-16 code units in the string.
	bool	autoalloc;	/// Set to true if allocated to stack
	// Char [capacity]
};

//-----------------------------------------------------------------------------

// NOTE: All arrayXxx functions operate with plain (character) arrays but 
// xxxString routines operate with string objects creates by createString.

/** 
 * Returns number of elements in the 0-terminated sequence.
 */
template <class T> static int arrayLength( const T* str )
{
	const T* str0 = str;
	while ( *str )
		++str;
	int len = str - str0;
	return len;
}

/** 
 * Copies specified number of elements from source Char array to the destination Char array.
 * Terminates destination Char array with 0.
 */
static void arrayCopy( Char* dest, const Char* src, int count )
{
	memcpy( dest, src, unsigned(count)*sizeof(Char) );
	dest[count] = 0;
}

/** 
 * Copies specified number of elements from source char array to the destination Char array.
 * Terminates destination Char array with 0.
 */
static void arrayCopy( Char* dest, const char* src, int count )
{
	while ( 0 != count )
	{
		*dest++ = (unsigned char)*src++;
		--count;
	}
	*dest = 0;
}

/**
 * Copies single Unicode code point to Char array.
 * @return Char array ptr after appended code point.
 */
inline static Char* arrayCopy( Char* str, Char32 cp )
{
	if ( cp >= 0x10000 )
	{
		*str++ = Char( ((cp-0x10000)>>10) + 0xD800 );
		*str++ = Char( ((cp-0x10000)&1023) + 0xDC00 );
	}
	else
	{
		*str++ = Char( cp );
	}
	return str;
}

/**
 * Gets next Unicode code point from the Char array.
 * @return Char array ptr after retrieved code point.
 */
inline static const Char* arrayNext( const Char* str, Char32* cp )
{
	Char32 c = *str++;

	// high-surrogate?
	if ( c >= 0xD800 && c <= 0xDBFF ) 
	{
		// supplementary code point, <H,L> pair
		Char32 c2 = *str++;
		c = ( Char32(c-0xD800) << 10) + ( Char32(c2-0xDC00) + 0x10000);
	}

	*cp = c;
	return str;
}

/**
 * Compare n code units from two Char arrays.
 * @return strcmp style result
 */
static int arrayCompare( const Char* s1, const Char* s2, int len )
{
	for ( ; 0 != len ; --len )
	{
		Char32 cp1, cp2;
		s1 = arrayNext( s1, &cp1 );
		s2 = arrayNext( s2, &cp2 );
		if ( cp1 != cp2 ) 
			return ( cp1 < cp2 ? -1 : 1 );
	}
	return 0;
}

/**
 * Creates a string object to heap. 
 * Reference count of created object is 1.
 * Initial size of the string is 0.
 * String object must be freed with releaseString.
 *
 * @param capacity Maximum number of code units in the string.
 */
static Char* createDynamicString( int capacity )
{
	capacity = (capacity+31)&~31;
	if ( capacity < 32 )
		capacity = 32;
	
	int bytes = sizeof(StringRep) + sizeof(Char)*unsigned(capacity);
	void* p = mem_alloc( bytes );
	memset( p, 0, bytes );

	StringRep* s = reinterpret_cast<StringRep*>(p);
	s->refs = 1;
	s->size = 0;
	s->capacity = capacity;
	s->autoalloc = false;
	Char* cs = reinterpret_cast<Char*>(s+1);
	*cs = 0;
	return cs;
}

/**
 * Creates a string object to static buffer if possible or to heap otherwise. 
 * Reference count of created object is 1.
 * Initial size of the string is 0.
 * String object must be freed with releaseString.
 */
static Char* createString( int maxLen, void* buffer, int bufferSize )
{
	assert( bufferSize > sizeof(StringRep)+sizeof(Char)*4 ); // Minimum size of string buffer

	unsigned freeBytes = bufferSize - sizeof(StringRep);
	int maxCapacity = freeBytes/sizeof(Char) - 1;
	assert( maxCapacity > 0 );
	
	if ( maxCapacity > maxLen )
	{
		StringRep* s = reinterpret_cast<StringRep*>( buffer );
		s->refs = 1;
		s->size = 0;
		s->capacity = maxCapacity;
		s->autoalloc = true;
		Char* cs = reinterpret_cast<Char*>(s+1);
		*cs = 0;
		return cs;
	}
	else
	{
		return createDynamicString( maxLen+1 );
	}
}

#ifdef _DEBUG
/**
 * Returns string object capacity variable.
 * String object must be created with createString.
 */
static int getStringCapacity( Char* str )
{
	StringRep* s = reinterpret_cast<StringRep*>(str)-1;
	return s->capacity;
}	
#endif // assert

/**
 * Returns number of referencews to the string object.
 */
/*static int getStringReferences( Char* str )
{
	StringRep* s = reinterpret_cast<StringRep*>(str)-1;
	return s->refs;
}*/

/**
 * Returns true if string object memory is allocated from heap.
 * String object must be created with createString.
 */
inline static bool isStringDynamic( Char* str )
{
	StringRep* s = reinterpret_cast<StringRep*>(str)-1;
	return !s->autoalloc;
}

/**
 * Sets string object size variable.
 * String object must be created with createString.
 */
static void setStringSize( Char* str, int size )
{
	assert( getStringCapacity(str) > size );
	str[size] = 0;
	StringRep* s = reinterpret_cast<StringRep*>(str)-1;
	s->size = size;
}

/**
 * Returns string object size variable.
 * String object must be created with createString.
 */
inline static int getStringSize( Char* str )
{
	StringRep* s = reinterpret_cast<StringRep*>(str)-1;
	return s->size;
}

/**
 * Releases string object. Frees memory if the string reference count is 0.
 * String object must be created with createString.
 */
static void releaseString( Char* str )
{
	StringRep* s = reinterpret_cast<StringRep*>(str)-1;
	if ( 0 == Mutex::decrementRC(&s->refs) )
	{
		if ( !s->autoalloc )
			mem_free( s );
	}
}

/**
 * Adds reference to string object.
 * String object must be created with createString.
 */
inline static void addStringReference( Char* str )
{
	assert( isStringDynamic(str) ); // we can't reference automatically allocated strings

	StringRep* s = reinterpret_cast<StringRep*>(str)-1;
	Mutex::incrementRC( &s->refs );
}

//-----------------------------------------------------------------------------

String::String()
{
	m_this = createString( 0, m_buffer, sizeof(m_buffer) );
}

String::String( const Char* str ) 
{
	int size = arrayLength(str);
	m_this = createString( size, m_buffer, sizeof(m_buffer) );
	arrayCopy( m_this, str, size );
	setStringSize( m_this, size );
}

String::String( const Char* begin, int count ) 
{
	m_this = createString( count, m_buffer, sizeof(m_buffer) );
	arrayCopy( m_this, begin, count );
	setStringSize( m_this, count );
}

String::String( const char* str ) 
{
	int size = arrayLength(str);
	m_this = createString( size, m_buffer, sizeof(m_buffer) );
	arrayCopy( m_this, str, size );
	setStringSize( m_this, size );
}

String::String( const char* begin, int count ) 
{
	m_this = createString( count, m_buffer, sizeof(m_buffer) );
	arrayCopy( m_this, begin, count );
	setStringSize( m_this, count );
}

String::String( const void* data, int size, const char* encoding ) 
{
	UTFConverter	cnv			( encoding );
	const uint8_t*	dataBytes0	= reinterpret_cast<const uint8_t*>( data );
	const uint8_t*	dataBytes	= dataBytes0;
	const uint8_t*	dataEnd		= reinterpret_cast<const uint8_t*>( data ) + size;
	Char*			str			= createString( size, m_buffer, sizeof(m_buffer) );

	m_this = str;
	while ( dataBytes < dataEnd )
	{
		Char32 cp;
		int srcBytes;
		if ( cnv.decode(dataBytes, dataEnd, &srcBytes, &cp) )
		{
			if ( Char32(0) == cp )
				break;
			str = arrayCopy( str, cp );
		}
		dataBytes += srcBytes;
	}
	setStringSize( m_this, str-m_this );
}

String::String( Char ch )
{
	m_this = createString( 1, m_buffer, sizeof(m_buffer) );
	arrayCopy( m_this, &ch, 1 );
	setStringSize( m_this, 1 );
}

String::String( const String& other ) 
{
	if ( isStringDynamic(other.m_this) )
	{
		addStringReference( other.m_this );
		m_this = other.m_this;
	}
	else
	{
		m_this = createString( 0, m_buffer, sizeof(m_buffer) );
		int otherLen = other.length();
		arrayCopy( m_this, other.m_this, otherLen );
		setStringSize( m_this, otherLen );
	}
}

String::~String() 
{
	releaseString( m_this );
}

String& String::operator=( const String& other ) 
{
	if ( m_this != other.m_this )
	{
		releaseString( m_this );
		if ( isStringDynamic(other.m_this) )
		{
			addStringReference( other.m_this );
			m_this = other.m_this;
		}
		else
		{
			m_this = createString( 0, m_buffer, sizeof(m_buffer) );
			int otherLen = other.length();
			arrayCopy( m_this, other.m_this, otherLen );
			setStringSize( m_this, otherLen );
		}
	}
	return *this;
}

int String::length() const 
{
	return getStringSize( m_this );
}

Char String::charAt( int index ) const
{
	assert( index >= 0 && index < length() );
	return m_this[index];
}

int String::getBytes( void* buffer, int bufferSize, const char* encoding ) const
{
	UTFConverter	cnv				( encoding );
	uint8_t*		byteBuffer		= reinterpret_cast<uint8_t*>(buffer);
	int				bytesNeeded		= 0;
	const Char*		src				= m_this;
	const Char*		srcEnd			= m_this + getStringSize(m_this);

	while ( src < srcEnd )
	{
		Char32 cp;
		src = arrayNext( src, &cp );

		int tmpBytes = 0;
		char tmp[16];
		if ( cnv.encode(tmp, tmp+sizeof(tmp), &tmpBytes, cp) )
		{
			if ( bytesNeeded+tmpBytes <= bufferSize )
				memcpy( byteBuffer+bytesNeeded, tmp, tmpBytes );

			bytesNeeded += tmpBytes;
		}
	}

	// *ALWAYS* terminate with zero
	if ( bytesNeeded < bufferSize )
		byteBuffer[bytesNeeded] = 0;
	else if ( bufferSize > 0 )
		byteBuffer[bufferSize-1] = 0;

	return bytesNeeded;
}

void String::getChars( int begin, int end, Char* dest ) const
{
	assert( begin >= 0 && begin <= length() );
	assert( begin >= 0 && begin <= end );
	assert( end <= length() );

	for ( ; begin < end ; ++begin )
		*dest++ = m_this[begin];
}

bool String::endsWith( const String& suffix ) const
{
	int		thisLen		= length();
	int		otherLen	= suffix.length();

	if ( otherLen <= thisLen )
		return 0 == arrayCompare( m_this+thisLen-otherLen, suffix.m_this, otherLen );
	else
		return false;
}

bool String::startsWith( const String& prefix ) const
{
	int		thisLen		= length();
	int		otherLen	= prefix.length();

	if ( otherLen <= thisLen )
		return 0 == arrayCompare( m_this, prefix.m_this, otherLen );
	else
		return false;
}

int String::hashCode() const
{
	int code = 0;
	int thisLen = length();

	for ( int i = 0 ; i < thisLen ; ++i )
	{
		code *= 31;
		code += m_this[i];
	}
	return code;
}

int String::indexOf( Char ch, int index ) const
{
	assert( index >= 0 && index < length() );

	int thisLen = length();

	for ( ; index < thisLen ; ++index )
	{
		if ( m_this[index] == ch )
			return index;
	}
	return -1;
}

int String::indexOf( const String& str, int index ) const
{
	assert( index >= 0 );

	int		thisLen		= length();
	int		strLen		= str.length();
	int		lastIndex	= thisLen - strLen;

	for ( ; index <= lastIndex ; ++index )
	{
		int j = 0;
		for ( ; j < strLen ; ++j )
		{
			if ( str.m_this[j] != m_this[index+j] )
				break;
		}
		if ( strLen == j )
			return index;
	}
	return -1;
}

int String::lastIndexOf( Char ch ) const										
{
	int len = length();
	if ( 0 == len )
		return -1;
	else
		return lastIndexOf( ch, len-1 );
}

int String::lastIndexOf( Char ch, int index ) const
{
	assert( index >= 0 && index < length() );

	for ( ; index >= 0 ; --index )
	{
		if ( m_this[index] == ch )
			return index;
	}
	return -1;
}

int String::lastIndexOf( const String& str, int index ) const
{
	assert( index >= 0 && index < length() );

	int		thisLen		= length();
	int		strLen		= str.length();

	if ( index+strLen > thisLen )
		index = thisLen - strLen;

	for ( ; index >= 0 ; --index )
	{
		int j = 0;
		for ( ; j < strLen ; ++j )
		{
			if ( str.m_this[j] != m_this[index+j] )
				break;
		}
		if ( strLen == j )
			return index;
	}
	return -1;
}

bool String::regionMatches( int thisOffset, const String& other, int otherOffset, int length ) const
{
	int		thisLen		= this->length();
	int		otherLen	= other.length();

	if ( thisOffset >= 0 && 
		otherOffset >= 0 &&
		thisOffset+length <= thisLen &&
		otherOffset+length <= otherLen )
	{
		for ( int c = 0 ; c < length ; ++c )
		{
			if ( m_this[thisOffset+c] != other.m_this[otherOffset+c] )
				return false;
		}
		return true;
	}
	return false;
}

String String::replace( Char oldChar, Char newChar ) const
{
	int thisLen = length();
	String s;
	s.m_this = createString( thisLen, s.m_buffer, sizeof(s.m_buffer) );
	setStringSize( s.m_this, thisLen );
	
	for ( int i = 0 ; i < thisLen ; ++i )
	{
		Char c = m_this[i];
		if ( c == oldChar )
			c = newChar;
		s.m_this[i] = c;
	}

	return s;
}

String String::substring( int begin, int end ) const
{
	assert( begin >= 0 && end <= length() && end >= begin );

	String s;
	s.m_this = createString( end-begin, s.m_buffer, sizeof(s.m_buffer) );

	unsigned len = end - begin;
	memcpy( s.m_this, m_this+begin, len*sizeof(Char) );
	
	setStringSize( s.m_this, len );
	
	return s;
}

String String::substring( int begin ) const
{
	assert( begin >= 0 && begin <= length() );
	return substring( begin, length() );
}

String String::toLowerCase() const
{
	int thisLen = length();
	String s;
	s.m_this = createString( thisLen, s.m_buffer, sizeof(s.m_buffer) );
	setStringSize( s.m_this, thisLen );
	
	for ( int i = 0 ; i < thisLen ; ++i )
	{
		Char ch = m_this[i];
		if ( ch < Char(0x80) )
			ch = (Char)tolower( (char)ch );
		s.m_this[i] = ch;
	}

	return s;
}

String String::toUpperCase() const
{
	int thisLen = length();
	String s;
	s.m_this = createString( thisLen, s.m_buffer, sizeof(s.m_buffer) );
	setStringSize( s.m_this, thisLen );
	
	for ( int i = 0 ; i < thisLen ; ++i )
	{
		Char ch = m_this[i];
		if ( ch < Char(0x80) )
			ch = (Char)toupper( (char)ch );
		s.m_this[i] = ch;
	}

	return s;
}

String String::trim() const
{
	int thisLen = length();
	int begin	= 0;
	int end		= thisLen;

	for ( ; begin < thisLen ; ++begin )
	{
		Char ch = m_this[begin];
		if ( ch >= Char(0x80) ||
			!isspace( (char)ch ) )
			break;
	}
	for ( ; end > 0 ; --end )
	{
		Char ch = m_this[end-1];
		if ( ch >= Char(0x80) ||
			!isspace( (char)ch ) )
			break;
	}
	return substring( begin, end );
}

int String::compareTo( const String& other ) const 
{
	//dev::Profile pr( "String.compareTo" );
/*
	// code unit compare
	const Char* s1 = m_this;
	const Char* s2 = other.m_this;
	int i;
	for ( i = 0 ; s1[i] && s2[i] ; ++i )
		if ( s1[i] != s2[i] )
			break;

	int dif = s1[i] - s2[i];
	if ( 0 == dif )
		return 0;
	else
		return dif;
*/
	// code point compare
	int		thisLen		= length();
	int		otherLen	= other.length();
	int		len			= ( thisLen <= otherLen ? thisLen : otherLen );

	int	ans	= arrayCompare( m_this, other.m_this, len );
	if ( 0 != ans )
		return ans;
	else if ( thisLen < otherLen )
		return -1;
	else if ( thisLen == otherLen )
		return 0;
	else // thisLen > Other.otherLen
		return 1;
}

String String::operator+( const String& other ) const 
{
	int		thisLen		= length();
	int		otherLen	= other.length();
	int		sumLen		= thisLen+otherLen;

	String s;
	s.m_this = createString( sumLen, s.m_buffer, sizeof(s.m_buffer) );
	arrayCopy( s.m_this, m_this, thisLen );
	arrayCopy( s.m_this+thisLen, other.m_this, otherLen );
	setStringSize( s.m_this, sumLen );

	return s;
}

String String::valueOf( int value )
{
	char buff[32];
	sprintf( buff, "%i", value );
	String str = buff;
	return str;
}

String String::valueOf( float value )
{
	char buff[32];
	sprintf( buff, "%g", value );
	String str = buff;
	return str;
}


} // lang


lang::String operator+( const char* first, const lang::String& second )						
{
	return lang::String(first) + second;
}
