#include "FileOutputStream.h"
#include "StdioEx.h"
#include "FileNotFoundException.h"
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


class FileOutputStream::FileOutputStreamImpl :
	public Object
{
public:
	FILE*	file;
	String	name;

	FileOutputStreamImpl( FILE* file, const String& name )
	{
		this->file = file;
		this->name = name;
	}

	~FileOutputStreamImpl()
	{
		close();
	}

	void close()
	{
		if ( file )
		{
			fflush( file );
			fclose( file );
			file = 0;
		}
	}
};

//-----------------------------------------------------------------------------

FileOutputStream::FileOutputStream( const String& filename )
{
	FILE* handle = fopen( filename, "wb" );
	if ( !handle )
		throw FileNotFoundException( Format("Failed to open file for writing: {0}",filename) );

	m_this = new FileOutputStreamImpl( handle, filename );
}

FileOutputStream::~FileOutputStream()
{
}

void FileOutputStream::close()
{
	if ( m_this )
	{
		m_this->close();
	}
}

void FileOutputStream::flush()
{
	if ( 0 != fflush(m_this->file) )
		throw IOException( Format("Cannot flush file output stream: {0}",m_this->name) );
}

void FileOutputStream::write( const void* data, long size )
{
	long bytes = fwrite( data, 1, size, m_this->file );
	if ( bytes != size || ferror(m_this->file) )
		throw IOException( Format("Failed to write to file: {0}",m_this->name) );
}

String FileOutputStream::toString() const
{
	return m_this->name;
}


} // io
