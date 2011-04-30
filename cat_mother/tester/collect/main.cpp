/*
 * Creates a makefile / project file from all test_*.cpp files in /projects dir.
 */
#include "listFiles.h"
#include <io/File.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <io/InputStreamReader.h>
#include <io/OutputStreamWriter.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <util/Vector.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <config_msvc.h>
#endif

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;

//-----------------------------------------------------------------------------

static String readFile( String fname )
{
	FileInputStream fin( fname );
	InputStreamReader reader( &fin );
	Vector<Char> buf( Allocator<Char>(__FILE__) );
	Char ch;
	while ( reader.read(&ch,1) > 0 )
		buf.add( ch );
	fin.close();
	return String( buf.begin(), buf.size() );
}

static void backup( String fname )
{
	char name[512];
	char bak[512];
	fname.getBytes( name, sizeof(name), "ASCII-7" );
	(fname+".bak").getBytes( bak, sizeof(bak), "ASCII-7" );

	FILE* srcf = fopen( name, "rb" );
	if ( !srcf )
		throw Exception( Format("Failed to read {0}", name) );

	FILE* dstf = fopen( bak, "wb" );
	if ( !dstf )
	{
		fclose( srcf );
		throw Exception( Format("Failed to write {0}", bak) );
	}
	
	const int BUF_SIZE = 1024;
	char buf[BUF_SIZE];
	int bytes;
	while ( (bytes=fread(buf,1,BUF_SIZE,srcf)) > 0 )
		fwrite( buf, 1, bytes, dstf );

	fclose( srcf );
	fclose( dstf );
}

static void writeFile( String fname, String data )
{
	backup( fname );
	/*FileOutputStream fout( fname );
	OutputStreamWriter writer( &fout, "ASCII-7" );
	Vector<Char> buf( Allocator<Char>(__FILE__) );
	buf.setSize( data.length() );
	data.getChars( 0, data.length(), buf.begin() );
	writer.write( buf.begin(), buf.size() );
	fout.close();*/
	Vector<char> buf( Allocator<char>(__FILE__) );
	buf.setSize( data.length()+4 );
	int bytes = data.getBytes( buf.begin(), buf.size(), "ASCII-7" );
	char str[512];
	fname.getBytes( str, sizeof(str), "ASCII-7" );
	FILE* fh = fopen( str, "wt" );
	fwrite( buf.begin(), 1, bytes, fh );
	fclose( fh );
}

static String readPrjFile( String prjdir )
{
#ifdef _MSC_VER
	String fname = "all_dsp";
#else
	#error "project file template name undefined"
#endif

	return readFile( fname );
}

static void writePrjFile( String prjdir, String data )
{
#ifdef _MSC_VER
	String fname = "../all/all.dsp";
#else
	#error "project file name undefined"
#endif

	writeFile( fname, data );
}

static String format( String fname )
{
#ifdef _MSC_VER
	return "# Begin Source File\n\nSOURCE=\"" + fname.replace('/','\\') + "\"\n# End Source File\n";
#else
	#error format undefined
#endif
}

static int run()
{
	String prjdir = "/projects";
	String prefix = "test_";
	String suffix = ".cpp";
	String srctag = "# INSERT_SOURCE_FILES_HERE";

	// list test_*.cpp
	printf( "Collecting test_*.cpp...\n" );
	Vector<String> files( Allocator<String>(__FILE__) );
	listFiles( prjdir, prefix, suffix, files );

	// read prj file template
	printf( "Processing project file template...\n" );
	String prj = readPrjFile( prjdir );
	String filelist = "";
	for ( int i = 0 ; i < files.size() ; ++i )
		filelist = filelist + format(files[i]);
	//filelist = filelist + format("./Copy of test_dummy.cpp");
	//filelist = filelist + format("./test_dummy.cpp");

	// find position for source file list
	int index = prj.indexOf(srctag);
	if ( index < 0 )
	{
		printf( "Missing \"# INSERT_SOURCE_FILES_HERE\" in project file template\n" );
		return 3;
	}

	// replace srctag with filelist
	prj = prj.substring(0,index) + filelist + prj.substring(index+srctag.length(),prj.length());
	
	printf( "Writing project file...\n" );
	writePrjFile( prjdir, prj );
	printf( "Done!\n" );
	return 0;
}

//-----------------------------------------------------------------------------

int main()
{
	try
	{
		return run();
	}
	catch ( Throwable& e )
	{
		char str[512];
		e.getMessage().format().getBytes( str, sizeof(str), "ASCII-7" );
		puts( str );
		return 1;
	}
	catch ( ... )
	{
		printf( "Unknown exception!\n" );
		return 2;
	}
	return 0;
}
