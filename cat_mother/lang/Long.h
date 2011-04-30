#ifndef _LANG_LONG_H
#define _LANG_LONG_H


#include <lang/Number.h>


namespace lang
{


class String;


/** 
 * Immutable long int wrapper and type information.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Long :
	public Number
{
public:
	/** Maximum size of a long integer value. */
	static long		MAX_VALUE;

	/** Minimum size of a long integer value. */
	static long		MIN_VALUE;

	///
	Long()																			{m_v=0;}

	/// 
	Long( long v )																	{m_v=v;}

	/** Returns true if the values are equal. */
	bool			operator==( const Long& other ) const							{return m_v == other.m_v;}

	/** Returns true if the values are not equal. */
	bool			operator!=( const Long& other ) const							{return m_v != other.m_v;}

	/** Returns true if this value is less than the other. */
	bool			operator<( const Long& other ) const							{return m_v < other.m_v;}

	/** Returns true if this value is less or equal as the other. */
	bool			operator<=( const Long& other ) const							{return m_v <= other.m_v;}

	/** Returns true if this value is greater than the other. */
	bool			operator>( const Long& other ) const							{return m_v > other.m_v;}

	/** Returns true if this value is greater or equal as the other. */
	bool			operator>=( const Long& other ) const							{return m_v >= other.m_v;}

	/** Returns the value of this object. */
	long			longValue() const												{return m_v;}

	/** Returns hash code of this object. */
	int				hashCode() const												{return m_v;}

	/** 
	 * Parses a long integer value from the string.
	 * @exception NumberFormatException
	 */
	static long		parseLong( const lang::String& str );
	
private:
	long m_v;
};


} // lang


#endif // _LANG_LONG_H
