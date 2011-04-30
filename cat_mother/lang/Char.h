#ifndef _LANG_CHAR_H
#define _LANG_CHAR_H


#ifdef _MSC_VER

	namespace lang
	{
		typedef unsigned __int16	Char;
		typedef unsigned __int32	Char32;
	}

#else

	#include <stdint.h>

	namespace lang
	{
		/** Unicode character. */
		typedef uint16_t	Char;

		/** Unicode code point. */
		typedef uint32_t	Char32;
	}

#endif // _MSC_VER


#endif // _LANG_CHAR_H
