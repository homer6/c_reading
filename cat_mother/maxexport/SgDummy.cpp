#include "StdAfx.h"
#include "SgDummy.h"
#include "ChunkUtil.h"
#include <io/ChunkOutputStream.h>
#include <lang/Debug.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

SgDummy::SgDummy() :
	boxMin(0,0,0),
	boxMax(0,0,0)
{
}

void SgDummy::write( ChunkOutputStream* out ) const
{
	out->beginChunk( "dummy" );
	
	SgNode::writeNodeChunks( out );

	// write box
	out->beginChunk( "box" );
	out->writeFloat( boxMin.x );
	out->writeFloat( boxMin.y );
	out->writeFloat( boxMin.z );
	out->writeFloat( boxMax.x );
	out->writeFloat( boxMax.y );
	out->writeFloat( boxMax.z );
	out->endChunk();
	
	out->endChunk();
}
