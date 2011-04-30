#ifndef _LANG_UTFCONVERTER_H
#define _LANG_UTFCONVERTER_H


#include <lang/Char.h>


namespace lang
{


/**
 * UTF-encoder/decoder.
 *
 * Supported encoding schemes:
 *	<pre>
        ASCII-7         UTF-8           UTF-16BE
        UTF-16LE        UTF-32BE        UTF-32LE
	</pre>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class UTFConverter
{
public:
	/** 
	 * Creates an UTF-encoding converter with specified encoding.
	 * See UTFConverter class description for list of supported encoding types.
	 * 
	 * @exception UnsupportedEncodingException
	 */
	explicit UTFConverter( const char* encoding );

	/** 
	 * Decodes bytes to Unicode code points. 
	 * Ignores invalid input byte sequences.
	 *
	 * @param src Ptr to the source data.
	 * @param srcEnd Ptr to the end of source data.
	 * @param srcBytes [out] Receives number of bytes decoded.
	 * @param dst [out] Receives decoded Unicode code point (if byte sequence was not malformed).
	 * @return false if decoded byte sequence was malformed.
	 */
	bool	decode( const void* src, const void* srcEnd, int* srcBytes, Char32* dst ) const;

	/** 
	 * Encodes Unicode code point to bytes. 
	 *
	 * @param dst Destination buffer which receives encoded byte sequence.
	 * @param dstEnd End of the destination buffer.
	 * @param dstBytes [out] Receives number of bytes encoded.
	 * @param src Unicode code point to encode.
	 * @return false if the code point couldn't be encoded.
	 */
	bool	encode( void* dst, void* dstEnd, int* dstBytes, Char32 src );

private:
	int		m_type;
};


} // lang


#endif // _LANG_UTFCONVERTER_H

