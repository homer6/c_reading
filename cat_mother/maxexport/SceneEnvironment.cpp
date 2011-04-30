#include "StdAfx.h"
#include "SceneEnvironment.h"
#include "ChunkUtil.h"
#include <io/ChunkOutputStream.h>

//-----------------------------------------------------------------------------

SceneEnvironment::SceneEnvironment()
{
	ambient[0] = ambient[1] = ambient[2] = 0.f;
}

SceneEnvironment::SceneEnvironment( Interface* ip, Interval animRange )
{
	// ambient
	Interval valid;
	Point3 amb = ip->GetAmbient( animRange.Start(), valid );
	ambient[0] = amb.x;
	ambient[1] = amb.y;
	ambient[2] = amb.z;
}

void SceneEnvironment::write( io::ChunkOutputStream* out )
{
	out->beginChunk( "environment" );
	
	ChunkUtil::writeFloatChunk3( out, "ambient", ambient[0], ambient[1], ambient[2] );

	out->endChunk();
}
