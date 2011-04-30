#include "StdAfx.h"
#include "SgMesh.h"
#include "GmModel.h"
#include "ChunkUtil.h"
#include "ShadowVolumeBuilder.h"
#include <io/ChunkOutputStream.h>
#include <lang/Debug.h>
#include <lang/Float.h>
#include <util/Vector.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

SgMesh::Bone::Bone()
{
	index	= -1;
	rest	= math::Matrix4x4(1);
}

//-----------------------------------------------------------------------------

SgMesh::SgMesh() :
	bones( Allocator<Bone>(__FILE__,__LINE__) )
{
	lodID		= "";
	lodNum		= -1;
	lodMin		= 0.f;
	lodMax		= Float::MAX_VALUE;
	model		= 0;
	shadow		= 0;
	obb			= OBBox();
}

SgMesh::~SgMesh()
{
}

void SgMesh::write( ChunkOutputStream* out ) const
{
	out->beginChunk( "mesh" );
	SgNode::writeNodeChunks( out );

	// geometry
	if ( model )
	{
		Debug::println( "    model = {0}", model->filename );
		ChunkUtil::writeStringChunk( out, "model", model->filename );
	}

	// level of detail
	Debug::println( "    lodMin = {0}, lodMax = {1}, lodID = {2}, lodNum = {3}", 
		lodMin, lodMax, lodID, lodNum );
	if ( -1 != lodNum )
	{
		out->beginChunk( "lod" );
		out->writeInt( lodNum );
		out->writeFloat( lodMin );
		out->writeFloat( lodMax );
		out->endChunk();
	}

	// bones
	if ( bones.size() > 0 )
	{
		Debug::println( "    bones = {0}", bones.size() );
		out->beginChunk( "bones" );
		out->writeInt( bones.size() );
		for ( int i = 0 ; i < bones.size() ; ++i )
		{
			const Bone& bone = bones[i];
			out->writeInt( bone.index );
			for ( int k = 0 ; k < 4 ; ++k )
				for ( int n = 0 ; n < 4 ; ++n )
					out->writeFloat( bone.rest(k,n) );
			//Debug::println( "      bone {0,#}: {1}", i, bone.index );
		}
		out->endChunk();
	}

	// shadow volumes
	if ( shadow )
	{
		if ( !shadow->dynamicShadow() )
		{
			shadow->projectVolumes();
			if ( shadow->silhuette().size() >= 6 && 
				shadow->volume().size() >= 3 )
			{
				out->beginChunk( "staticshadow" );
				
				ChunkUtil::writeTriangleList( out, shadow->silhuette() );
				Debug::println( "    static shadow silhuette = {0,#} triangles", shadow->silhuette().size()/3 );
				
				ChunkUtil::writeTriangleList( out, shadow->volume() );
				Debug::println( "    static shadow volume = {0,#} triangles", shadow->volume().size()/3 );

				Vector4 plane = shadow->capPlane();
				for ( int i = 0 ; i < 4 ; ++i )
					out->writeFloat( plane[i] );
				Debug::println( "    static shadow cap plane = {0} {1} {2} {3}", plane[0], plane[1], plane[2], plane[3] );

				Vector3 lt = shadow->lightWorld();
				ChunkUtil::writeFloatChunk3( out, "dir", lt.x, lt.y, lt.z );
				Debug::println( "    static shadow world direction = {0} {1} {2} {3}", lt.x, lt.y, lt.z );

				out->endChunk();
			}
		}
		else
		{
			if ( model )
			{
				out->beginChunk( "dynamicshadow" );
				out->writeString( shadow->modelName() );
				Vector3 lt = shadow->lightWorld();
				float len = shadow->shadowLength();
				ChunkUtil::writeFloatChunk4( out, "dir", lt.x, lt.y, lt.z, len );
				out->endChunk();
				Debug::println( "    dynamic shadow world direction = {0} {1} {2}", lt.x, lt.y, lt.z );
				Debug::println( "    dynamic shadow length = {0}", len );
			}
		}
	}

	out->endChunk();
}

bool SgMesh::isAnimated() const
{
	return SgNode::isAnimated() || bones.size() > 0;
}
