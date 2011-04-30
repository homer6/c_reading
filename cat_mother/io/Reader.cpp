#include "Reader.h"
#include <lang/Char.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace io
{


Reader::Reader()
{
}

Reader::~Reader()
{
}

long Reader::skip( long count )
{
	long c = 0;
	lang::Char ch;
	while ( c < count && 1 == read(&ch,1) )
	{
		++c;
	}
	return c;
}

void Reader::mark( int /*readlimit*/ )
{
}

void Reader::reset()
{
}

bool Reader::markSupported() const
{
	return false;
}


} // io
