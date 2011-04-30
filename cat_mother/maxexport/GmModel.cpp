#include "StdAfx.h"
#include "GmModel.h"
#include "GmMaterial.h"
#include "ChunkUtil.h"
#include "Version.h"
#include "GmModelPrimitive.h"
#include <mb/Vertex.h>
#include <mb/Polygon.h>
#include <mb/VertexMap.h>
#include <mb/VertexMapFormat.h>
#include <mb/DiscontinuousVertexMap.h>
#include <io/IOException.h>
#include <io/ChunkOutputStream.h>
#include <dev/Profile.h>
#include <lang/Debug.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <util/Vector.h>
#include <algorithm>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace math;
using namespace util;

//-----------------------------------------------------------------------------

GmModel::GmModel() :
	node3ds(0),
	morphChannels( Allocator<P(GmMorphChannel)>(__FILE__,__LINE__) ),
	morphBase(0),
	materials( Allocator<P(GmMaterial)>(__FILE__,__LINE__) ),
	patches( Allocator<GmPatch>(__FILE__,__LINE__) ),
	primitives( Allocator<P(GmModelPrimitive)>(__FILE__,__LINE__) ),
	lineLists( Allocator<P(GmLineList)>(__FILE__) )
{
}

GmModel::~GmModel()
{
}

void GmModel::write( ChunkOutputStream* out )
{
	//Debug::println( "  (writing {0})", name );
	out->beginChunk( "gm" );
	out->writeInt( GM_FILE_VER );

	if ( !morphBase )
	{
		//Debug::println( "  (writing {0} materials)", name );
		// write used materials
		if ( materials.size() > 0 )
			writeMaterials( out );

		//Debug::println( "  (writing {0} patchlist)", name );
		// write patchlist
		if ( patches.size() > 0 )
			writePatchList( out );

		//Debug::println( "  (writing {0} models)", name );
		// write model primitives
		if ( primitives.size() > 0 )
			writeModelPrimitives( out );

		//Debug::println( "  (writing {0} morpher)", name );
		// write morpher
		if ( morphChannels.size() > 0 )
			writeMorpher( out );

		//Debug::println( "  (writing {0} line lists)", name );
		// write line lists
		if ( lineLists.size() > 0 )
			writeLineLists( out );
	}
	else
	{
		//Debug::println( "  (writing {0} morph targets)", name );
		// write morph target
		writeMorphTargets( out );
	}

	//Debug::println( "  (writing {0} done)", name );
	out->endChunk(); // gm
}

void GmModel::writeLineList( ChunkOutputStream* out, GmLineList* lineList )
{
	Debug::println( "Writing line list {0}", name );
	out->beginChunk( "linelist" );
	
	out->writeInt( lineList->points.size() );
	for ( int i = 0 ; i < lineList->points.size() ; ++i )
		ChunkUtil::writeVector3( out, lineList->points[i] );

	out->endChunk();
}

void GmModel::writeLineLists( ChunkOutputStream* out )
{
	for ( int i = 0 ; i < lineLists.size() ; ++i )
		writeLineList( out, lineLists[i] );
}

void GmModel::writeMorphTargets( ChunkOutputStream* out )
{
	require( morphBase );

	if ( morphBase->primitives.size() == 0 )
		throw Exception( Format("Morph base object {0} needs to have material set", morphBase->name) );

	Vector<int>		vertexIndices( Allocator<int>(__FILE__) );
	Vector<Vector3> vertexDeltas( Allocator<Vector3>(__FILE__) );

	for ( int i = 0 ; i < morphBase->primitives.size() ; ++i )
	{
		GmModelPrimitive* base = morphBase->primitives[i];
		if ( base->vertices.size() > 0 )
		{
			out->beginChunk( "morphtarget" );

			out->beginChunk( "info" );
			out->writeString( morphChannelName() );
			out->writeInt( 0 );
			out->endChunk();

			out->beginChunk( "material" );
			out->writeInt( base->matIndex );
			out->writeString( base->mat->name );
			out->endChunk();

			float maxDeltaLen = 0.f;
			getMorphDeltas( base, &vertexIndices, &vertexDeltas, &maxDeltaLen );
			out->beginChunk( "deltas" );
			out->writeInt( vertexDeltas.size() );
			out->writeFloat( maxDeltaLen );

			for ( int i = 0 ; i < vertexDeltas.size() ; ++i )
			{
				out->writeInt( vertexIndices[i] );
				ChunkUtil::writeVector3( out, vertexDeltas[i] );
			}
			out->endChunk();

			out->endChunk();
		}
	}
}

void GmModel::writeMorpher( ChunkOutputStream* out )
{
	require( morphChannels.size() == 0 || !morphBase );

	out->beginChunk( "morpher" );

	out->beginChunk( "info" );
	out->writeInt( morphChannels.size() );
	out->endChunk();

	for ( int i = 0 ; i < morphChannels.size() ; ++i )
	{
		const GmMorphChannel& chn = *morphChannels[i];
		out->beginChunk( "channel" );
		out->writeString( chn.name );
		out->writeString( chn.target->filename );
		ChunkUtil::writeAnim( out, chn.weightAnim );
		out->endChunk();
	}

	out->endChunk();
}

void GmModel::writeMaterials( ChunkOutputStream* out )
{
	for ( int i = 0 ; i < materials.size() ; ++i )
	{
		GmMaterial* mat = materials[i];
		mat->write( out );
	}
}

void GmModel::writeModelPrimitives( ChunkOutputStream* out )
{
	require( primitives.size() > 0 );
	for ( int i = 0 ; i < primitives.size() ; ++i )
		writeModelPrimitive( out, primitives[i] );
}

void GmModel::splitToPrimitives()
{
	// write one model primitive for each used material
	Vector<bool> vertexAdded( Allocator<bool>(__FILE__) );
	primitives.clear();
	for ( int i = 0 ; i < materials.size() ; ++i )
	{
		P(GmModelPrimitive) mp = new GmModelPrimitive;
		mp->mat = materials[i];
		mp->matIndex = i;

		// collect faces and unique vertices associated with the material
		int j;
		mp->vertices.clear();
		mp->polys.clear();
		vertexAdded.clear();
		vertexAdded.setSize( vertices(), false );
		for ( j = 0 ; j < polygons() ; ++j )
		{
			mb::Polygon* poly = getPolygon(j);

			if ( i == poly->material() )
			{
				mp->polys.add( poly );
				for ( int k = 0 ; k < poly->vertices() ; ++k )
				{
					mb::Vertex* v = poly->getVertex(k);
					if ( !vertexAdded[v->index()] )
					{
						vertexAdded[v->index()] = true;
						mp->vertices.add( v );
					}
				}
			}
		}

		if ( mp->mat && mp->vertices.size() >= 3 && mp->polys.size() > 0 )
		{
			// get texture coordinates, vertex colors, vertex normals
			mp->texlayers.clear();
			mp->vertcolor = 0;
			mp->normals = 0;
			for ( j = 0 ; j < discontinuousVertexMaps() ; ++j )
			{
				mb::DiscontinuousVertexMap* vmad = getDiscontinuousVertexMap( j );
				if ( vmad->format() == mb::VertexMapFormat::VERTEXMAP_TEXCOORD )
				{
					mp->texlayers.add( vmad );
				}
				else if ( vmad->format() == mb::VertexMapFormat::VERTEXMAP_RGB ||
					vmad->format() == mb::VertexMapFormat::VERTEXMAP_RGBA )
				{
					// vertex color
					require( !mp->vertcolor );
					mp->vertcolor = vmad;
				}
				else if ( vmad->format() == mb::VertexMapFormat::VERTEXMAP_NORMALS )
				{
					// vertex normals
					require( !mp->normals );
					require( vmad->dimensions() == 3 );
					mp->normals = vmad;
				}
			}

			// get weight maps
			mp->weightMaps.clear();
			for ( j = 0 ; j < vertexMaps() ; ++j )
			{
				mb::VertexMap* vmap = getVertexMap( j );
				if ( vmap->format() == mb::VertexMapFormat::VERTEXMAP_WEIGHT )
					mp->weightMaps.add( vmap );
			}

			// build vertex weight lists
			mp->maxWeightsPerVertex = 0;
			mp->vertexBoneIndices.clear();
			mp->vertexBoneWeights.clear();
			mp->vertexBoneCounts.clear();
			for ( j = 0 ; j < mp->vertices.size() ; ++j )
			{
				int weights = 0;
				for ( int k = 0 ; k < mp->weightMaps.size() ; ++k )
				{
					float w = 0.f;
					if ( mp->weightMaps[k]->getValue(mp->vertices[j]->index(), &w, 1) && w > 0.f )
					{
						mp->vertexBoneIndices.add( k );
						mp->vertexBoneWeights.add( w );
						++weights;
					}
				}
				if ( weights > mp->maxWeightsPerVertex )
					mp->maxWeightsPerVertex = weights;
				mp->vertexBoneCounts.add( weights );
			}

			// removed degenerate faces
			for ( int i = 0 ; i < mp->polys.size() ; )
			{
				mb::Polygon* poly = mp->polys[i];
				bool removePoly = false;

				// remove degenerate poly
				if ( poly->vertices() < 3 )
				{
					removePoly = true;
					Debug::printlnWarning( "Removed degenerate polygon ({1}) in {0}", name, poly->index() );
				}
				else
				{
					const int n = poly->vertices();
					int j = n-1;
					for ( int k = 0 ; k < n ; j = k++ )
					{
						mb::Vertex* v0 = poly->getVertex(j);
						mb::Vertex* v1 = poly->getVertex(k);

						// remove degenerate edge
						Vector3 pos0;
						v0->getPosition( &pos0.x, &pos0.y, &pos0.z );
						Vector3 pos1;
						v1->getPosition( &pos1.x, &pos1.y, &pos1.z );
						Vector3 edge = pos1 - pos0;
						if ( edge.lengthSquared() < 1e-12f )
						{
							removePoly = true;
							Debug::printlnWarning( "Removed degenerate polygon ({1}) edge in {0}", name, poly->index() );
						}

						// check for degenerate texture edge
						if ( mp->texlayers.size() > 0 )
						{
							Vector3 tc0(0,0,0);
							Vector3 tc1(0,0,0);
							mp->texlayers[0]->getValue( v0->index(), poly->index(), tc0.begin(), 2 );
							mp->texlayers[0]->getValue( v1->index(), poly->index(), tc1.begin(), 2 );
							Vector3 tedge = tc1 - tc0;
							if ( tedge.lengthSquared() < 1e-12f )
							{
								//removePoly = true;
								Debug::printlnWarning( "Degenerate texture polygon ({5}) edge in {0}: ({1}, {2}) - ({3}, {4})", name, tc0.x, tc0.y, tc1.x, tc1.y, poly->index() );
							}
						}
						else
						{
							removePoly = true;
						}
					}
				}

				if ( removePoly )
					mp->polys.remove( i );
				else
					++i;
			}

			if ( mp->polys.size() > 0 )
				primitives.add( mp );
		}
	}
}

void GmModel::writeModelPrimitive( ChunkOutputStream* out, GmModelPrimitive* mp )
{
	require( mp->mat );

	// write model
	Debug::println( "  Writing geometry using material {0}", mp->mat->name );
	out->beginChunk( "model" );

	// limit number of texture coordinate layers
	int texcoords = mp->texlayers.size();
	if ( texcoords > 1 && 0 == (mp->mat->layerFlags() & GmMaterial::L_LGHT) )
	{
		Debug::printlnWarning( "GmModel.writeModelPrimitive: {0} texture coordinates layers in {1} without lightmap", mp->texlayers.size(), name );
		texcoords = 1;
	}
	if ( texcoords > 2 && 0 != (mp->mat->layerFlags() & GmMaterial::L_LGHT) )
	{
		Debug::printlnWarning( "GmModel.writeModelPrimitive: {0} texture coordinates layers in {1}", mp->texlayers.size(), name );
		texcoords = 2;
	}

	// write info chunk
	int vertcolorDim = ( mp->vertcolor ? mp->vertcolor->dimensions() : 0 );
	bool needsVertexTangentSpaceU = (mp->mat->layerFlags() & GmMaterial::L_BUMP) != 0;
	Debug::println( "    mp->vertices = {0,#}", mp->vertices.size() );
	Debug::println( "    polygons = {0,#}", mp->polys.size() );
	Debug::println( "    mp->texcoords = {0,#}", texcoords );
	Debug::println( "    mp->maxWeightsPerVertex = {0,#}", mp->maxWeightsPerVertex );
	Debug::println( "    vertexcolorsDim = {0,#}", vertcolorDim );
	out->beginChunk( "info" );
	out->writeInt( mp->vertices.size() );
	out->writeInt( mp->polys.size() );
	out->writeInt( mp->maxWeightsPerVertex );
	out->writeInt( texcoords );
	for ( int j = 0 ; j < texcoords ; ++j )
		out->writeInt( mp->texlayers[j]->dimensions() );
	out->writeInt( vertcolorDim );
	out->writeInt( needsVertexTangentSpaceU );
	out->endChunk(); // info

	// check that we have texturecoordinates if we need tangent space
	if ( 0 != (mp->mat->layerFlags() & (GmMaterial::L_DIFF|GmMaterial::L_LGHT|GmMaterial::L_GLOS|GmMaterial::L_BUMP)) && 
		texcoords == 0 )
	{
		throw Exception( Format("Object {0} material {1} requires texture coordinates but object does not have texture coordinates.", name, mp->mat->name) );
	}

	// write material ref chunk
	ChunkUtil::writeIntChunk( out, "material", mp->matIndex );

	// write points
	out->beginChunk( "points" );
	for ( int j = 0 ; j < mp->vertices.size() ; ++j )
	{
		float v[3];
		mp->vertices[j]->getPosition( &v[0], &v[1], &v[2] );
		for ( int k = 0 ; k < 3 ; ++k )
			out->writeFloat( v[k] );
	}
	out->endChunk(); // points

	// write faces
	out->beginChunk( "faces" );
	for ( int j = 0 ; j < mp->polys.size() ; ++j )
	{
		for ( int k = 0 ; k < 3 ; ++k )
		{
			mb::Vertex* v = mp->polys[j]->getVertex(k);
			mb::Vertex** vit = std::find( mp->vertices.begin(), mp->vertices.end(), v );
			require( vit != mp->vertices.end() && *vit == v );
			int vi = vit - mp->vertices.begin();
			out->writeInt( vi );
		}
	}
	out->endChunk(); // faces

	// write vertex mp->normals
	if ( mp->normals )
		writeDVMap( out, "vertexnormalsf", mp->normals, mp->vertices );

	// write vertex colors
	if ( vertcolorDim > 0 )
		writeDVMapByteValues( out, "vertexcolors", mp->vertcolor, mp->vertices );

	// write texcoord layers
	for ( int layer = 0 ; layer < texcoords ; ++layer )
		writeDVMap( out, "texcoordlayer", mp->texlayers[layer], mp->vertices );

	// write skin
	if ( mp->maxWeightsPerVertex > 0 )
	{
		out->beginChunk( "skin" );
		out->writeInt( mp->weightMaps.size() );
		
		// write weight map / bone names
		for ( int j = 0 ; j < mp->weightMaps.size() ; ++j )
			out->writeString( mp->weightMaps[j]->name() );

		// write vertex weights
		int weightIndex = 0;
		for ( int j = 0 ; j < mp->vertices.size() ; ++j )
		{
			int weights = mp->vertexBoneCounts[j];
			out->writeInt( weights );
			for ( int k = 0 ; k < weights ; ++k )
			{
				out->writeInt( mp->vertexBoneIndices[weightIndex] );
				out->writeFloat( mp->vertexBoneWeights[weightIndex] );
				++weightIndex;
			}
		}

		out->endChunk(); // skin
	}

	out->endChunk(); // model
}

void GmModel::writeDVMap( ChunkOutputStream* out, const String& chunkName, mb::DiscontinuousVertexMap* vmad, const util::Vector<mb::Vertex*>& vertices )
{
	int dim = vmad->dimensions();
	require( dim <= 8 );

	out->beginChunk( chunkName );
	for ( int j = 0 ; j < vertices.size() ; ++j )
	{
		mb::Vertex* vert = vertices[j];
		require( vert->getPolygon(0) );

		float uv[8];
		int k;
		for ( k = 0 ; k < vmad->dimensions() ; ++k )
			uv[k] = 0.f;

		bool found = vmad->getValue( vert->index(), vert->getPolygon(0)->index(), uv, dim );
		//require( found );

		for ( k = 0 ; k < vmad->dimensions() ; ++k )
			out->writeFloat( uv[k] );
	}
	out->endChunk();
}

void GmModel::writeDVMapByteValues( ChunkOutputStream* out, const String& chunkName, mb::DiscontinuousVertexMap* vmad, const util::Vector<mb::Vertex*>& vertices )
{
	int dim = vmad->dimensions();
	require( dim <= 8 );

	out->beginChunk( chunkName );
	for ( int j = 0 ; j < vertices.size() ; ++j )
	{
		mb::Vertex* vert = vertices[j];
		require( vert->getPolygon(0) );

		float uv[8];
		int k;
		for ( k = 0 ; k < vmad->dimensions() ; ++k )
			uv[k] = 0.f;

		bool found = vmad->getValue( vert->index(), vert->getPolygon(0)->index(), uv, dim );
		require( found );

		for ( k = 0 ; k < vmad->dimensions() ; ++k )
		{
			float v = uv[k] * 255.f;
			if ( v < 0.f )
				v = 0.f;
			else if ( v > 255.f )
				v = 255.f;
			out->writeByte( (int)(v+.5f) );
		}
	}
	out->endChunk();
}

bool GmModel::operator==( const GmModel& other ) const
{
	if ( materials.size() != other.materials.size() )
		return false;
	if ( patches.size() != other.patches.size() )
		return false;
	if ( this->MeshBuilder::operator!=(other) )
		return false;
	for ( int i = 0 ; i < materials.size() ; ++i )
		if ( *materials[i] != *other.materials[i] )
			return false;
	if ( this->lineLists.size() != other.lineLists.size() )
		return false;
	for ( int i = 0 ; i < lineLists.size() ; ++i )
		if ( *lineLists[i] != *other.lineLists[i] )
			return false;
	return true;
}

void GmModel::writePatchList( ChunkOutputStream* out )
{
	out->beginChunk( "patchlist" );
	
	// write info chunk
	out->beginChunk( "info" );
	out->writeInt( patches.size() );
	out->writeInt( 4 );
	out->endChunk();

	// write patch material (always the first, see GmUtil)
	ChunkUtil::writeIntChunk( out, "material", 0 );

	// write patch points
	out->beginChunk( "points" );
	for ( int k = 0 ; k < patches.size() ; ++k )
	{
		for ( int i = 0 ; i < 4 ; ++i )
			for ( int j = 0 ; j < 4 ; ++j )
				ChunkUtil::writeVector3( out, patches[k].getControlPoint(i,j) );
	}
	out->endChunk();

	out->endChunk();
}

String GmModel::morphChannelName() const
{
	require( morphBase );

	for ( int i = 0 ; i < morphBase->morphChannels.size() ; ++i )
	{
		if ( morphBase->morphChannels[i]->target == this )
			return morphBase->morphChannels[i]->name;
	}

	require( false );
	return "";
}

void GmModel::getMorphDeltas( GmModelPrimitive* base, Vector<int>* vertexIndices, Vector<Vector3>* vertexDeltas, float* maxDeltaLen )
{
	require( morphBase );

	vertexIndices->clear();
	vertexDeltas->clear();
	*maxDeltaLen = 0.f;
	for ( int i = 0 ; i < base->vertices.size() ; ++i )
	{
		// base vertex
		Vector3 v0;
		base->vertices[i]->getPosition( &v0.x, &v0.y, &v0.z );

		// target vertex
		Vector3 v1;
		int vi = base->vertices[i]->index();
		require( vi >= 0 && vi < vertices() );
		getVertex( vi )->getPosition( &v1.x, &v1.y, &v1.z );

		// delta
		Vector3 delta = v1 - v0;
		if ( delta.length() > 1e-9f )
		{
			if ( delta.length() > *maxDeltaLen )
				*maxDeltaLen = delta.length();

			vertexIndices->add( i );
			vertexDeltas->add( delta );
		}
	}

	require( vertexIndices->size() == vertexDeltas->size() );
}
