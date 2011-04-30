#include "ChunkInputStream.h"
#include <io/DataInputStream.h>
#include <io/IOException.h>
#include <lang/String.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


static int readVarInt( DataInputStream* in )
{
	int		bits	= 0xFF & in->readByte();
	bool	sign	= 0 != (2 & bits);
	int		value	= bits >> 2;
	int		shift	= 6;
	
	while ( 0 != (1&bits) )
	{
		bits = 0xFF & in->readByte();
		value += (bits >> 1) << shift;
		shift += 7;
	}

	if ( sign )
		value = -value;

	return value;
}

//-----------------------------------------------------------------------------

ChunkInputStream::ChunkInputStream( InputStream* in ) :
	FilterInputStream( in ), m_in( in )
{
}

void ChunkInputStream::beginChunk( String* name, long* end )
{
	*name = m_in.readUTF();
	long datasize = m_in.readInt();
	if ( datasize < 0 )
		throw IOException( Format("Invalid chunk {2} length at {0,#} while reading {1}", m_in.size(), m_in.toString(), *name) );
	long cur = m_in.size();
	*end = cur + datasize;
}

void ChunkInputStream::endChunk( long end )
{
	long cur	= m_in.size();
	long left	= end - cur;

	if ( left < 0 )
		throw IOException( Format("Chunk read overflow at {0,#} while reading {1}", m_in.size(), m_in.toString()) );
	if ( left > 0 )
		m_in.skip( left );
}

int ChunkInputStream::readInt()
{
	return readVarInt( &m_in );
}

float ChunkInputStream::readFloat()
{
	return m_in.readFloat();
}

void ChunkInputStream::readFloatArray( float* array, int count )
{
	assert( array );
	assert( count >= 0 );

	for ( int i = 0 ; i < count ; ++i )
		array[i] = m_in.readFloat();
}

String ChunkInputStream::readString()
{
	return m_in.readUTF();
}

long ChunkInputStream::size() const
{
	return m_in.size();
}

uint8_t ChunkInputStream::readByte()
{
	return m_in.readByte();
}


} // io
