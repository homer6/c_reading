#include "VertexFormat.h"
#include <stdio.h>
#include <string.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;

//-----------------------------------------------------------------------------

namespace sg
{


static void append( char* buf, int bufSize, const char* str )
{
	bool fitsToBuffer = ( (int)(strlen(buf)+strlen(str)) < bufSize);
	assert( fitsToBuffer );

	if ( fitsToBuffer )
	{
		if ( *buf == 0 )
		{
			strcpy( buf, str );
		}
		else
		{
			strcat( buf, "|" );
			strcat( buf, str );
		}
	}
}

//-----------------------------------------------------------------------------

String VertexFormat::toString() const
{
	const int bufSize = 512;
	char buf[bufSize+1] = "XYZ";

	if ( hasRHW() )
		append( buf, bufSize, "RHW" );
	if ( hasNormal() )
		append( buf, bufSize, "NORMAL" );
	if ( hasDiffuse() )
		append( buf, bufSize, "DIFFUSE" );
	if ( hasSpecular() )
		append( buf, bufSize, "SPECULAR" );

	char msg[512];
	for ( int i = 0 ; i < textureCoordinates() ; ++i )
	{
		sprintf( msg, "TEXCOORDSIZE%i(%i)", getTextureCoordinateSize(i), i );
		append( buf, bufSize, msg );
	}

	if ( weights() > 0 )
	{
		sprintf( msg, "WEIGHTS=%i", weights() );
		append( buf, bufSize, msg );
	}

	return buf;
}


} // sg
