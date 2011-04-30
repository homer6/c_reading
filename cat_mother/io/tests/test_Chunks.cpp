#include <tester/Test.h>
#include <io/ChunkOutputStream.h>
#include <io/ChunkInputStream.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <io/DataOutputStream.h>
#include <io/DataInputStream.h>
#include <lang/Throwable.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

static void writeTest( const char* filename )
{
	P(FileOutputStream) fout = new FileOutputStream( filename );
	P(ChunkOutputStream) writer = new ChunkOutputStream( fout );

	writer->beginChunk( "mainchunk" );
		writer->writeInt( 1234 );
		writer->beginChunk( "subchunk1" );
			writer->beginChunk( "subsubchunk1" );
			writer->writeFloat( 12.34f );
			writer->writeString( "Hello, world!" );
			writer->writeString( "Another string" );
			writer->endChunk();
		writer->endChunk();
		writer->beginChunk( "subchunk2" );
			writer->beginChunk( "subsubchunk2" );
			writer->endChunk();
		writer->endChunk();
	writer->endChunk();

	fout->close();
}

static void readInt( ChunkInputStream* reader, int ref )
{
	int x = reader->readInt();
	assert( x == ref );
	x = ref = 0;
}

static void readFloat( ChunkInputStream* reader, float ref )
{
	float x = reader->readFloat();
	assert( x == ref );
	x = ref = 0.f;
}

static void readString( ChunkInputStream* reader, String ref )
{
	String x = reader->readString();
	assert( x == ref );
	x = ref = "";
}

static void readTest( const char* filename )
{
	P(FileInputStream) fin = new FileInputStream( filename );
	P(ChunkInputStream) reader = new ChunkInputStream( fin );

	String str;
	long end[8];
	reader->beginChunk( &str, &end[0] );
		readInt( reader, 1234 );
		reader->beginChunk( &str, &end[1] ); assert( str == "subchunk1" );
			reader->beginChunk( &str, &end[2] ); assert( str == "subsubchunk1" );
			readFloat( reader, 12.34f );
			readString( reader, "Hello, world!" );
			readString( reader, "Another string" );
			reader->endChunk( end[2] );
		reader->endChunk( end[1] );
		reader->beginChunk( &str, &end[3] ); assert( str == "subchunk2" );
			reader->beginChunk( &str, &end[4] ); assert( str == "subsubchunk2" );
			reader->endChunk( end[4] );
		reader->endChunk( end[3] );
	reader->endChunk( end[0] );

	fin->close();
}

static int test()
{
	writeTest( "/tmp/out/chunks.dat" );
	readTest( "/tmp/out/chunks.dat" );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
