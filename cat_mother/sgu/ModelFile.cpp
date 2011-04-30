#include "ModelFile.h"
#include "ChunkUtil.h"
#include "ModelFileCache.h"
#include <io/File.h>
#include <io/ChunkInputStream.h>
#include <io/IOException.h>
#include <io/DataInputStream.h>
#include <io/InputStreamArchive.h>
#include <sg/Model.h>
#include <sg/LineList.h>
#include <sg/Effect.h>
#include <sg/Morpher.h>
#include <sg/Texture.h>
#include <sg/CubeTexture.h>
#include <sg/Material.h>
#include <sg/PatchList.h>
#include <sg/MorphTarget.h>
#include <sg/VertexAndIndexLock.h>
#include <sg/VertexFormat.h>
#include <pix/Color.h>
#include <pix/Colorf.h>
#include <lang/Debug.h>
#include <lang/String.h>
#include <math/Vector3.h>
#include <math/Matrix4x4.h>
#include <util/Vector.h>
#include <anim/VectorInterpolator.h>
#include <algorithm>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define GM_FILE_VER 0x100

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;

//-----------------------------------------------------------------------------

namespace sgu
{


class ModelFile::ModelFileImpl :
	public lang::Object
{
public:
	String						name;
	Vector< P(Primitive) >		primitives;

	ModelFileImpl( const String& modelName,
		const String* boneNames, int bones,
		const Colorf& ambientColor,
		InputStreamArchive* arch, 
		ModelFileCache* modelFileCache,
		int loadFlags ) :
		name( modelName ),
		primitives( Allocator<P(Primitive)>(__FILE__,__LINE__) ),
		m_arch( arch ),
		m_meshBones( bones ),
		m_meshBoneNames( boneNames ),
		m_modelCache( modelFileCache ),
		m_loadFlags( loadFlags ),
		m_ambient( ambientColor ),
		m_path( File(modelName).getParent() ),
		m_materials( Allocator<P(Shader)>(__FILE__,__LINE__) )
	{
		assert( arch );

		P(InputStream) in = m_arch->getInputStream( name );
		ChunkInputStream reader( in );
		String chunk;
		long end;
		reader.beginChunk( &chunk, &end );
		if ( chunk != "gm" )
			throw IOException( Format("Invalid header in geometry file: {0}",name) );
		readMain( &reader, end );
		reader.endChunk( end );

		// reset temporaries
		in->close();
		m_arch = 0;
		m_meshBones = 0;
		m_meshBoneNames = 0;
		m_modelCache = 0;

		// stats
		int verts = 0;
		int faces = 0;
		for ( int i = 0 ; i < primitives.size() ; ++i )
		{
			Primitive* prim = primitives[i];
			Model* model = dynamic_cast<Model*>( prim );
			if ( model )
			{
				verts += model->vertices();
				faces += model->indices() / 3;
			}
		}
	}

	void readMain( ChunkInputStream* in, long end )
	{
		String subname;
		long subend;

		int ver = in->readInt();
		if ( ver != GM_FILE_VER )
			throw IOException( Format("Invalid geometry file version (expected {1,x}, got {1,x}): {0}", name, GM_FILE_VER, ver) );

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );
			
			if ( subname == "material" && (m_loadFlags&ModelFile::LOAD_GEOMETRY) )
				readMaterial( in, subend );
			if ( subname == "effect" && (m_loadFlags&ModelFile::LOAD_GEOMETRY) )
				readEffect( in, subend );
			else if ( subname == "model" && (m_loadFlags&ModelFile::LOAD_GEOMETRY) )
				readModel( in, subend );
			else if ( subname == "patchlist" && (m_loadFlags&ModelFile::LOAD_GEOMETRY) )
				readPatchList( in, subend );
			else if ( subname == "morphtarget" && (m_loadFlags&ModelFile::LOAD_MORPH) )
				readMorphTarget( in, subend );
			else if ( subname == "morpher" && (m_loadFlags&ModelFile::LOAD_MORPH) )
				readMorpher( in, subend );
			else if ( subname == "linelist" && (m_loadFlags&ModelFile::LOAD_GEOMETRY) )
				readLineList( in, subend );

			in->endChunk( subend );
		}
	}

	void readMorpher( ChunkInputStream* in, long end )
	{
		String subname;
		long subend;

		//Debug::println( "Reading morph animation {0}", name );

		// read info chunk
		in->beginChunk( &subname, &subend );
		if ( subname != "info" )
			throw IOException( Format("The geometry file morpher chunk must begin with info subchunk: {0}",name) );

		// read basic info
		int channels = in->readInt();

		// check basic info
		if ( channels < 1 )
			throw IOException( Format("Morpher must have at least one channel: {0}", name) );

		in->endChunk( subend );

		// read subchunks
		int channelsRead = 0;

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );

			if ( subname == "channel" )
			{
				String chnName = in->readString();
				String modelFilename = in->readString();
				P(VectorInterpolator) weightAnim = new VectorInterpolator(1);
				ChunkUtil::readAnim( in, weightAnim, "morph weight" );

				P(ModelFile) targetFile = m_modelCache->getByName( File(m_path,modelFilename).getPath(), m_meshBoneNames, m_meshBones, m_ambient, ModelFile::LOAD_MORPH );
				for ( int i = 0 ; i < targetFile->primitives() ; ++i )
				{
					MorphTarget* target = dynamic_cast<MorphTarget*>( targetFile->getPrimitive(i) );
					if ( target && target->name() == chnName )
					{
						// try to find existing morpher using same material
						P(Morpher) morpher = 0;
						for ( int i = 0 ; i < primitives.size() ; ++i )
						{
							P(Morpher) mp = dynamic_cast<Morpher*>( primitives[i].ptr() );
							if ( mp )
							{
								assert( mp->targets() > 0 ); // every added morpher must have one target
								if ( mp->getTarget(0)->materialName() == target->materialName() )
								{
									morpher = mp;
									break;
								}
							}
						}

						// create new morpher
						if ( !morpher )
						{
							for ( int i = 0 ; i < primitives.size() ; ++i )
							{
								P(Model) base = dynamic_cast<Model*>( primitives[i].ptr() );
								if ( base && base->shader()->name() == target->materialName() )
								{
									morpher = new Morpher;
									morpher->setBase( base );
									primitives.remove( i );
									primitives.add( morpher.ptr() );
									break;
								}
							}

							if ( !morpher )
							{
								if ( 0 != (m_loadFlags & LOAD_GEOMETRY) )
								{
									throw IOException( Format("Invalid morpher base model in {0}", name) );
								}
								else
								{
									morpher = new Morpher;
									primitives.add( morpher.ptr() );
								}
							}
						}

						// add channel to the morpher
						morpher->addTarget( target );
						morpher->setTargetWeightController( chnName, weightAnim );
					}
				}

				++channelsRead;
			}

			if ( in->size() > subend )
				throw IOException( Format("Chunk read overflow in morpher chunk of a geometry file: {0}",name) );
			in->endChunk( subend );
		}

		// check subchunk read ok
		if ( channelsRead != channels )
			throw IOException( Format("Incorrect number of channel subchunks ({1}, should be {2}) in morpher chunk of a geometry file: {0}",name,channelsRead,channels) );
	}

	void readMorphTarget( ChunkInputStream* in, long end )
	{
		String subname;
		long subend;

		// read info chunk
		in->beginChunk( &subname, &subend );
		if ( subname != "info" )
			throw IOException( Format("The geometry file morphtarget chunk must begin with info subchunk: {0}",name) );

		// read basic info
		String	chnName	= in->readString();
		int		type	= in->readInt();

		// check basic info
		if ( type != 0 )
			throw IOException( Format("Invalid morphtarget type ({1}) in geometry file: {0}", name, type) );
		if ( chnName == "" )
			throw IOException( Format("Invalid morphtarget channel name ({1}) in geometry file: {0}", name, chnName) );

		in->endChunk( subend );
		
		// create the morphtarget
		P(MorphTarget) morphtarget = new MorphTarget;
		morphtarget->setName( chnName );

		// read subchunks
		bool deltasRead		= false;
		bool materialRead	= false;
		String materialName;

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );

			if ( subname == "material" )
			{
				in->readInt(); // skip materials -- morph targets are not rendered directly
				materialName = in->readString();
				morphtarget->setMaterialName( materialName );
				materialRead = true;
			}
			else if ( subname == "deltas" )
			{
				int count = in->readInt();
				//if ( count == 0 )
				//	Debug::printlnWarning( "Model {0} has has no deltas in channel {1} (material {2})", name, chnName, materialName );
				const float maxDeltaLen = in->readFloat();
				for ( int i = 0 ; i < count ; ++i )
				{
					int vertexIndex = in->readInt();
					float dx = in->readFloat();
					float dy = in->readFloat();
					float dz = in->readFloat();
					morphtarget->addDelta( vertexIndex, Vector3(dx,dy,dz), maxDeltaLen );
				}
				deltasRead = true;
			}

			if ( in->size() > subend )
				throw IOException( Format("Chunk read overflow in morphtarget chunk of a geometry file: {0}",name) );
			in->endChunk( subend );
		}

		// check subchunk read ok
		if ( !deltasRead )
			throw IOException( Format("Missing deltas subchunk in morphtarget chunk of a geometry file: {0}",name) );
		if ( !materialRead )
			throw IOException( Format("Missing material subchunk in morphtarget chunk of a geometry file: {0}",name) );

		primitives.add( morphtarget.ptr() );
	}

	void readLineList( ChunkInputStream* in, long /*end*/ )
	{
		int points = in->readInt();

		if ( points < 2 )
			throw IOException( Format("Too few points in linelist in {0}", name) );

		P(LineList) linelist = new LineList( points, LineList::LINES_3D );
		VertexLock<LineList> lk( linelist, LineList::LOCK_WRITE );

		Vector3 firstpt(0,0,0);
		Vector3 prevpt(0,0,0);
		for ( int i = 0 ; i <= points ; ++i )
		{
			Vector3 pt;
			if ( i < points )
			{
				for ( int k = 0 ; k < 3 ; ++k )
					pt[k] = in->readFloat();
			}
			else
			{
				pt = firstpt;
			}

			if ( i == 0 )
				firstpt = pt;
			
			if ( i > 0 )
				linelist->addLine( prevpt, pt, Color(255,0,255) );

			prevpt = pt;
		}

		primitives.add( linelist.ptr() );
	}

	void readPatchList( ChunkInputStream* in, long end )
	{
		String subname;
		long subend;

		// read info chunk
		in->beginChunk( &subname, &subend );
		if ( subname != "info" )
			throw IOException( Format("The geometry file patchlist chunk must begin with info subchunk: {0}",name) );

		// read basic info
		int		patches				= in->readInt();

		// check basic info
		if ( patches < 1 )
			throw IOException( Format("Invalid patch count ({1}) in geometry file: {0}", name, patches) );

		in->endChunk( subend );
		
		// create the patchlist
		P(PatchList) patchlist = new PatchList( patches*16 );
		VertexLock<PatchList> lock( patchlist, PatchList::LOCK_READWRITE );

		// read subchunks
		bool pointsRead		= false;
		bool materialRead	= false;

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );

			if ( subname == "material" )
			{
				int ix = in->readInt();
				if ( ix < 0 || ix >= (int)m_materials.size() )
					throw IOException( Format("Undefined material index ({1,#}) used in the geometry file: {0}", name, ix) );
				
				Shader* mat = m_materials[ix];
				mat->setVertexFormat( patchlist->vertexFormat() );
				patchlist->setShader( mat );
				materialRead = true;
			}
			else if ( subname == "points" )
			{
				Vector3 points[4][4];
				for ( int k = 0 ; k < patches ; ++k )
				{
					for ( int i = 0 ; i < 4 ; ++i )
					{
						for ( int j = 0 ; j < 4 ; ++j )
						{
							for ( int d = 0 ; d < 3 ; ++d )
								points[i][j][d] = in->readFloat();
						}
					}
					patchlist->setVertexPositions( k*16, &points[0][0], 4*4 );
				}
				pointsRead = true;
			}

			if ( in->size() > subend )
				throw IOException( Format("Chunk read overflow in patchlist chunk of a geometry file: {0}",name) );
			in->endChunk( subend );
		}

		// check subchunk read ok
		if ( !pointsRead )
			throw IOException( Format("Missing points subchunk in patchlist chunk of a geometry file: {0}",name) );
		if ( !materialRead )
			throw IOException( Format("Missing material subchunk in patchlist chunk of a geometry file: {0}",name) );

		primitives.add( patchlist.ptr() );
	}
	
	void readModel( ChunkInputStream* in, long end )
	{
		String subname;
		long subend;
		
		// read info chunk
		in->beginChunk( &subname, &subend );
		if ( subname != "info" )
			throw IOException( Format("The geometry file model chunk must begin with info subchunk: {0}",name) );

		// read basic info
		int		vertices			= in->readInt();
		int		triangles			= in->readInt();
		int		weightsPerVertex	= in->readInt();
		int		texCoordLayers		= in->readInt();

		// check basic info
		if ( vertices < 1 )
			throw IOException( Format("Invalid vertex count ({1}) in geometry file: {0}", name, vertices) );
		if ( vertices > 65534 )
			throw IOException( Format("Too many vertices ({1}) in a single batch: {0}", name, vertices) );
		if ( triangles < 0 )
			throw IOException( Format("Invalid triangle count ({1}) in geometry file: {0}", name, triangles) );
		if ( weightsPerVertex < 0 )
			throw IOException( Format("Invalid per vertex weight count ({1}) in geometry file: {0}", name, weightsPerVertex) );
		//if ( texCoordLayers < 1 || texCoordLayers > 4 )
		//	throw IOException( Format("Invalid texture coordinate layer count ({1}) in geometry file: {0}", name, texCoordLayers) );

		// read texcoord layer sizes
		int texCoordSizes[ 8 ];
		for ( int i = 0 ; i < texCoordLayers ; ++i )
		{
			texCoordSizes[i] = in->readInt();
			if ( texCoordSizes[i] < 1 || texCoordSizes[i] > 4 )
				throw IOException( Format("Invalid texture coordinate layer dimension ({1}) in geometry file: {0}", name, texCoordSizes[i]) );
		}

		// read vertex color dimension
		int vertexColorDim = 0;
		if ( in->size() < subend )
			vertexColorDim = in->readInt();
		if ( 0 != vertexColorDim && 3 != vertexColorDim && 4 != vertexColorDim )
			throw IOException( Format( "Invalid vertex color size ({1}) in geometry file: {0}", name, vertexColorDim) );
		bool lit = (0 == vertexColorDim);

		// read if tangent space U-axis is needed (for bump mapping)
		bool needsTangentSpaceU = false;
		if ( in->size() < subend )
			needsTangentSpaceU = in->readInt() != 0;

		in->endChunk( subend );

		// check for texcoord count limits
		if ( texCoordLayers + 
			(needsTangentSpaceU?1:0) + 
			(weightsPerVertex>0?2:0) > VertexFormat::MAX_LAYERS )
		{
			throw IOException( Format("Too many texture coordinate layers in {0}", name) );
		}

		// find out vertex format
		VertexFormat vf;
		vf.setWeights( weightsPerVertex );
		for ( int i = 0 ; i < texCoordLayers ; ++i )
			vf.addTextureCoordinate( texCoordSizes[i] );
		if ( vertexColorDim > 0 )
			vf.addDiffuse();
		vf.addNormal();
		int tangentLayer = -1;
		if ( needsTangentSpaceU )
		{
			tangentLayer = vf.textureCoordinates();
			vf.addTextureCoordinate( 3 );
		}

		// create the model
		P(Model) model = new Model( vertices, triangles*3, vf );
		if ( vf.textureCoordinates() > model->vertexFormat().textureCoordinates() )
			throw IOException( Format("Too many texture coordinates in {0}", name) );
		vf = model->vertexFormat();
		VertexAndIndexLock<Model> lock( model, Model::LOCK_READWRITE );

		// read subchunks
		int texCoordLayer	= 0;
		bool materialRead	= false;
		bool pointsRead		= false;
		bool facesRead		= false;
		bool skinRead		= false;
		bool normalsRead	= false;

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );

			if ( subname == "material" )
			{
				int ix = in->readInt();
				if ( ix < 0 || ix >= (int)m_materials.size() )
					throw IOException( Format("Undefined material (index {1,#}) used in the geometry file: {0}", name, ix) );
				
				P(Shader) mat = m_materials[ix]->clone();
				mat->setVertexFormat( model->vertexFormat() );
				model->setShader( mat );
				materialRead = true;

				Material* mtl = dynamic_cast<Material*>( mat.ptr() );
				if ( mtl )
				{
					// enable vertex color usage if needed
					if ( vertexColorDim > 0 )
					{
						mtl->setVertexColor( true );
						mtl->setEmissiveColorSource( Material::MCS_COLOR1 );
					}

					// disable lighting if the mesh is unlit
					if ( !lit ) 
						mtl->setLighting( false );
				}
			}
			else if ( subname == "points" )
			{
				for ( int i = 0 ; i < vertices ; ++i )
				{
					Vector3 v;
					for ( int k = 0 ; k < 3 ; ++k )
						v[k] = in->readFloat();
					model->setVertexPositions( i, &v );
				}
				pointsRead = true;
			}
			else if ( subname == "vertexnormalsf" )
			{
				if ( vf.hasNormal() )
				{
					for ( int i = 0 ; i < vertices ; ++i )
					{
						Vector3 v;
						for ( int k = 0 ; k < 3 ; ++k )
							v[k] = in->readFloat();
						model->setVertexNormals( i, &v );
					}
				}
				normalsRead = true;
			}
			else if ( subname == "faces" )
			{
				int triangleindex = 0;
				for ( int i = 0 ; i < triangles ; ++i )
				{
					int indices[3];
					for ( int k = 0 ; k < 3 ; ++k )
					{
						int ix = in->readInt();
						if ( ix < 0 || ix >= vertices )
							throw IOException( Format("Invalid vertex index ({1,#}) in a geometry file: {0}", name, ix) );
						indices[k] = ix;
					}
					model->setIndices( triangleindex, indices, 3 );
					triangleindex += 3;
				}
				facesRead = true;
			}
			else if ( subname == "vertexcolors" )
			{
				if ( vf.hasDiffuse() )
				{
					int dim = vertexColorDim;
					assert( dim > 0 && dim <= 4 );

					// base color for vertices (=emissive+ambient)
					Colorf vertexBaseColor(0,0,0);
					Material* mat = dynamic_cast<Material*>( model->shader() );
					float alpha = 1.f;
					if ( mat )
					{
						alpha = mat->diffuseColor().alpha();
						vertexBaseColor.setAlpha( 0 );
					}

					for ( int i = 0 ; i < vertices ; ++i )
					{
						float v[4] = {1.f,1.f,1.f,alpha};

						for ( int i = 0 ; i < dim ; ++i )
						{
							v[i] = in->readByte();
							v[i] *= 1.f / 255.f;
						}

						Colorf color = vertexBaseColor + Colorf( v[0], v[1], v[2], v[3] );
						Color vcolor( color );
						model->setVertexDiffuseColors( i, &vcolor, 1 );
					}
				}
			}
			else if ( subname == "texcoordlayer" )
			{
				if ( texCoordLayer < vf.textureCoordinates() )
				{
					float tc[8];
					int dim = texCoordSizes[texCoordLayer];
					if ( dim == vf.getTextureCoordinateSize(texCoordLayer) )
					{
						for ( int i = 0 ; i < vertices ; ++i )
						{
							for ( int k = 0 ; k < dim ; ++k )
								tc[k] = in->readFloat();
							model->setVertexTextureCoordinates( i, texCoordLayer, dim, tc );
						}
					}
					++texCoordLayer;
				}
			}
			else if ( subname == "skin" )
			{
				if ( vf.weights() > 0 )
				{
					// read affecting bones
					const int MAX_BONES = 256;
					int bones = in->readInt();
					if ( bones > MAX_BONES )
						throw IOException( Format("Too many bones ({0,#}) in a geometry file: {0}", name, bones) );
					String boneNames[MAX_BONES];
					for ( int i = 0 ; i < bones ; ++i )
						boneNames[i] = in->readString();

					// build mapping from affecting bone to mesh bone index
					int meshBoneIndices[MAX_BONES];
					const String* meshBoneNamesEnd = m_meshBoneNames+m_meshBones;
					for ( int i = 0 ; i < bones ; ++i )
					{
						const String* meshBone = std::find( m_meshBoneNames, meshBoneNamesEnd, boneNames[i] );
						if ( meshBone == meshBoneNamesEnd )
							throw IOException( Format("Model file {0} has weights defined for bone {1} but the mesh does not have a such bone.", name, boneNames[i]) );
						meshBoneIndices[i] = meshBone-m_meshBoneNames;
					}

					// read weights
					const int	maxBonesPerVertex = MAX_BONES;
					int			boneIndices[maxBonesPerVertex];
					float		boneWeights[maxBonesPerVertex];

					for ( int i = 0 ; i < vertices ; ++i )
					{
						int vertexBones = in->readInt();
						int usedBones = vertexBones;
						if ( usedBones > maxBonesPerVertex )
							usedBones = maxBonesPerVertex;

						for ( int k = 0 ; k < vertexBones ; ++k )
						{
							int bi = in->readInt();
							float bw = in->readFloat();
							if ( bi < 0 || bi >= bones )
								throw IOException( Format("Invalid bone index ({1,#}) in geometry file: {0}", name, bi) );
							if ( k < maxBonesPerVertex )
							{
								boneIndices[k] = 1+meshBoneIndices[bi];
								boneWeights[k] = bw;
							}
						}

						usedBones = Model::sortVertexWeights( boneIndices, boneWeights, usedBones, boneIndices, boneWeights );
						model->setVertexWeights( i, boneIndices, boneWeights, usedBones );
					}

					model->optimizeBoneIndices();
					Debug::println( "Geometry {0} using material {1} uses {2} bones.", name, model->shader() ? model->shader()->name() : "(none)", model->usedBones() );
				}
				skinRead = true;
			}

			if ( in->size() > subend )
				throw IOException( Format("Chunk read overflow in model chunk of a geometry file: {0}",name) );
			in->endChunk( subend );
		}

		// check subchunk read ok
		if ( texCoordLayer != texCoordLayers )
			throw IOException( Format("Missing texture coordinate layer subchunk in model chunk of a geometry file: {0}",name) );
		if ( !materialRead )
			throw IOException( Format("Missing material subchunk in model chunk of a geometry file: {0}",name) );
		if ( !pointsRead )
			throw IOException( Format("Missing points subchunk in model chunk of a geometry file: {0}",name) );
		if ( !facesRead )
			throw IOException( Format("Missing faces subchunk in model chunk of a geometry file: {0}",name) );
		if ( !skinRead && weightsPerVertex > 0 )
			throw IOException( Format("Missing skin subchunk in model chunk of a geometry file: {0}",name) );

		// check for degenerate polygons
		bool once1 = true;
		bool once2 = true;
		const int indices = model->indices();
		for ( int i = 0 ; i < indices ; i += 3 )
		{
			const int n = 3;

			int ind[n];
			model->getIndices( i, ind, n );

			Vector3 v[n];
			Vector4 texc[n];
			for ( int k = 0 ; k < n ; ++k )
			{
				model->getVertexPositions( ind[k], &v[k] );
				texc[k] = Vector4(0,0,0,0);
				if ( texCoordLayers > 0 )
					model->getVertexTextureCoordinates( ind[k], 0, texCoordSizes[0], texc[k].begin() );
			}

			int j = n-1;
			for ( int k = 0 ; k < n ; j = k++ )
			{
				Vector3 edge = v[k] - v[j];
				if ( edge.length() < 1e-9f && once1 )
				{
					Debug::printlnWarning( "Degenerate polygon {0} in model {1} using material {2}", i/3, name, model->shader()->name() );
					once1 = false;
					break;
				}

				if ( texCoordLayers > 0 )
				{
					Vector4 tedge = texc[k] - texc[j];
					if ( tedge.length() < 1e-9f && once2 )
					{
						Debug::printlnWarning( "Degenerate texture polygon {0} in model {1} using material {2}", i/3, name, model->shader()->name() );
						once2 = false;
						break;
					}
				}
			}
		}

		// generate vertex normals if needed
		if ( !normalsRead && vf.hasNormal() )
			model->computeVertexNormals();

		// generate vertex tangents if needed
		if ( needsTangentSpaceU )
		{
			if ( vf.textureCoordinates() < 2 ||
				vf.getTextureCoordinateSize(tangentLayer) != 3 || 
				vf.getTextureCoordinateSize(0) < 2 ||
				!vf.hasNormal() )
			{
				throw IOException( Format("Invalid tangent space vertex format in {0}", name) );
			}
			model->computeVertexTangents( 0, tangentLayer );
		}

		primitives.add( model.ptr() );
	}

	void readMaterial( ChunkInputStream* in, long end )
	{
		String subname;
		long subend;

		P(Material) mat = new Material;
		subname = in->readString();
		mat->setName( subname );
		int layers = 0;

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );

			if ( subname == "blend" )
			{
				Material::BlendMode src = (Material::BlendMode)in->readInt();
				Material::BlendMode dst = (Material::BlendMode)in->readInt();
				mat->setBlend( src, dst );
			}
			else if ( subname == "zwrite" )
			{
				bool on = (in->readInt() != 0);
				mat->setDepthWrite( on );
			}
			else if ( subname == "cull" )
			{
				Material::CullMode cull = (Material::CullMode)in->readInt();
				mat->setCull( cull );
			}
			else if ( subname == "lighting" )
			{
				bool on = (in->readInt() != 0);
				mat->setLighting( on );
			}
			else if ( subname == "texlayer" )
			{
				readTextureLayer( in, subend, mat, layers );
				++layers;
			}
			else if ( subname == "tex_only" )
			{
				lang::String type("bitmap");
				
				while ( in->size() < end )
				{
					in->beginChunk( &subname, &subend );
					
					if ( subname == "type" )
						type = in->readString();				
					else if ( subname == "texmap")
					{
						P(BaseTexture) tex = 0;
						if ( type == "cubemap" )
							tex = (sg::BaseTexture*)loadCubeTex( in->readString() );
						if (type == "bitmap" )
							tex = (sg::BaseTexture*)loadTex( in->readString() );					
						mat->setTexture( 0, tex );
					}
					in->endChunk( subend );
				}

				mat->setTextureColorCombine( 0, Material::TA_TEXTURE, Material::TOP_MODULATE, Material::TA_DIFFUSE );
				mat->setTextureAlphaCombine( 0, Material::TA_TEXTURE, Material::TOP_SELECTARG1, Material::TA_CURRENT );
			}
			else if ( subname == "tex_and_light" )
			{
				lang::String type("bitmap");
				while ( in->size() < end )
				{
					in->beginChunk( &subname, &subend );
					
					if ( subname == "type" )
						type = in->readString();				
					else if ( subname == "texmap")
					{
						P(BaseTexture) tex = 0;
						if ( type == "cubemap" )
							tex = (sg::BaseTexture*)loadCubeTex( in->readString() );
						if (type == "bitmap" )
							tex = (sg::BaseTexture*)loadTex( in->readString() );					
						mat->setTexture( 0, tex );
					}
					else if ( subname == "lightmap")
					{
						P(BaseTexture) tex = 0;
						if ( type == "cubemap" )
							tex = (sg::BaseTexture*)loadCubeTex( in->readString() );
						if (type == "bitmap" )
							tex = (sg::BaseTexture*)loadTex( in->readString() );					
						mat->setTexture( 1, tex );
					}
					in->endChunk( subend );
				}

				mat->setTextureColorCombine( 0, Material::TA_TEXTURE, Material::TOP_MODULATE, Material::TA_DIFFUSE );
				mat->setTextureAlphaCombine( 0, Material::TA_TEXTURE, Material::TOP_SELECTARG1, Material::TA_CURRENT );
				mat->setTextureColorCombine( 1, Material::TA_TEXTURE, Material::TOP_MODULATE, Material::TA_CURRENT );
				mat->setTextureAlphaCombine( 1, Material::TA_TEXTURE, Material::TOP_DISABLE, Material::TA_CURRENT );
			}
			else if ( subname == "env_only" &&
				subname == "tex_and_env" &&
				subname == "tex_and_bump" &&
				subname == "tex_and_masked_env" &&
				subname == "light_only" &&
				subname == "env_and_light" &&
				subname == "tex_env_and_light" &&
				subname == "tex_bump_and_light" &&
				subname == "tex_masked_env_and_light" )
			{
				throw Exception( Format("Unsupported texture channel combination ({0}) in material {1}", subname, name) );
			}
			else if ( subname == "diffuse" )
			{
				float r = in->readFloat();
				float g = in->readFloat();
				float b = in->readFloat();
				Colorf c = mat->diffuseColor();
				c.setRed( r );
				c.setGreen( g );
				c.setBlue( b );
				mat->setDiffuseColor( c );
			}
			else if ( subname == "specular" )
			{
				mat->setSpecularEnabled( true );
				
				float r = in->readFloat();
				float g = in->readFloat();
				float b = in->readFloat();
				mat->setSpecularColor( Colorf(r,g,b) );

				float e = in->readFloat();
				if ( e < 0.f )
					e = 0.f;
				if ( e > 1000.f )
					e = 1000.f;
				mat->setSpecularExponent( e );
			}
			else if ( subname == "emissive" )
			{
				float r = in->readFloat();
				float g = in->readFloat();
				float b = in->readFloat();
				//mat->setEmissiveColor( Colorf(r,g,b) );

				// emissive color does not work in ATI Radeon 9700 Pro
				if ( (r+g+b)*.33f > 0.49f )
					mat->setLighting( false );
			}
			else if ( subname == "opacity" )
			{
				float a = in->readFloat();
				if ( a < 0.f || a > 1.f )
					throw IOException( Format("Invalid material ({1}) opacity ({2,#.##}) in a geometry file: {0}", name, mat->name(), a) );
				Colorf c = mat->diffuseColor();
				c.setAlpha( a );
				mat->setDiffuseColor( c );
				mat->setTextureAlphaCombine( 0, Material::TA_DIFFUSE, Material::TOP_SELECTARG1, Material::TextureArgument() );
				if ( a == 0.f )
					mat->setEnabled( false );
			}
			else if ( subname == "nofog" )
			{
				bool on = (in->readInt() != 0);
				mat->setFogDisabled( on );
			}

			if ( in->size() > subend )
				throw IOException( Format("Chunk read overflow in material chunk of a geometry file: {0}",name) );
			in->endChunk( subend );
		}

		// enable per polygon depth sorting if the material is transparent and SORT tag present
		if ( mat->sourceBlend() != Material::BLEND_ONE && 
			mat->sourceBlend() != Material::BLEND_ZERO &&
			mat->name().indexOf("SORT") >= 0 )
		{
			Debug::println( "Enabling per polygon sorting for material {0}", mat->name() );
			mat->setPolygonSorting( true );
			mat->setPass( Material::DEFAULT_TRANSPARENCY_PASS );
		}

		// enable alpha test if requested
		/*if ( mat->name().indexOf("ATEST") >= 0 )
		{
			Debug::println( "Enabling alpha test for material {0}", mat->name() );
			mat->setBlend( Material::BLEND_ONE, Material::BLEND_ZERO );
			mat->setPolygonSorting( false );
			mat->setDepthWrite( true );
			mat->setPass( Material::DEFAULT_OPAQUE_PASS );
			mat->setAlphaTest( true, Material::CMP_GREATEREQUAL, 1 );
		}*/

		m_materials.add( mat.ptr() );
	}

	void readEffect( ChunkInputStream* in, long end )
	{
		String subname;
		long subend;

		P(Effect) fx = loadEffect( in->readString() );
		fx->setName( in->readString() );

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );

			if ( subname == "texture" )
			{
				String paramName = in->readString();
				String texFileName = in->readString();
				P(BaseTexture) tex = loadTex( texFileName );
				fx->setTexture( paramName, tex );
				//Debug::println( "Material {0}: Assigned bitmap texture {1} to effect parameter {2}", fx->name(), texFileName, paramName );
			}
			else if ( subname == "cubetexture" )
			{
				String paramName = in->readString();
				String texFileName = in->readString();
				P(BaseTexture) tex = loadCubeTex( texFileName );
				fx->setTexture( paramName, tex );
				//Debug::println( "Material {0}: Assigned cube texture {1} to effect parameter {2}", fx->name(), texFileName, paramName );
			}

			if ( in->size() > subend )
				throw IOException( Format("Chunk read overflow in effect chunk of a geometry file: {0}",name) );
			in->endChunk( subend );
		}

		// per polygon sorting enabled?
		if ( fx->name().indexOf("SORT") >= 0 )
		{
			Debug::println( "Enabling per polygon sorting for fx {0}", fx->name() );
			fx->setPolygonSorting( true );
			fx->setPass( Material::DEFAULT_TRANSPARENCY_PASS );
		}

		// alpha test can be enabled in .fx file so using the tag is meaningless
		if ( fx->name().indexOf("ATEST") >= 0 )
		{
			throw IOException( Format("ATEST (=alpha test enable) tag is meaningless when shader effect (fx) is used (file {0}, material {1})", name, fx->name()) );
		}

		m_materials.add( fx.ptr() );
	}

	String readStringChunk( ChunkInputStream* in, const String& chunkName )
	{
		String subname;
		long subend;
		in->beginChunk( &subname, &subend );
		if ( subname != chunkName )
			throw IOException( Format("Invalid chunk (got {0}, expected {1}) in geometry file: {2}",subname,chunkName,name) );
		String str = in->readString();
		in->endChunk( subend );
		return str;
	}

	P(Effect) loadEffect( const String& name )
	{
		String filename = File(m_path,name).getPath();
		P(InputStream) in = m_arch->getInputStream( filename );
		P(Effect) fx = new Effect( in );
		in->close();
		return fx;
	}

	P(Texture) loadTex( const String& name )
	{
		String filename = File(m_path,name).getPath();
		P(InputStream) in = m_arch->getInputStream( filename );
		P(Texture) tex = new Texture( in, filename );
		in->close();
		return tex;
	}

	P(CubeTexture) loadCubeTex( const String& name )
	{
		String filename = File(m_path,name).getPath();
		P(InputStream) in = m_arch->getInputStream( filename );
		P(CubeTexture) tex = new CubeTexture( in, filename );
		in->close();
		return tex;
	}

	void readTextureLayer( ChunkInputStream* in, long end, Material* mat, int layer )
	{
		String subname;
		long subend;

		while ( in->size() < end )
		{
			in->beginChunk( &subname, &subend );
			
			if ( subname == "filename" )
			{
				P(BaseTexture) tex = loadTex( in->readString() );
				mat->setTexture( layer, tex );
			}
			else if ( subname == "colorcombine" )
			{
				Material::TextureArgument arg1 = (Material::TextureArgument)in->readInt();
				Material::TextureOperation op = (Material::TextureOperation)in->readInt();
				Material::TextureArgument arg2 = (Material::TextureArgument)in->readInt();
				mat->setTextureColorCombine( layer, arg1, op, arg2 );
			}
			else if ( subname == "alphacombine" )
			{
				Material::TextureArgument arg1 = (Material::TextureArgument)in->readInt();
				Material::TextureOperation op = (Material::TextureOperation)in->readInt();
				Material::TextureArgument arg2 = (Material::TextureArgument)in->readInt();
				mat->setTextureAlphaCombine( layer, arg1, op, arg2 );
			}
			else if ( subname == "coordset" )
			{
				int index = in->readInt();
				mat->setTextureCoordinateSet( layer, index );
			}
			else if ( subname == "coordgen_env" ||
				subname == "coordgen_refl" ||
				subname == "coordgen_planar" )
			{
				float uoffs = in->readFloat();
				float voffs = in->readFloat();
				float uscale = in->readFloat();
				float vscale = in->readFloat();
				Material::TextureCoordinateTransformMode ttff = Material::TTFF_COUNT2;
				Material::TextureCoordinateSourceType tcs = Material::TCS_CAMERASPACENORMAL;
				
				if ( subname == "coordgen_env" )
					tcs = Material::TCS_CAMERASPACENORMAL;
				else if ( subname == "coordgen_refl" )
					tcs = Material::TCS_CAMERASPACEREFLECTIONVECTOR;
				else if ( subname == "coordgen_planar" )
					tcs = Material::TCS_CAMERASPACEPOSITION;

				Matrix4x4 mtx(0);
				mtx(0,0) = uscale;
				mtx(1,1) = vscale;
				mtx(0,3) = uoffs;
				mtx(1,3) = voffs;
				
				mat->setTextureCoordinateSource( layer, tcs );
				mat->setTextureCoordinateTransform( layer, ttff, mtx );
			}

			in->endChunk( subend );
		}
	}

private:
	InputStreamArchive*		m_arch;
	int						m_meshBones;
	const String*			m_meshBoneNames;
	ModelFileCache*			m_modelCache;
	int						m_loadFlags;
	Colorf					m_ambient;
	String					m_path;
	Vector< P(Shader) >		m_materials;

	ModelFileImpl();
	ModelFileImpl( const ModelFileImpl& );
	ModelFileImpl& operator=( const ModelFileImpl& );
};

//-----------------------------------------------------------------------------

ModelFile::ModelFile( const String& name, 
	const String* boneNames, int bones,
	const Colorf& ambient,
	InputStreamArchive* arch,
	ModelFileCache* modelCache, 
	int loadFlags )
{
	m_this = new ModelFileImpl( name, boneNames, bones, ambient, arch, modelCache, loadFlags );
}

sg::Primitive* ModelFile::getPrimitive( int index ) const
{
	assert( index >= 0 && index < primitives() );
	return m_this->primitives[index];
}

int	ModelFile::primitives() const
{
	return m_this->primitives.size();
}

const String& ModelFile::name() const
{
	return m_this->name;
}


} // sgu
