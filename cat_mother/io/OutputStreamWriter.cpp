#include "OutputStreamWriter.h"
#include <io/OutputStream.h>
#include <lang/UTF16.h>
#include <lang/UTFConverter.h>
#include <stdint.h>
#include <string.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


class OutputStreamWriter::OutputStreamWriterImpl :
	public Object
{
public:
	OutputStreamWriterImpl( OutputStream* out, const char* encoding ) :
		m_conv( encoding ), m_out(out), m_chbuf( Char(-1) )
	{
		// byte order mark candidates
		/*const uint8_t bombytes[][5] = 
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
			if ( !strcmp(bomnames[i],encoding) )
			{
				out->write( &bombytes[i][1], bombytes[i][0] );
				break;
			}
		}*/
	}

	~OutputStreamWriterImpl()
	{
	}

	void writeCP( Char32 cp )
	{
		uint8_t buf[16];
		int bytes;
		if ( m_conv.encode(buf,buf+sizeof(buf),&bytes,cp) )
			m_out->write( buf, bytes );
	}

	void write( const Char* buffer, long count )
	{
		for ( long i = 0 ; i < count ; ++i )
		{
			Char ch = buffer[i];

			if ( UTF16::isFirstSurrogate(ch) )
			{
				// 1st character of a supplementary pair
				m_chbuf = ch;
			}
			else if ( UTF16::isSecondSurrogate(ch) && m_chbuf != Char(-1) )
			{
				// 2nd character of a supplementary pair
				Char32 cp = UTF16::makeCodePoint( m_chbuf, ch );
				writeCP( cp );
				m_chbuf = Char(-1);
			}
			else if ( !UTF16::isSecondSurrogate(ch) )
			{
				// single, non-supplementary character
				writeCP( ch );
				m_chbuf = Char(-1);
			}
		}
	}

	void flush()
	{
		m_out->flush();
	}

	void close()
	{
		m_out->close();
	}

	String toString() const
	{
		return m_out->toString();
	}

private:
	UTFConverter	m_conv;
	OutputStream*	m_out;
	Char			m_chbuf;

	OutputStreamWriterImpl( const OutputStreamWriterImpl& );
	OutputStreamWriterImpl& operator=( const OutputStreamWriterImpl& );
};

//-----------------------------------------------------------------------------

OutputStreamWriter::OutputStreamWriter( OutputStream* out )
{
	m_this = new OutputStreamWriterImpl( out, "ASCII-7" );
}

OutputStreamWriter::OutputStreamWriter( OutputStream* out, const char* encoding )
{
	m_this = new OutputStreamWriterImpl( out, encoding );
}

OutputStreamWriter::~OutputStreamWriter()
{
}

void OutputStreamWriter::write( const Char* buffer, long count )
{
	m_this->write( buffer, count );
}

void OutputStreamWriter::flush()
{
	m_this->flush();
}

void OutputStreamWriter::close()
{
	m_this->close();
}

String OutputStreamWriter::toString() const
{
	return m_this->toString();
}


} // io
