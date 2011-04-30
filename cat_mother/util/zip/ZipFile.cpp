#include "ZipFile.h"
#include "MemoryInputStream.h"
#include <io/IOException.h>
#include <io/FileInputStream.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <assert.h>
#include <stdint.h>
#include <zlib.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

namespace util
{
namespace zip
{


//-----------------------------------------------------------------------------

class ZipFile::ZipFileImpl :
	public Object
{
public:
	struct Entry
	{
		String	name;
		int		begin;
		int		size;
		bool	used;
	};

	Vector<Entry>		entries;
	Vector<uint8_t>		data;
	String				name;

	ZipFileImpl() :
		entries( Allocator<Entry>(__FILE__) ),
		data( Allocator<uint8_t>(__FILE__) ),
		name( "" )
	{
	}

private:
	ZipFileImpl( const ZipFileImpl& );
	ZipFileImpl& operator=( const ZipFileImpl& );
};

//-----------------------------------------------------------------------------

static bool bigEndian()
{
	uint32_t x = 1;
	return 0 == *reinterpret_cast<uint8_t*>(&x);
}

static void reverse( uint8_t* begin, uint8_t* end )
{
	for ( ; begin != end && begin != --end ; ++begin )
	{
		uint8_t b = *begin;
		*begin = *end;
		*end = b;
	}
}

static int readInt( gzFile file )
{
	int32_t v = 0;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	gzread( file, bytes, sizeof(int32_t) );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );
	return v;
}

static String readString( gzFile file )
{
	int len = readInt( file );

	char localbuff[1024];
	char* buff = localbuff;
	if ( len > 1023 )
		buff = new char[len];

	int i;
	for ( i = 0 ; i < len ; ++i )
	{
		char ch = 0;
		gzread( file, &ch, 1 );
		buff[i] = ch;
	}
	buff[i] = 0;

	String str = buff;
	if ( buff != localbuff )
	{
		delete[] buff;
		buff = 0;
	}

	return str.toLowerCase();
}

static String stripPath( const String& name )
{
	String str;
	int end = name.lastIndexOf('/');
	int end2 = name.lastIndexOf('\\');
	if ( end2 > end )
		end = end2;
	if ( end >= 0 && end+1 < name.length() )
		str = name.substring( end+1 );
	else
		str = name;
	return str;
}

//-----------------------------------------------------------------------------

ZipFile::ZipFile( const String& name )
{
	gzFile zf = 0;

	try
	{
		m_this = new ZipFileImpl;
		m_this->name = name;

		// open the zip
		char namebuff[2048];
		name.getBytes( namebuff, sizeof(namebuff), "ASCII-7" );
		zf = gzopen( namebuff, "rb" );
		if ( !zf )
			throw IOException( Format("Failed to open zip file: {0}",name) );

		// read entries
		int files = readInt( zf );
		for ( int i = 0 ; i < files ; ++i )
		{
			ZipFileImpl::Entry entry;
			entry.name = readString( zf ).toLowerCase();
			entry.begin = m_this->data.size();
			entry.size = readInt( zf );
			entry.used = false;
			
			m_this->data.setSize( entry.begin + entry.size );
			if ( entry.size != gzread( zf, entry.begin + m_this->data.begin(), entry.size ) )
				throw IOException( Format("Failed to read from zip file: {0}",name) );

			m_this->entries.add( entry );
		}

		gzclose( zf );
	}
	catch ( ... )
	{
		if ( zf )
		{
			gzclose( zf );
			zf = 0;
		}
		throw;
	}

}

void ZipFile::close()
{
	Debug::println( "Unused zip entries in {0}:", m_this->name );
	for ( int i = 0 ; i < (int)m_this->entries.size() ; ++i )
	{
		ZipFileImpl::Entry& entry = m_this->entries[i];
		if ( !entry.used )
			Debug::println( "  %s", entry.name );
	}
	Debug::println( "<end of unused zip entries>" );
}

InputStream* ZipFile::getInputStream( const String& name )
{
	String str = stripPath( name );
	String namelower = str.toLowerCase();

	for ( int i = 0 ; i < (int)m_this->entries.size() ; ++i )
	{
		ZipFileImpl::Entry& entry = m_this->entries[i];
		if ( entry.name == namelower )
		{
			entry.used = true;
			return new MemoryInputStream( this, m_this->data.begin()+entry.begin, entry.size, m_this->name+"/"+entry.name );
		}
	}

	return new FileInputStream( name );
}

InputStream* ZipFile::getInputStream( int index )
{
	assert( index >= 0 && index < size() );
	ZipFileImpl::Entry& entry = m_this->entries[index];
	entry.used = true;
	return new MemoryInputStream( this, m_this->data.begin()+entry.begin, entry.size, m_this->name+"/"+entry.name );
}

lang::String ZipFile::getEntry( int index ) const
{
	assert( index >= 0 && index < size() );
	return m_this->entries[index].name;
}

int ZipFile::size() const
{
	return m_this->entries.size();
}

String ZipFile::toString() const
{
	return m_this->name;
}


} // zip
} // util
