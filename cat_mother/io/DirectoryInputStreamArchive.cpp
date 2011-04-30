#include "DirectoryInputStreamArchive.h"
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileNotFoundException.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace io
{


class DirectoryInputStreamArchive::DirectoryInputStreamArchiveImpl :
	public Object
{
public:
	DirectoryInputStreamArchiveImpl()
	{
		m_entriesDirty = true;
	}

	void addPath( const String& path )
	{
		File dir( path );
		m_paths.add( dir.getAbsolutePath() );
		m_entriesDirty = true;
	}

	void addPaths( const String& path )
	{
		// add root
		File dir( path );
		m_paths.add( dir.getAbsolutePath() );

		// list files in the directory
		Array<String,256> files;
		files.setSize( 256 );
		int count = dir.list( &files[0], files.size() );
		files.setSize( count );
		if ( count > 256 )
			dir.list( &files[0], files.size() );

		// recurse subdirectories
		for ( int i = 0 ; i < files.size() ; ++i )
		{
			File file( path, files[i] );
			if ( file.isDirectory() )
				addPaths( file.getPath() );
		}

		m_entriesDirty = true;
	}

	void removePaths()
	{
		m_paths.clear();
		m_entriesDirty = true;
	}

	void refreshEntriesRecurse( const String& path ) const
	{
		// get files in the path
		Array<String,256> files;
		files.setSize( 256 );
		File dir( path );
		int count = dir.list( &files[0], files.size() );
		files.setSize( count );
		if ( count > 256 )
			dir.list( &files[0], files.size() );

		// add entries
		for ( int i = 0 ; i < files.size() ; ++i )
		{
			File file( path, files[i] );
			m_entries.add( file );
		}

		// recurse subdirectories
		for ( int i = 0 ; i < files.size() ; ++i )
		{
			File file( path, files[i] );
			if ( file.isDirectory() )
				refreshEntriesRecurse( file.getPath() );
		}
	}

	void refreshEntries() const
	{
		Debug::println( "Listing files from directories {0}", toString() );

		m_entries.clear();
		for ( int i = 0 ; i < m_paths.size() ; ++i )
		{
			const String& path = m_paths[i];
			refreshEntriesRecurse( path );
		}
		m_entriesDirty = false;
	}

	String getEntry( int index )
	{
		if ( m_entriesDirty )
			refreshEntries();

		assert( index >= 0 && index < m_entries.size() );
		return m_entries[index].getPath();
	}

	InputStream* getInputStream( const String& name )
	{
		String path = name;
		if ( !File(path).isAbsolute() )
		{
			// relative path
			for ( int k = 0 ; k < m_paths.size() ; ++k )
			{
				File file( m_paths[k], path );
				if ( file.exists() )
				{
					path = file.getAbsolutePath();
					break;
				}
			}
		}

		if ( !File(path).exists() )
			throw FileNotFoundException( Format("File {0} is not in directories {1}", name, toString() ) );
		m_inputStream = new FileInputStream( path );
		return m_inputStream;
	}

	InputStream* getInputStream( int index )
	{
		if ( m_entriesDirty )
			refreshEntries();

		assert( index >= 0 && index < m_entries.size() );
		m_inputStream = new FileInputStream( m_entries[index].getPath() );
		return m_inputStream;
	}

	int size() const
	{
		if ( m_entriesDirty )
			refreshEntries();

		return m_entries.size();
	}

	String toString() const
	{
		String str;
		for ( int i = 0 ; i < m_paths.size() ; ++i )
		{
			if ( i > 0 )
				str = str + File::pathSeparator + " ";
			str = str + m_paths[i];
		}
		return str;
	}

private:
	Array<String,1>			m_paths;
	mutable Array<File,1>	m_entries;
	mutable bool			m_entriesDirty;
	P(InputStream)			m_inputStream;
};

//-----------------------------------------------------------------------------

DirectoryInputStreamArchive::DirectoryInputStreamArchive()
{
	m_this = new DirectoryInputStreamArchiveImpl;
}

DirectoryInputStreamArchive::~DirectoryInputStreamArchive()
{
}
	
void DirectoryInputStreamArchive::addPath( const String& path )
{
	m_this->addPath( path );
}

void DirectoryInputStreamArchive::addPaths( const String& path )
{
	m_this->addPaths( path );
}

void DirectoryInputStreamArchive::removePaths()
{
	m_this->removePaths();
}

void DirectoryInputStreamArchive::close()
{
}

InputStream* DirectoryInputStreamArchive::getInputStream( const String& name )
{
	return m_this->getInputStream( name );
}

InputStream* DirectoryInputStreamArchive::getInputStream( int index )
{
	return m_this->getInputStream( index );
}

String DirectoryInputStreamArchive::getEntry( int index ) const
{
	return m_this->getEntry( index );
}

int DirectoryInputStreamArchive::size() const
{
	return m_this->size();
}

String DirectoryInputStreamArchive::toString() const
{
	return m_this->toString();;
}


} // io
