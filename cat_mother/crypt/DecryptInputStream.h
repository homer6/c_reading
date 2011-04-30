#ifndef _CRYPT_DECRYPTINPUTSTREAM_H
#define _CRYPT_DECRYPTINPUTSTREAM_H


#include <io/FilterInputStream.h>


namespace crypt
{


/**
 * Simple stream decrypting.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class DecryptInputStream :
	public io::FilterInputStream
{
public:
	DecryptInputStream( io::InputStream* in );
	~DecryptInputStream();

	/**
	 * Tries to read specified number of bytes from the source stream.
	 * Doesn't block the caller if specified number of bytes isn't available.
	 *
	 * @return Number of bytes actually read.
	 * @exception IOException
	 */
	long	read( void* data, long size );

	/**
	 * Tries to skip over n bytes from the stream.
	 *
	 * @return Number of bytes actually skipped.
	 * @exception IOException
	 */
	long	skip( long n );

private:
	P(io::InputStream)	m_in;
	long				m_size;
};


} // crypt


#endif // _CRYPT_DECRYPTINPUTSTREAM_H
