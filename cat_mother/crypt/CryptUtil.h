#ifndef _CRYPT_CRYPTUTIL_H
#define _CRYPT_CRYPTUTIL_H


#include <lang/String.h>
#include <stdint.h>


namespace crypt
{


/** 
 * Utilities for simple data crypting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class CryptUtil
{
public:
	/** Returns crypted path name. */
	static lang::String	cryptFileName( const lang::String& inputName );

	/** 
	 * Encrypts buffer contents. 
	 * @param in Input buffer.
	 * @param out [out] Output buffer.
	 * @param c Number of bytes to encrypt.
	 * @param offset File offset of start of input buffer.
	 */
	static void			cryptBuffer( const uint8_t* in, uint8_t* out, long c, long offset );

	/** 
	 * Decrypts buffer contents. 
	 * @param in Input buffer.
	 * @param out [out] Output buffer.
	 * @param c Number of bytes to decrypt.
	 * @param offset File offset of start of input buffer.
	 */
	static void			decryptBuffer( const uint8_t* in, uint8_t* out, long c, long offset );
};


} // crypt


#endif // _CRYPT_CRYPTUTIL_H
