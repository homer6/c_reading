#include <tester/Test.h>
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
	P(DataOutputStream) dout = new DataOutputStream( fout );

	dout->writeUTF( "Boolean" );
	dout->writeBoolean( false );

	dout->writeUTF( "Byte" );
	dout->writeByte( 0x66 );
	
	dout->writeUTF( "Char" );
	dout->writeChar( 0xC4 );
	
	dout->writeUTF( "Double" );
	dout->writeDouble( 123.456 );

	dout->writeUTF( "Float" );
	dout->writeFloat( 123.456f );

	dout->writeUTF( "Int" );
	dout->writeInt( 0x1234 );

	dout->writeUTF( "Long" );
	dout->writeLong( 0x1234 );

	dout->writeUTF( "Short" );
	dout->writeShort( 0x1234 );

	lang::Char sz[256];
	int szlen = 0;
	sz[szlen++] = 'H';
	sz[szlen++] = 'e';
	sz[szlen++] = 'l';
	sz[szlen++] = 'l';
	sz[szlen++] = (lang::Char)0xF6;
	sz[szlen++] = '!';
	sz[szlen]	= (lang::Char)0;

	dout->writeUTF( "UTF" );
	dout->writeUTF( lang::String(sz,szlen) );
	dout->close();
}

static void readTest( const char* filename )
{
	P(FileInputStream) fin = new FileInputStream( filename );
	P(DataInputStream) din = new DataInputStream( fin );

	String name = din->readUTF();
	bool boolVar = din->readBoolean();
	assert( name == "Boolean" );
	assert( boolVar == false );

	name = din->readUTF();
	int byteVar = din->readByte();
	assert( name == "Byte" );
	assert( byteVar == 0x66 );
	
	name = din->readUTF();
	lang::Char charVar = din->readChar();
	assert( name == "Char" );
	assert( charVar == lang::Char(0xC4) );
	
	name = din->readUTF();
	double doubleVar = din->readDouble();
	assert( name == "Double" );
	assert( doubleVar == 123.456 );

	name = din->readUTF();
	float floatVar = din->readFloat();
	assert( name == "Float" );
	assert( floatVar == 123.456f );

	name = din->readUTF();
	int intVar = din->readInt();
	assert( name == "Int" );
	assert( intVar == 0x1234 );

	name = din->readUTF();
	long longVar = din->readLong();
	assert( name == "Long" );
	assert( longVar == 0x1234 );

	name = din->readUTF();
	int shortVar = din->readShort();
	assert( name == "Short" );
	assert( shortVar == 0x1234 );

	//throw Throwable( Format("Test exception: {2}, {1}, {0}",3,2,1) );

	lang::Char sz[256];
	int szlen = 0;
	sz[szlen++] = 'H';
	sz[szlen++] = 'e';
	sz[szlen++] = 'l';
	sz[szlen++] = 'l';
	sz[szlen++] = (lang::Char)0xF6;
	sz[szlen++] = '!';
	sz[szlen]	= (lang::Char)0;

	name = din->readUTF();
	lang::String str = din->readUTF();
	assert( name == "UTF" );
	assert( str == String(sz,szlen) );
	din->close();
}

static int test_DataStreams1()
{
	writeTest( "/tmp/out/dataout.dat" );
	readTest( "/tmp/out/dataout.dat" );
	readTest( "data/dataout_java.dat" );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test_DataStreams1, __FILE__ );
