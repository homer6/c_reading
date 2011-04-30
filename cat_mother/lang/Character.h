#ifndef _LANG_CHARACTER_H
#define _LANG_CHARACTER_H


#include <lang/Char.h>


namespace lang
{


/** 
 * Immutable Char wrapper and type information.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Character
{
public:
	///
	Character()																		{m_v=0;}

	/// 
	Character( Char v )																{m_v=v;}

	/** Returns true if the values are equal. */
	bool			operator==( const Character& other ) const						{return m_v == other.m_v;}

	/** Returns true if the values are not equal. */
	bool			operator!=( const Character& other ) const						{return m_v != other.m_v;}

	/** Returns the value of this object. */
	Char			charValue() const												{return m_v;}

	/** Returns hash code of this object. */
	int				hashCode() const												{return m_v;}

	/** Returns true if the code point is a Unicode digit. */
	static bool		isDigit( Char32 cp );

	/** Returns true if the code point is a Unicode letter. */
	static bool		isLetter( Char32 cp );

	/** Returns true if the code point is a Unicode letter or digit. */
	static bool		isLetterOrDigit( Char32 cp );

	/** Returns true if the code point is a Unicode lower-case. */
	static bool		isLowerCase( Char32 cp );

	/** Returns true if the code point is a Unicode upper-case. */
	static bool		isUpperCase( Char32 cp );

	/** Returns true if the code point is a Unicode whitespace. */
	static bool		isWhitespace( Char32 cp );

private:
	Char m_v;
};


} // lang


#endif // _LANG_CHARACTER_H
