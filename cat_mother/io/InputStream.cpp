#include "InputStream.h"
#include <assert.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace io
{


InputStream::InputStream()
{
}

InputStream::~InputStream()
{
}

void InputStream::close()
{
}

long InputStream::skip( long n )
{
	assert( n >= 0 );

	const long buffSize = 256;
	uint8_t buff[buffSize];

	long bytesSkipped = 0;
	while ( bytesSkipped < n )
	{
		int bytes = n - bytesSkipped;
		if ( bytes > buffSize )
			bytes = buffSize;

		bytes = read( buff, bytes );
		bytesSkipped += bytes;

		if ( 0 == bytes )
			break;
	}

	return bytesSkipped;
}

void InputStream::mark( int /*readlimit*/ )
{
}

void InputStream::reset()
{
}

bool InputStream::markSupported() const
{
	return false;
}


} // io
