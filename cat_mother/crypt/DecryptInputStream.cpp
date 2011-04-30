#include "DecryptInputStream.h"
#include "CryptUtil.h"
#include "config.h"

//-----------------------------------------------------------------------------

namespace crypt
{


DecryptInputStream::DecryptInputStream( io::InputStream* in ) :
	FilterInputStream( in ),
	m_in( in ),
	m_size( 0 )
{
}

DecryptInputStream::~DecryptInputStream()
{
}

long DecryptInputStream::read( void* data, long size )
{
	long c = m_in->read( data, size );
	CryptUtil::decryptBuffer( (uint8_t*)data, (uint8_t*)data, c, m_size );
	m_size += c;
	return c;
}

long DecryptInputStream::skip( long n )
{
	long c = m_in->skip( n );
	m_size += c;
	return c;
}


} // crypt
