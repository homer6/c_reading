#include "FileInputStream.h"
#include "StdioEx.h"
#include "FileNotFoundException.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


class FileInputStream::FileInputStreamImpl :
	public Object
{
public:
	FILE*	file;
	String	name;

	FileInputStreamImpl( FILE* file, const String& name )
	{
		this->file = file;
		this->name = name;
	}

	~FileInputStreamImpl()
	{
		close();
	}

	void close()
	{
		if ( file )
		{
			fclose( file );
			file = 0;
		}
	}
};

//-----------------------------------------------------------------------------

FileInputStream::FileInputStream( const String& filename )
{
	FILE* handle = fopen( filename, "rb" );
	if ( !handle )
		throw FileNotFoundException( Format("Failed to open file for reading: {0}",filename) );

	m_this = new FileInputStreamImpl( handle, filename );
}

FileInputStream::~FileInputStream()
{
}

void FileInputStream::close()
{
	if ( m_this )
	{
		m_this->close();
	}
}

long FileInputStream::read( void* data, long size )
{
	long bytes = fread( data, 1, size, m_this->file );
	if ( bytes < size && ferror(m_this->file) )
		throw IOException( Format("Failed to read from file: {0}",m_this->name) );

	return bytes;
}

long FileInputStream::available() const
{
	long cur = ftell( m_this->file );
	fseek( m_this->file, 0, SEEK_END );
	long end = ftell( m_this->file );
	fseek( m_this->file, cur, SEEK_SET );
	
	if ( ferror(m_this->file) )
		throw IOException( Format("Failed to seek file position: {0}",m_this->name) );

	return end-cur;
}

String FileInputStream::toString() const
{
	return m_this->name;
}


} // io
