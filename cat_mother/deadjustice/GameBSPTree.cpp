#include "GameBSPTree.h"
#include "CollisionInfo.h"
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/VertexAndIndexLock.h>
#include <io/File.h>
#include <io/InputStream.h>
#include <io/FileOutputStream.h>
#include <io/InputStreamArchive.h>
#include <pix/Color.h>
#include <pix/Colorf.h>
#include <bsp/BSPNode.h>
#include <bsp/BSPTree.h>
#include <bsp/BSPFile.h>
#include <bsp/BSPTreeBuilder.h>
#include <bsp/BSPBalanceSplitSelector.h>
#include <bsp/BSPBoxSplitSelector.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <lang/Character.h>
#include <algorithm>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace bsp;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

static void getPolys( BSPNode* bspnode, Vector<BSPPolygon>& polys )
{
	for ( int i = 0 ; i < bspnode->polygons() ; ++i )
		polys.add( bspnode->getPolygon(i) );

	if ( bspnode->positive() )
		getPolys( bspnode->positive(), polys );
	if ( bspnode->negative() )
		getPolys( bspnode->negative(), polys );
}

//-----------------------------------------------------------------------------

GameBSPTree::GameBSPTree( io::InputStreamArchive* arch, sg::Node* geometry, 
	const lang::String& bspFileName, int bspBuildPolySkip, 
	const Vector<P(GameSurface)>& collisionMaterialTypes, bsp::BSPTree* cachedBSPTree ) :
	m_tree( cachedBSPTree ),
	m_models( Allocator<MeshModelPair>(__FILE__) ),
	m_boundSphere( 0.f )
{
	if ( !m_tree )
	{
		// Construct BSP tree for collision detection if not found on disk
		if ( !File(bspFileName).exists() )
		{
			m_tree = buildBSP( geometry, bspBuildPolySkip );
			FileOutputStream output( bspFileName );
			BSPFile collisiontree( m_tree, &output );
		}
		else
		{
			P(InputStream) in = arch->getInputStream( bspFileName );
			BSPFile	collisiontree( in );
			m_tree = collisiontree.tree();
		}
	}

	// Find all (mesh,model) pairs in the hierarchy
	int modelPolys = 0;
	for ( Node* it = geometry ; it ; it = it->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( it );

		if ( isCollidable(mesh) )
		{
			for ( int i = 0; i < mesh->primitives(); ++i )
			{
				Model* model = dynamic_cast<Model*>( mesh->getPrimitive(i) );

				if ( model )
				{
					// get collision material type
					P(GameSurface) surface = getCollisionMaterialType( model->shader()->name(), collisionMaterialTypes );
					MeshModelPair pair( mesh, model, surface );
					m_models.add( pair );
					modelPolys += model->indices()/3;
				}
			}
		}
	}
	Debug::println( "BSP {0} has {1} source polygons", bspFileName, modelPolys );

	// remove collision meshes
	for ( Node* it = geometry ; it ; it = it->nextInHierarchy() )
	{
		if ( !isVisual(it) )
		{
			Debug::println( "Removed collision object {0} from visual scene {1}", it->name(), geometry->name() );
			it->unlink();
			it = geometry;
		}
	}

	/*// DEBUG: count duplicate polygons
	Debug::println( "Computing BSP tree {0} polygon stats...", bspFileName );
	Vector<BSPPolygon> polys( Allocator<BSPPolygon>(__FILE__) );
	getPolys( m_root, polys );
	int bsppolys = polys.size();
	int dups = 0;
	for ( int i = 0 ; i < polys.size() ; ++i )
	{
		for ( int j = i+1 ; j < polys.size() ; ++j )
		{
			if ( polys[i] == polys[j] )
			{
				polys.remove( j-- );
				++dups;
			}
		}
	}
	Debug::println( "BSP tree has {1} polys ({0} unique, {2} duplicates), potential memsave={3,#}KB", polys.size(), bsppolys, dups, dups*sizeof(BSPPolygon)/1000 );
	*/

	m_boundSphere = computeBoundSphereRecurse( m_tree->root(), 0.f );
}

GameSurface* GameBSPTree::getCollisionMaterialType( const String& materialName, const Vector<P(GameSurface)>& collisionMaterialTypes )
{
	assert( collisionMaterialTypes.size() > 0 );

	// find type tag
	int typeIndex = materialName.indexOf( "TYPE=" );
	if ( typeIndex == -1 )
		return collisionMaterialTypes.firstElement();
	typeIndex += 5;
	int typeEnd = typeIndex;
	while ( typeEnd < materialName.length() && Character::isUpperCase(materialName.charAt(typeEnd)) )
		++typeEnd;

	String typeTag = materialName.substring( typeIndex, typeEnd );
	GameSurface* surface = 0;
	for ( int i = 0 ; i < collisionMaterialTypes.size() ; ++i )
	{
		if ( collisionMaterialTypes[i]->name() == typeTag )
		{
			surface = collisionMaterialTypes[i];
			break;
		}
	}
	if ( !surface )
		throw Exception( Format("Collision material tag {0} (used in material {1}) not found in level", typeTag, materialName) );

	return surface;
}

bool GameBSPTree::getVisualByBSPPolygonID( int polyid, sg::Mesh** mesh, sg::Model** model, int* triangleIndex, GameSurface** surface ) const
{
	int modelIndex = (polyid>>22) & 0x3FF;
	int triIndex = polyid & 0x3FFFFF;

	assert( modelIndex >= 0 && modelIndex < m_models.size() );
	if ( modelIndex >= 0 && modelIndex < m_models.size() )
	{
		const MeshModelPair& pair = m_models[modelIndex];
		int indices = pair.model->indices();

		//assert( triIndex >= 0 && triIndex*3 < indices );
		if ( triIndex >= 0 && triIndex*3 < indices )
		{
			if (mesh) *mesh = pair.mesh;
			if (model) *model = pair.model;
			if (triangleIndex) *triangleIndex = triIndex;
			if (surface) *surface = pair.surface;
			return true;
		}
	}

	return false;
}

bool GameBSPTree::getLightmapPixel( const Vector3& worldPoint, const BSPPolygon* poly, Colorf* pix ) const
{
	// get visual mesh
	int polyid = poly->id();
	Mesh* mesh = 0;
	Model* model = 0;
	int polyIndex = 0;
	getVisualByBSPPolygonID( polyid, &mesh, &model, &polyIndex, 0 );

	if ( mesh && model )
	{
		// get lightmap texture
		Texture* lightmap = 0;
		Shader* shader = model->shader();
		if ( shader->hasParameter("tLightMap") )
		{
			lightmap = dynamic_cast<Texture*>( shader->getTexture( "tLightMap" ) );
		}
		else
		{
			Material* mtl = dynamic_cast<Material*>( shader );
			if ( mtl )
				lightmap = dynamic_cast<Texture*>( mtl->getTexture(1) );
		}

		if ( lightmap )
		{
			//assert( model->vertexFormat().textureCoordinates() == 2 );
			if ( model->vertexFormat().textureCoordinates() >= 2 )
			{
				// get world space vertex positions
				VertexAndIndexLock<Model> lk( model, Model::LOCK_READ );
				const int n = 3;
				Vector3 v[n];
				Vector3 normals[n];
				Vector3 texcoord[n];
				Matrix4x4 wtm = mesh->worldTransform();
				for ( int i = 0 ; i < n ; i++ )
				{
					int ix;
					model->getIndices( polyIndex*n+i, &ix );

					Vector3 v0;
					model->getVertexPositions( ix, &v0 );
					v[i] = wtm.transform( v0 );

					float uv[2] = {0,0};
					model->getVertexTextureCoordinates( ix, 1, 2, uv );
					texcoord[i] = Vector3( uv[0], uv[1], 0 );

					Vector3 n0;
					model->getVertexNormals( ix, &n0 );
					normals[i] = wtm.rotate( n0 );
				}

				// compute texture coordinate at collision point
				Vector3 vec1 = v[1] - v[0];
				Vector3 vec2 = v[2] - v[0];
				Vector3 tvec1 = texcoord[1] - texcoord[0];
				Vector3 tvec2 = texcoord[2] - texcoord[0];
				Matrix4x4 toWorld(1.f);
				toWorld.setColumn( 0, Vector4(vec1.x,vec1.y,vec1.z,0.f) );
				toWorld.setColumn( 1, Vector4(vec2.x,vec2.y,vec2.z,0.f) );
				toWorld.setColumn( 2, Vector4(normals[0].x,normals[0].y,normals[0].z,0.f) );
				toWorld.setColumn( 3, Vector4(v[0].x,v[0].y,v[0].z,1.f) );
				Matrix4x4 toPoly = toWorld.inverse();
				Vector4 polyPoint = toPoly * Vector4(worldPoint.x,worldPoint.y,worldPoint.z,1.f);
				Vector3 uv = texcoord[0] + tvec1*polyPoint.x + tvec2*polyPoint.y;

				// get color
				*pix = Colorf( Color(lightmap->getPixel( uv.x, uv.y )) );
				return true;
			}
		}
	}

	return false;
}

P(BSPTree) GameBSPTree::buildBSP( Node* root, int bspBuildPolySkip )
{
	BSPTreeBuilder	bspbuilder;
	Vector<Vector3> vertexpositions( Allocator<Vector3>(__FILE__) );
	Vector<int>		indices( Allocator<int>(__FILE__) );
	int				modelIndex = 0; // used for computing unique id (bsp->visual mapping) for a BSP polygon

	for ( Node* it = root ; it ; it = it->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>(it);
		if ( isCollidable(mesh) )
		{
			Vector3 minv(Float::MAX_VALUE,Float::MAX_VALUE,Float::MAX_VALUE);
			Vector3 maxv(-Float::MAX_VALUE,-Float::MAX_VALUE,-Float::MAX_VALUE);

			for ( int i = 0; i < mesh->primitives(); ++i )
			{
				Model* model = dynamic_cast<Model*>(mesh->getPrimitive(i));
				if ( model )
				{
					VertexAndIndexLock<Model> lk( model, Model::LOCK_READ );

					vertexpositions.setSize( model->vertices() );
					indices.setSize( model->indices() );

					model->getTransformedVertexPositions( &mesh->worldTransform(), 1, Matrix4x4(1), vertexpositions.begin(), model->vertices(), false );
					model->getIndices( 0, indices.begin(), model->indices() );

					// collision mask for this material
					int collisionMask = CollisionInfo::COLLIDE_GEOMETRY_SOLID;
					if ( model->shader() && model->shader()->name().indexOf("SEETHRU") != -1 )
					{
						Debug::println( "Material {0} of mesh {1} can be seen/shoot through", model->shader()->name(), mesh->name() );
						collisionMask = CollisionInfo::COLLIDE_GEOMETRY_SEETHROUGH;
					}

					// update min/max vertex bounds
					for ( int j = 0 ; j < vertexpositions.size() ; ++j )
					{
						const Vector3& v = vertexpositions[j];
						for ( int k = 0 ; k < 3 ; ++k )
						{
							minv[k] = Math::min( v[k], minv[k] );
							maxv[k] = Math::max( v[k], maxv[k] );
						}
					}
				
					// add polys to BSP tree builder
					for ( int j = 0; j < model->indices() / 3; ++j )
					{
						int polyid = (modelIndex<<22) + j;

						// get vertex positions
						Vector3 triangle[3];
						triangle[0] = vertexpositions[ indices[ j*3 + 0 ] ];
						triangle[1] = vertexpositions[ indices[ j*3 + 1 ] ];
						triangle[2] = vertexpositions[ indices[ j*3 + 2 ] ];

						// check for poly ID overflow
						/*Mesh* mesh2 = 0;
						Model* model2 = 0;
						int tri2 = 0;
						getVisualByBSPPolygonID( polyid, &mesh2, &model2, &tri2, 0 );
						if ( mesh2 != mesh || model2 != model || tri2 != j )
							throw Exception( Format("BSP polygon ID overflow in mesh {0}", mesh->name()) );*/

						bspbuilder.addPolygon( triangle, 3, polyid, collisionMask ); 
						if ( collisionMask & CollisionInfo::COLLIDE_GEOMETRY_SEETHROUGH )
						{
							std::reverse( triangle, triangle+3 );
							bspbuilder.addPolygon( triangle, 3, polyid, collisionMask ); 
						}
					}

					++modelIndex;
				}

				Debug::println( "{0} bounds: min=({1}, {2}, {3}), max=({4}, {5}, {6})", mesh->name(), minv.x, minv.y, minv.z, maxv.x, maxv.y, maxv.z );
			}
		}
	}
	
	Debug::println( "Building {0} BSP", root->name() );
	BSPBalanceSplitSelector selector( bspBuildPolySkip );
	return bspbuilder.build( &selector );
}

bool GameBSPTree::isCollidable( Mesh* mesh )
{
	return mesh != 0 && -1 == mesh->name().indexOf("VISUAL");
}

bool GameBSPTree::isVisual( Node* node )
{
	return node != 0 && -1 == node->name().indexOf("COLLIDE");
}

float GameBSPTree::boundSphere() const
{
	return m_boundSphere;
}

bsp::BSPNode* GameBSPTree::root() const
{
	return m_tree->root();
}

float GameBSPTree::computeBoundSphereRecurse( bsp::BSPNode* node, float r )
{
	if ( !node )
		return r;

	for ( int i = 0 ; i < node->polygons() ; ++i )
	{
		const BSPPolygon& poly = node->getPolygon(i);
		
		for ( int k = 0 ; k < poly.vertices() ; ++k )
		{
			Vector3 v = poly.getVertex(k);
			float vlen = v.length();
			if ( vlen > r )
				r = vlen;
		}
	}

	if ( node->positive() )
		r = computeBoundSphereRecurse( node->positive(), r );
	if ( node->negative() )
		r = computeBoundSphereRecurse( node->negative(), r );
	return r;
}

bsp::BSPTree* GameBSPTree::tree() const
{
	return m_tree;
}
