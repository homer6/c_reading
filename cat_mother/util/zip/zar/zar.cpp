/*
 * Small in-house command line zip file utility.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <zlib.h>
#ifdef _MSC_VER
#include <config_msvc.h>
#endif

//-----------------------------------------------------------------------------

struct File
{
	std::string				name;
	std::string				path;
	std::vector<uint8_t>	data;
};

//-----------------------------------------------------------------------------

static void readFiles( std::vector<File>& files, const char* fname )
{
	char cwd[2048];
	_getcwd( cwd, sizeof(cwd) );

	_finddata_t info;
	long handle = _findfirst( fname, &info );
	if ( -1 != handle )
	{
		File file;

		do
		{
			if ( info.name[0] != '.' )
			{
				file.name = info.name;
				file.path = cwd;

				printf( "Reading %s...\n", info.name );

				FILE* fh = fopen( info.name, "rb" );
				if ( fh )
				{
					fseek( fh, 0, SEEK_END );
					file.data.resize( ftell( fh ) );
					fseek( fh, 0, SEEK_SET );
					fread( file.data.begin(), 1, file.data.size(), fh );

					if ( !ferror(fh) )
						files.push_back( file );

					fclose( fh );
				}
			}

		} while ( 0 == _findnext(handle,&info) );
	}

	handle = _findfirst( "*", &info );
	if ( -1 != handle )
	{
		do
		{
			if ( 0 != (info.attrib & _A_SUBDIR) && info.name[0] != '.' )
			{
				if ( 0 == _chdir(info.name) )
				{
					printf( "Recursing to %s...\n", info.name );
					readFiles( files, fname );
					_chdir( cwd );
				}
			}
		} while ( 0 == _findnext(handle,&info) );
	}
}

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

static void writeInt( gzFile file, int value )
{
	int32_t v = value;
	uint8_t* bytes = reinterpret_cast<uint8_t*>( &v );
	if ( !bigEndian() )
		reverse( bytes, bytes+sizeof(v) );
	gzwrite( file, bytes, sizeof(int32_t) );
}

static void writeString( gzFile file, const char* str )
{
	int len = strlen(str);
	writeInt( file, len );

	int i;
	for ( i = 0 ; i < len ; ++i )
	{
		char ch = str[i];
		ch &= 0x7F;
		gzwrite( file, &ch, 1 );
	}
}

static void writeBytes( gzFile file, const uint8_t* bytes, int count )
{
	writeInt( file, count );
	gzwrite( file, (void*)bytes, count );
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

static void readString( gzFile file, std::string& str )
{
	int len = readInt( file );

	int i;
	for ( i = 0 ; i < len ; ++i )
	{
		char ch = 0;
		gzread( file, &ch, 1 );
		str += ch;
	}
}

static void readBytes( gzFile file, std::vector<uint8_t>& bytes )
{
	int count = readInt( file );
	bytes.resize( count );
	gzread( file, bytes.begin(), count );
}

//-----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
	char cwd[2048];
	_getcwd( cwd, sizeof(cwd) );

	// zar <opt> <dir>
	if ( argc != 4 )
	{
		printf( "Usage:\n"
			"archive:   zar -a <name> <dir>\n"
			"unarchive: zar -x <name> <dir>\n"
			"remove:    zar -d <name> <filelist>\n" );
		return 0;
	}
	const char* opt		= argv[1];
	const char* name	= argv[2];
	const char* dir		= argv[3];

	if ( std::string(opt) == "-a" )
	{
		// read files
		std::vector<File> files;
		if ( -1 != _chdir(dir) )
		{
			printf( "Processing %s...\n", dir );
			readFiles( files, "*.*" );
		}
		else
		{
			printf( "Path %s not found\n", dir );
		}
		_chdir( cwd );

		// check for duplicate file names
		int i;
		for ( i = 0 ; i < (int)files.size() ; ++i )
		{
			for ( int j = i+1 ; j < (int)files.size() ; ++j )
			{
				if ( files[i].name == files[j].name )
				{
					printf( "Warning: Two files have identical names (%s). Paths:\n", files[i].name.c_str() );
					printf( "%s\n", files[i].path.c_str() );
					printf( "%s\n", files[j].path.c_str() );
					printf( "Removing %s/%s...\n", files[j].path.c_str(), files[j].name.c_str() );
					files.erase( files.begin()+j );
					--j;
				}
			}
		}

		// write zip
		printf( "Writing %s...\n", name );
		gzFile zip = gzopen( name, "wb" );
		writeInt( zip, files.size() );
		for ( i = 0 ; i < (int)files.size() ; ++i )
		{
			File& file = files[i];
			writeString( zip, file.name.c_str() );
			writeBytes( zip, file.data.begin(), file.data.size() );
		}
		gzclose( zip );
	}
	else if ( std::string(opt) == "-x" )
	{
		// read zip
		printf( "Reading %s...\n", name );
		gzFile zip = gzopen( name, "rb" );
		if ( !zip )
		{
			printf( "Failed to open %s.", name );
			return 1;
		}
		std::vector<File> files;
		int count = readInt( zip );
		files.resize( count );
		for ( int i = 0 ; i < count ; ++i )
		{
			File& file = files[i];
			readString( zip, file.name );
			readBytes( zip, file.data );
		}
		gzclose( zip );

		// write files
		if ( -1 != _chdir(dir) )
		{
			for ( int i = 0 ; i < (int)files.size() ; ++i )
			{
				File& file = files[i];
				printf( "Writing %s...\n", file.name.c_str() );
				FILE* fh = fopen( file.name.c_str(), "wb" );
				if ( fh )
				{
					fwrite( file.data.begin(), 1, file.data.size(), fh );
					fclose( fh );
				}
				else
				{
					printf( "Cannot write to file: %s\n", file.name.c_str() );
				}
			}
		}
		else
		{
			printf( "Path %s not found\n", dir );
		}

		_chdir( cwd );
	}
	else if ( std::string(opt) == "-d" )
	{
		// read zip
		printf( "Reading %s...\n", name );
		gzFile zip = gzopen( name, "rb" );
		if ( !zip )
		{
			printf( "Failed to open %s.", name );
			return 1;
		}
		std::vector<File> files;
		int count = readInt( zip );
		files.resize( count );
		for ( int i = 0 ; i < count ; ++i )
		{
			File& file = files[i];
			readString( zip, file.name );
			readBytes( zip, file.data );
		}
		gzclose( zip );

		// read file list
		printf( "Reading list %s...\n", dir );
		FILE* fh = fopen( dir, "rt" );
		if ( !fh )
		{
			printf( "File list not found: %s\n", dir );
			return 1;
		}
		std::vector<std::string> filelist;
		char buf[2048];
		while ( !feof(fh) )
		{
			if ( 1 != fscanf(fh,"%s",buf) )
				break;
			filelist.push_back( buf );
		}

		// remove listed files
		for ( int i = 0 ; i < (int)filelist.size() ; ++i )
		{
			std::string str = filelist[i];
			for ( int j = 0 ; j < (int)files.size() ; ++j )
			{
				if ( files[j].name == str )
				{
					files.erase( files.begin()+j );
					--j;
				}
			}
		}

		// write zip
		printf( "Writing %s...\n", name );
		zip = gzopen( name, "wb" );
		writeInt( zip, files.size() );
		for ( int i = 0 ; i < (int)files.size() ; ++i )
		{
			File& file = files[i];
			writeString( zip, file.name.c_str() );
			writeBytes( zip, file.data.begin(), file.data.size() );
		}
		gzclose( zip );
	}
	else
	{
		printf( "Invalid option: %s\n", opt );
	}

	return 0;
}
