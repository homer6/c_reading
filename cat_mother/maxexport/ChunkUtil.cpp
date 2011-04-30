#include "StdAfx.h"
#include "ChunkUtil.h"
#include <io/ChunkOutputStream.h>
#include "KeyFrame.h"
#include "KeyFrameContainer.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

void ChunkUtil::writeIntChunk( ChunkOutputStream* out, const String& name, int x )
{
	out->beginChunk( name );
	out->writeInt( x );
	out->endChunk();
}

void ChunkUtil::writeFloatChunk( ChunkOutputStream* out, const String& name, float x )
{
	out->beginChunk( name );
	out->writeFloat( x );
	out->endChunk();
}

void ChunkUtil::writeStringChunk( ChunkOutputStream* out, const String& name, const String& x )
{
	out->beginChunk( name );
	out->writeString( x );
	out->endChunk();
}

void ChunkUtil::writeIntChunk2( ChunkOutputStream* out, const String& name, int x1, int x2 )
{
	out->beginChunk( name );
	out->writeInt( x1 );
	out->writeInt( x2 );
	out->endChunk();
}

void ChunkUtil::writeIntChunk3( ChunkOutputStream* out, const String& name, int x1, int x2, int x3 )
{
	out->beginChunk( name );
	out->writeInt( x1 );
	out->writeInt( x2 );
	out->writeInt( x3 );
	out->endChunk();
}

void ChunkUtil::writeFloatChunk2( ChunkOutputStream* out, const String& name, float x1, float x2 )
{
	out->beginChunk( name );
	out->writeFloat( x1 );
	out->writeFloat( x2 );
	out->endChunk();
}

void ChunkUtil::writeFloatChunk3( ChunkOutputStream* out, const String& name, float x1, float x2, float x3 )
{
	out->beginChunk( name );
	out->writeFloat( x1 );
	out->writeFloat( x2 );
	out->writeFloat( x3 );
	out->endChunk();
}

void ChunkUtil::writeFloatChunk4( ChunkOutputStream* out, const String& name, float x1, float x2, float x3, float x4 )
{
	out->beginChunk( name );
	out->writeFloat( x1 );
	out->writeFloat( x2 );
	out->writeFloat( x3 );
	out->writeFloat( x4 );
	out->endChunk();
}

void ChunkUtil::writeAnim( ChunkOutputStream* out, const KeyFrameContainer& anim )
{
	out->writeInt( anim.channels() );
	out->writeInt( anim.keys() );
	out->writeInt( anim.interpolation() );

	for ( int i = 0 ; i < anim.keys() ; ++i )
	{
		const KeyFrame& key = anim.getKey(i);
		
		out->writeFloat( key.time );
		
		for ( int k = 0 ; k < anim.channels() ; ++k )
			out->writeFloat( key.getChannel(k) );
	}
}

void ChunkUtil::writeAnimChunk( ChunkOutputStream* out, const String& name, const KeyFrameContainer& anim )
{
	out->beginChunk( name );
	writeAnim( out, anim );
	out->endChunk();
}

void ChunkUtil::writeVector3( ChunkOutputStream* out, const math::Vector3& v )
{
	for ( int k = 0 ; k < 3 ; ++k )
		out->writeFloat( v[k] );
}

void ChunkUtil::writeTriangleList( ChunkOutputStream* out, const util::Vector<math::Vector3>& vec )
{
	require( vec.size() % 3 == 0 );

	out->writeInt( vec.size()/3 );
	for ( int i = 0 ; i < vec.size() ; ++i )
		writeVector3( out, vec[i] );
}
