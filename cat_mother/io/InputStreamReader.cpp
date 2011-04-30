#include "InputStreamReader.h"
#include "InputStream.h"
#include <lang/UTF16.h>
#include <lang/UTFConverter.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


class InputStreamReader::InputStreamReaderImpl :
	public Object
{
public:
	InputStreamReaderImpl( InputStream* in, const char* encoding ) :
		m_in( in ), m_cnv( encoding ), m_bufferBegin( 0 ), m_bufferEnd( 0 ),
		m_markBuffer(0), m_markBufferBegin(0), m_markBufferSize(0), m_markBufferCapacity(0), m_markBufferReset(0), m_readlimit(0)
	{
		uint8_t bytes[8];
		memset( bytes, 0xAA, sizeof(bytes) );
		peekBytes( bytes, sizeof(bytes) );

		// byte order mark candidates
		const uint8_t bombytes[][5] = 
		{
			{4, 0,0,0xFE,0xFF},			// UTF-32BE
			{4, 0xFF,0xFE,0,0},			// UTF-32LE
			{3, 0xEF,0xBB,0xBF,0},		// UTF-8
			{2, 0xFE,0xFF,0,0},			// UTF-16BE
			{2, 0xFF,0xFE,0,0},			// UTF-16LE
			{0, 0,0,0,0}				// list terminator
		};
		const char* const bomnames[] =
		{
			"UTF-32BE",
			"UTF-32LE",
			"UTF-8",
			"UTF-16BE",
			"UTF-16LE",
			0 // list terminator
		};

		for ( int i = 0 ; bomnames[i] ; ++i )
		{
			if ( !memcmp(&bombytes[i][1],bytes,bombytes[i][0]) )
			{
				readBytes( bytes, bombytes[i][0] );
				m_cnv = UTFConverter( bomnames[i] );
				break;
			}
		}
	}

	~InputStreamReaderImpl()
	{
		if ( m_markBuffer )
			delete[] m_markBuffer;
	}

	long read( Char* out, long count )
	{
		Char* out0 = out;
		const int bufferSize = 8;
		uint8_t buffer[bufferSize];

		if ( m_markBufferSize-m_markBufferReset < m_readlimit )
		{
			if ( m_markBufferCapacity-m_markBufferReset <= m_readlimit )
				compactMarkBuffer();
			assert( m_markBufferCapacity-m_markBufferReset > m_readlimit );
			
			while ( m_markBufferSize-m_markBufferReset < m_readlimit )
			{
				long bufferBytes = peekBytes( buffer, bufferSize );
				if ( 0 == bufferBytes )
					break;

				int decodedBytes = 0;
				Char32 cp = Char32(0);
				bool ok = m_cnv.decode(buffer, buffer+bufferBytes, &decodedBytes, &cp);
				readBytes( buffer, decodedBytes );
				if ( !ok )
					break;
				m_markBufferSize = UTF16::append( m_markBuffer+m_markBufferSize, cp ) - m_markBuffer;
			}
		}

		while ( m_markBufferBegin < m_markBufferSize && count > 0 )
		{
			Char ch = m_markBuffer[m_markBufferBegin++];
			*out++ = ch;
			--count;
		}

		while ( count > 0 )
		{
			m_readlimit = 0;

			long bufferBytes = peekBytes( buffer, bufferSize );
			if ( 0 == bufferBytes )
				break;

			int decodedBytes = 0;
			Char32 cp = Char32(0);
			bool ok = m_cnv.decode(buffer, buffer+bufferBytes, &decodedBytes, &cp);
			readBytes( buffer, decodedBytes );
			if ( ok )
			{
				out = UTF16::append( out, cp );
				--count;
			}
		}

		return out-out0;
	}

	void close()
	{
		if ( m_in )
			m_in->close();
	}

	void mark( int readlimit )
	{
		assert( m_markBufferBegin <= m_markBufferSize );
		assert( m_markBufferSize <= m_markBufferCapacity );

		if ( readlimit >= m_markBufferCapacity )
		{
			int newCapacity = readlimit;
			if ( newCapacity < 32 )
				newCapacity = 32;
			newCapacity += 2; // for Unicode supplementary planes
			Char* newMarkBuffer = new Char[newCapacity];
			for ( int i = m_markBufferReset ; i < m_markBufferSize ; ++i )
				newMarkBuffer[i-m_markBufferReset] = m_markBuffer[i];
			m_markBufferSize -= m_markBufferReset;
			m_markBufferBegin -= m_markBufferReset;
			m_markBufferReset = 0;
			m_markBufferCapacity = newCapacity;
			delete[] m_markBuffer;
			m_markBuffer = newMarkBuffer;
		}
		assert( readlimit <= m_markBufferCapacity );

		m_markBufferReset = m_markBufferBegin;
		if ( m_markBufferBegin+readlimit >= m_markBufferCapacity )
			compactMarkBuffer();
		assert( m_markBufferBegin+readlimit < m_markBufferCapacity );

		m_readlimit = readlimit;
	}

	void reset()
	{
		m_markBufferBegin = m_markBufferReset;
	}

	bool ready() const
	{
		return m_markBufferBegin < m_markBufferSize || m_in->available();
	}

	String toString() const
	{
		return m_in->toString();
	}

private:
	enum { BUFFER_SIZE = 512 };

	InputStream*		m_in;
	UTFConverter		m_cnv;
	long				m_bufferBegin;
	long				m_bufferEnd;
	uint8_t				m_buffer[BUFFER_SIZE];

	Char*				m_markBuffer;
	int					m_markBufferBegin;
	int					m_markBufferSize;
	int					m_markBufferCapacity;
	int					m_markBufferReset;
	int					m_readlimit;

	long readBytes( void* buffer, long count )
	{
		uint8_t* byteBuffer = reinterpret_cast<uint8_t*>( buffer );
		uint8_t* byteBuffer0 = byteBuffer;
		
		while ( count > 0 ) 
		{
			for ( ; m_bufferBegin < m_bufferEnd && count > 0 ; ++m_bufferBegin, --count )
				*byteBuffer++ = m_buffer[ m_bufferBegin ];

			if ( count > 0 )
			{
				long newBytes = m_in->read( m_buffer, BUFFER_SIZE );
				m_bufferEnd = newBytes;
				m_bufferBegin = 0;
				if ( 0 == newBytes )
					break;
			}
		}

		return byteBuffer - byteBuffer0;
	}

	long peekBytes( void* buffer, long count )
	{
		assert( count <= BUFFER_SIZE );

		if ( m_bufferBegin+count > m_bufferEnd )
		{
			long oldBytes = m_bufferEnd - m_bufferBegin;
			memmove( m_buffer, m_buffer+m_bufferBegin, oldBytes );
			m_bufferEnd = oldBytes;
			m_bufferBegin = 0;

			long c = count;
			while ( m_bufferBegin+c > m_bufferEnd )
			{
				long bytesToRead = BUFFER_SIZE - m_bufferEnd;
				assert( bytesToRead > 0 );
				if ( bytesToRead <= 0 )
					break;
				int bytesRead = m_in->read( m_buffer+m_bufferEnd, bytesToRead );
				if ( 0 == bytesRead )
					break;
				c -= bytesRead;
				m_bufferEnd += bytesRead;
			}
		}

		long bytesPeeked = count;
		if ( m_bufferBegin+bytesPeeked > m_bufferEnd )
			bytesPeeked = m_bufferEnd - m_bufferBegin;
		memcpy( buffer, m_buffer+m_bufferBegin, bytesPeeked );
		return bytesPeeked;
	}

	void compactMarkBuffer()
	{
		for ( int i = m_markBufferReset ; i < m_markBufferSize ; ++i )
			m_markBuffer[i-m_markBufferReset] = m_markBuffer[i];
		m_markBufferSize -= m_markBufferReset;
		m_markBufferBegin -= m_markBufferReset;
		m_markBufferReset = 0;
	}

	InputStreamReaderImpl();
	InputStreamReaderImpl( const InputStreamReaderImpl& );
	InputStreamReaderImpl& operator=( const InputStreamReaderImpl& );
};

//-----------------------------------------------------------------------------

InputStreamReader::InputStreamReader( InputStream* in )
{
	m_this = new InputStreamReaderImpl( in, "ASCII-7" );
}

InputStreamReader::InputStreamReader( InputStream* in, const char* encoding )
{
	m_this = new InputStreamReaderImpl( in, encoding );
}

InputStreamReader::~InputStreamReader()
{
}

long InputStreamReader::read( Char* buffer, long count )
{
	return m_this->read( buffer, count );
}

void InputStreamReader::close()
{
	m_this->close();
}

void InputStreamReader::mark( int readlimit )
{
	m_this->mark( readlimit );
}

void InputStreamReader::reset()
{
	m_this->reset();
}

bool InputStreamReader::markSupported() const
{
	return true;
}

bool InputStreamReader::ready() const
{
	return m_this->ready();
}

String InputStreamReader::toString() const
{
	return m_this->toString();
}


} // io
