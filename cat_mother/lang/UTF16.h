#ifndef _LANG_UTF16_H
#define _LANG_UTF16_H


#include <lang/Char.h>


namespace lang
{


/**
 * Low-level UTF-16 encoding functionality.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class UTF16
{
public:
	/** Returns true if the character is first surrogate. */
	static bool				isFirstSurrogate( lang::Char ch )						{return ( lang::Char32(ch) >= 0xD800 && lang::Char32(ch) <= 0xDBFF );}

	/** Returns true if the character is second surrogate. */
	static bool				isSecondSurrogate( lang::Char ch )						{return ( lang::Char32(ch) >= 0xDC00 && lang::Char32(ch) <= 0xDFFF );}

	/** Returns code point from surrogate pair. */
	static lang::Char32		makeCodePoint( lang::Char ch1, lang::Char ch2 )			{return ( ((lang::Char32(ch1)-0xD800) << 10) + (lang::Char32(ch2)-0xDC00) + 0x10000 );}

	/** 
	 * Appends code point to character sequence. 
	 *
	 * @return Pointer to sequence after added character(s).
	 */
	static lang::Char*		append( lang::Char* cz, lang::Char32 cp )				{if ( cp >= 0x10000 ) {*cz++ = lang::Char( ((cp-0x10000)>>10) + 0xD800 ); *cz++ = lang::Char( ((cp-0x10000)&1023) + 0xDC00 );} else {*cz++ = lang::Char( cp );} return cz;}
};


} // lang


#endif // _LANG_UTF16_H
