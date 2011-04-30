#ifndef _LANG_FLOAT_H
#define _LANG_FLOAT_H


#include <lang/Number.h>


namespace lang
{


class String;


/** 
 * Immutable float wrapper and type information.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Float :
	public Number
{
public:
	/** Maximum size of a float value. */
	static float	MAX_VALUE;

	/** Minimum size of a float value. */
	static float	MIN_VALUE;

	/** A value representing positive infinity. */
	static float	POSITIVE_INFINITY;

	/** A value representing negative infinity. */
	static float	NEGATIVE_INFINITY;

	/** Not a number. */
	static float	NaN;

	///
	Float()																			{m_v=0;}

	/// 
	Float( float v )																{m_v=v;}

	/** Returns true if the values are equal. */
	bool			operator==( const Float& other ) const							{return m_v == other.m_v;}

	/** Returns true if the values are not equal. */
	bool			operator!=( const Float& other ) const							{return m_v != other.m_v;}

	/** Returns true if this value is less than the other. */
	bool			operator<( const Float& other ) const							{return m_v < other.m_v;}

	/** Returns true if this value is less or equal as the other. */
	bool			operator<=( const Float& other ) const							{return m_v <= other.m_v;}

	/** Returns true if this value is greater than the other. */
	bool			operator>( const Float& other ) const							{return m_v > other.m_v;}

	/** Returns true if this value is greater or equal as the other. */
	bool			operator>=( const Float& other ) const							{return m_v >= other.m_v;}

	/** Returns the value of this object. */
	float			floatValue() const												{return m_v;}

	/** Returns hash code of this object. */
	int				hashCode() const												{return *reinterpret_cast<const int*>(&m_v);}

	/** 
	 * Parses a float value from the string.
	 * @exception NumberFormatException
	 */
	static float	parseFloat( const lang::String& str );

private:
	float m_v;
};


} // lang


#endif // _LANG_FLOAT_H
