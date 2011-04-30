#ifndef _LANG_NUMBERPARSE_H
#define _LANG_NUMBERPARSE_H


#include "Character.h"
#include "NumberReader.h"
#include "NumberFormatException.h"
#include "config_inl.h"


namespace lang
{


/** 
 * Utility class for parsing numeric values of type T. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class T> class NumberParse
{
public:
	/** 
	 * Parses numeric value (English/US locale only) from string. 
	 * Dependencies: NumberReader, Character, NumberFormatException
	 * @exception NumberFormatException
	 */
	static T parse( const String& str )
	{
		// get trailing whitespace
		int end = str.length();
		if ( end > 0 && Character::isWhitespace( str.charAt(end-1) ) )
			--end;

		// parse number
		NumberReader<T> nr;
		int i = 0;
		for ( ; i < end ; ++i )
		{
			Char ch = str.charAt(i);
			if ( ch >= 0x80 )
				break;
			if ( 0 == nr.put( (char)ch ) )
				break;
		}

		// check results
		if ( !nr.valid() || i < end )
		{
			NumberFormatException e( Format("Number cannot be parsed from string: {0}",str) );
			throw e;
		}
		return nr.value();
	}
};


} // lang


#endif // _LANG_NUMBERPARSE_H
