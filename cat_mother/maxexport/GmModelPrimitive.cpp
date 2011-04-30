#include "StdAfx.h"
#include "GmModelPrimitive.h"
#include "GmMaterial.h"

//-----------------------------------------------------------------------------

using namespace util;

//-----------------------------------------------------------------------------

GmModelPrimitive::GmModelPrimitive() :
	vertices( Allocator<mb::Vertex*>(__FILE__) ),
	polys( Allocator<mb::Polygon*>(__FILE__) ),
	mat(0),
	matIndex(-1),
	texlayers( Allocator<mb::DiscontinuousVertexMap*>(__FILE__) ),
	vertcolor(0),
	normals(0),
	weightMaps( Allocator<mb::VertexMap*>(__FILE__) ),
	maxWeightsPerVertex(0),
	vertexBoneIndices( Allocator<int>(__FILE__) ),
	vertexBoneWeights( Allocator<float>(__FILE__) ),
	vertexBoneCounts( Allocator<int>(__FILE__) )
{
	
}

GmModelPrimitive::~GmModelPrimitive()
{
}
