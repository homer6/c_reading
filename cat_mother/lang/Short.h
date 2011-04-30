#ifndef _LANG_SHORT_H
#define _LANG_SHORT_H


#include <lang/Number.h>


namespace lang
{


class String;


/** 
 * Immutable short int wrapper and type information.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Short :
	public Number
{
public:
	/** Maximum size of a short integer value. */
	static short	MAX_VALUE;

	/** Minimum size of a short integer value. */
	static short	MIN_VALUE;

	///
	Short()																			{m_v=0;}

	/// 
	Short( short v )																{m_v=v;}

	/** Returns true if the values are equal. */
	bool			operator==( const Short& other ) const							{return m_v == other.m_v;}

	/** Returns true if the values are not equal. */
	bool			operator!=( const Short& other ) const							{return m_v != other.m_v;}

	/** Returns true if this value is less than the other. */
	bool			operator<( const Short& other ) const							{return m_v < other.m_v;}

	/** Returns true if this value is less or equal as the other. */
	bool			operator<=( const Short& other ) const							{return m_v <= other.m_v;}

	/** Returns true if this value is greater than the other. */
	bool			operator>( const Short& other ) const							{return m_v > other.m_v;}

	/** Returns true if this value is greater or equal as the other. */
	bool			operator>=( const Short& other ) const							{return m_v >= other.m_v;}

	/** Returns the value of this object. */
	short			shortValue() const												{return m_v;}

	/** Returns hash code of this object. */
	int				hashCode() const												{return m_v;}

	/** 
	 * Parses a short integer value from the string.
	 * @exception NumberFormatException
	 */
	static short	parseShort( const lang::String& str );

private:
	short m_v;
};


} // lang


#endif // _LANG_SHORT_H
