#include "StdAfx.h"
#include "SgLOD.h"
#include "ChunkUtil.h"
#include <io/ChunkOutputStream.h>
#include <lang/Debug.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

SgLOD::SgLOD()
{
	lodID = "";
}

void SgLOD::write( ChunkOutputStream* out ) const
{
	out->beginChunk( "lod" );

	SgNode::writeNodeChunks( out );

	out->endChunk();
}
