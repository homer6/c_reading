/*
 * Simple file (de)crypt utility.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <lang/Array.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <util/Vector.h>
#include <crypt/CryptUtil.h>
#include <stdint.h>
#include <time.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <config_msvc.h>
#endif

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;
using namespace crypt;

//-----------------------------------------------------------------------------

const char* cstr( String str )
{
	const int COUNT = 8;
	static char buf[COUNT][512];
	static int i = 0;
	i = (i+1)%COUNT;
	str.getBytes( buf[i], sizeof(buf[0]), "ASCII-7" );
	return buf[i];
}

void listFiles( bool recurse, String dir, Vector<File>& files )
{
	Vector<String> fnames( Allocator<String>(__FILE__) );
	fnames.setSize( File(dir).list(0,0) );
	int n = File(dir).list( fnames.begin(), fnames.size() );
	if ( fnames.size() > n )
		fnames.setSize( n );

	for ( int i = 0 ; i < fnames.size() ; ++i )
	{
		String fname = fnames[i];
		File file( dir, fname );
		if ( file.isFile() && !fname.toLowerCase().endsWith(".mp3") )
		{
			files.add( file );
		}
		else if ( file.isDirectory() && recurse )
		{
			listFiles( recurse, file.getPath(), files );
		}
	}
}

void cryptFile( String inputName )
{
	String outputName = CryptUtil::cryptFileName( inputName );
	printf( "encrypting %s\n", cstr(inputName) );

	// read
	FileInputStream fin( inputName );
	Vector<uint8_t> bytes( Allocator<uint8_t>(__FILE__) );
	bytes.setSize( fin.available() );
	fin.read( bytes.begin(), bytes.size() );
	fin.close();

	// encrypt
	CryptUtil::cryptBuffer( bytes.begin(), bytes.begin(), bytes.size(), 0 );

	// write
	FileOutputStream fout( outputName );
	fout.write( bytes.begin(), bytes.size() );
	fout.close();
}

void decryptFile( String inputName )
{
	String outputName = CryptUtil::cryptFileName( inputName );
	printf( "decrypting %s\n", cstr(inputName) );

	// read
	FileInputStream fin( inputName );
	Vector<uint8_t> bytes( Allocator<uint8_t>(__FILE__) );
	bytes.setSize( fin.available() );
	fin.read( bytes.begin(), bytes.size() );
	fin.close();

	// decrypt
	CryptUtil::decryptBuffer( bytes.begin(), bytes.begin(), bytes.size(), 0 );

	// write
	FileOutputStream fout( outputName );
	fout.write( bytes.begin(), bytes.size() );
	fout.close();
}

void printRandomNumbers( const char* fname )
{
	FILE* file = fopen( fname, "wt" );
	static const int CRYPT_RANDOM_COUNT = 1013;
	fprintf( file, "#include <stdint.h>\n\n" );
	fprintf( file, "static const int CRYPT_RANDOM_COUNT = %i;\n\n", CRYPT_RANDOM_COUNT );
	fprintf( file, "static const uint8_t s_cryptRandoms[CRYPT_RANDOM_COUNT] =\n{" );
	srand( time(0) );
	for ( int i = 0 ; i < CRYPT_RANDOM_COUNT ; ++i )
	{
		if ( i % 50 == 0 )
			fprintf( file, "\n\t" );

		int x = rand();
		x = (x & 0xFF0)>>4;

		fprintf( file, "%i,", x );
	}
	fprintf( file, "\n};\n" );
	fclose( file );
}

int main( int argc, char* argv[] )
{
	try
	{
		if ( argc < 2 )
		{
			printf( "----------------------------------------------------------------\n" );
			printf( "!WARNING: THIS UTILITY MODIFIES ALL FILES IN ALL SUBDIRECTORIES!\n" );
			printf( "----------------------------------------------------------------\n" );
			printf( "usage: crypt <options>\n" );
			printf( "options:\n" );
			printf( "-e         encrypt, recursive\n" );
			printf( "-d         decrypt, recursive\n" );
			return 0;
		}

		// get options and extensions
		Vector<String> exts( Allocator<String>(__FILE__) );
		bool recurse = true;
		bool encrypt = false;
		bool decrypt = false;
		for ( int i = 1 ; i < argc ; ++i )
		{
			String arg = String(argv[i]);
			if ( arg == "-e" )
				encrypt = true;
			if ( arg == "-d" )
				decrypt = true;
		}

		// get list of files
		Vector<File> files( Allocator<File>(__FILE__) );
		listFiles( recurse, "", files );

		// DEBUG: print found path/names
		//for ( int i = 0 ; i < files.size() ; ++i )
		//	printf( "%s/%s\n", cstr(files[i].path), cstr(files[i].fname) );

		// encrypt/decrypt files
		for ( int i = 0 ; i < files.size() ; ++i )
		{
			File fd = files[i];
			if ( encrypt && !decrypt )
				cryptFile( fd.getPath() );
			if ( !encrypt && decrypt )
				decryptFile( fd.getPath() );
		}

		// fprint random numbers to random.h
		//printRandomNumbers( "/projects/tools/crypt/random.h" );
	}
	catch ( Throwable& e )
	{
		printf( "Error: %s\n", cstr(e.getMessage().format()) );
		return 1;
	}
	return 0;
}
