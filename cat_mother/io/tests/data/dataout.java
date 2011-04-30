import java.io.DataOutputStream;
import java.io.FileOutputStream;


/**
 * Writes 'dataout_java.dat' to test that C++ classes can
 * read input written from Java.
 */
public class dataout
{
	public static void main( String[] args )
		throws Exception
	{
		FileOutputStream fout = new FileOutputStream( "dataout_java.dat" );
		DataOutputStream dout = new DataOutputStream( fout );

		dout.writeUTF( "Boolean" );
		dout.writeBoolean( false );

		dout.writeUTF( "Byte" );
		dout.writeByte( 0x66 );
		
		dout.writeUTF( "Char" );
		dout.writeChar( (char)0xC4 );
		
		dout.writeUTF( "Double" );
		dout.writeDouble( 123.456 );

		dout.writeUTF( "Float" );
		dout.writeFloat( 123.456f );

		dout.writeUTF( "Int" );
		dout.writeInt( 0x1234 );

		dout.writeUTF( "Long" );
		dout.writeLong( 0x1234 );

		dout.writeUTF( "Short" );
		dout.writeShort( 0x1234 );

		char[] sz = new char[256];
		int szlen = 0;
		sz[szlen++] = 'H';
		sz[szlen++] = 'e';
		sz[szlen++] = 'l';
		sz[szlen++] = 'l';
		sz[szlen++] = (char)0xF6;
		sz[szlen++] = '!';
		sz[szlen]	= (char)0;

		dout.writeUTF( "UTF" );
		String str = new String(sz, 0, szlen);
		dout.writeUTF( str );

		fout.flush();
		fout.close();
	}
}
