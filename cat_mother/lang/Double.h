#ifndef _LANG_DOUBLE_H
#define _LANG_DOUBLE_H


#include <lang/Number.h>


namespace lang
{


class String;


/** 
 * Immutable double wrapper and type information.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Double :
	public Number
{
public:
	/** Maximum size of a double value. */
	static double	MAX_VALUE;

	/** Minimum size of a double value. */
	static double	MIN_VALUE;

	/** A value representing positive infinity. */
	static double	POSITIVE_INFINITY;

	/** A value representing negative infinity. */
	static double	NEGATIVE_INFINITY;

	/** Not a number. */
	static double	NaN;

	///
	Double()																		{m_v=0;}

	/// 
	Double( double v )																{m_v=v;}

	/** Returns true if the values are equal. */
	bool			operator==( const Double& other ) const							{return m_v == other.m_v;}

	/** Returns true if the values are not equal. */
	bool			operator!=( const Double& other ) const							{return m_v != other.m_v;}

	/** Returns true if this value is less than the other. */
	bool			operator<( const Double& other ) const							{return m_v < other.m_v;}

	/** Returns true if this value is less or equal as the other. */
	bool			operator<=( const Double& other ) const							{return m_v <= other.m_v;}

	/** Returns true if this value is greater than the other. */
	bool			operator>( const Double& other ) const							{return m_v > other.m_v;}

	/** Returns true if this value is greater or equal as the other. */
	bool			operator>=( const Double& other ) const							{return m_v >= other.m_v;}

	/** Returns the value of this object. */
	double			doubleValue() const												{return m_v;}

	/** Returns hash code of this object. */
	int				hashCode() const												{float v=(float)m_v; return *reinterpret_cast<int*>(&v);}

	/** 
	 * Parses a double value from the string.
	 * @exception NumberFormatException
	 */
	static double	parseDouble( const lang::String& str );

private:
	double m_v;
};


} // lang


#endif // _LANG_DOUBLE_H
