#ifndef _LANG_INTEGER_H
#define _LANG_INTEGER_H


#include <lang/Number.h>


namespace lang
{


class String;


/** 
 * Immutable int wrapper and type information.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Integer :
	public Number
{
public:
	/** Maximum size of an integer value. */
	static int		MAX_VALUE;

	/** Minimum size of an integer value. */
	static int		MIN_VALUE;

	///
	Integer()																		{m_v=0;}

	/// 
	Integer( int v )																{m_v=v;}

	/** Returns true if the values are equal. */
	bool			operator==( const Integer& other ) const						{return m_v == other.m_v;}

	/** Returns true if the values are not equal. */
	bool			operator!=( const Integer& other ) const						{return m_v != other.m_v;}

	/** Returns true if this value is less than the other. */
	bool			operator<( const Integer& other ) const							{return m_v < other.m_v;}

	/** Returns true if this value is less or equal as the other. */
	bool			operator<=( const Integer& other ) const						{return m_v <= other.m_v;}

	/** Returns true if this value is greater than the other. */
	bool			operator>( const Integer& other ) const							{return m_v > other.m_v;}

	/** Returns true if this value is greater or equal as the other. */
	bool			operator>=( const Integer& other ) const						{return m_v >= other.m_v;}

	/** Returns the value of this object. */
	int				intValue() const												{return m_v;}

	/** Returns hash code of this object. */
	int				hashCode() const												{return m_v;}

	/** 
	 * Parses an integer value from the string.
	 * @exception NumberFormatException
	 */
	static int		parseInt( const lang::String& str );

private:
	int m_v;
};


} // lang


#endif // _LANG_INTEGER_H
