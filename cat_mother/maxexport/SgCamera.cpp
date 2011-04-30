#include "StdAfx.h"
#include "SgCamera.h"
#include "ChunkUtil.h"
#include <io/ChunkOutputStream.h>
#include <lang/Debug.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

SgCamera::SgCamera()
{
	resampleAnimations = false;
}

void SgCamera::write( ChunkOutputStream* out ) const
{
	out->beginChunk( "camera" );
	SgNode::writeNodeChunks( out );

	Debug::println( "    fov = {0}", fov );
	ChunkUtil::writeFloatChunk( out, "fov", fov );

	out->endChunk();
}
