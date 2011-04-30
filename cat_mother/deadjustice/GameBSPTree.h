#ifndef _GAMEBSPTREE_H
#define _GAMEBSPTREE_H


#include <sg/Mesh.h>
#include <sg/Model.h>
#include "GameSurface.h"


namespace io {
	class InputStreamArchive;}

namespace sg {
	class Mesh;
	class Model;
	class Node;}

namespace bsp {
	class BSPTree;
	class BSPPolygon;
	class BSPNode;}

namespace pix {
	class Colorf;}

namespace math {
	class Vector3;}

namespace lang {
	class String;}


/** 
 * BSP tree created from visual scene graph.
 * Provides also mapping between visual polygons and BSP polygons.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class GameBSPTree :
	public lang::Object
{
public:
	/** 
	 * Constructs BSP tree in root space. 
	 * BSP tree is either constructed from the disk (cached)
	 * or by computing it on the fly and then storing it to disk for re-use.
	 *
	 * @param arch Archive used in loading.
	 * @param geometry Geometry hierarchy used to compute BSP tree.
	 * @param bspFilename File used to load/store BSP tree as cache.
	 * @param bspBuildPolySkip BSP building quality. 0 is best, larger values compute the tree faster.
	 * @param collisionMaterialTypes Used for mapping visual scene material types (TYPE=METAL etc.) to surfaces meaningful to game.
	 * @param cachedBSPTree Cached BSP tree if any. Can be 0.
	 */
	GameBSPTree( io::InputStreamArchive* arch, sg::Node* geometry,
		const lang::String& bspFileName, int bspBuildPolySkip, 
		const util::Vector<P(GameSurface)>& collisionMaterialTypes, bsp::BSPTree* cachedBSPTree );

	/** Returns root BSP tree node. */
	bsp::BSPNode*	root() const;

	/** Returns BSP tree. */
	bsp::BSPTree*	tree() const;

	/** 
	 * Gets pixel at lightmap point. 
	 * @return true if lightmap available at specified point.
	 */
	bool			getLightmapPixel( const math::Vector3& worldPoint, const bsp::BSPPolygon* poly, pix::Colorf* pix ) const;

	/** 
	 * Returns mesh, model and triangle index by BSP polygon id. 
	 * @return true if data retrieved successfully.
	 */
	bool			getVisualByBSPPolygonID( int polyid, sg::Mesh** mesh, sg::Model** model, int* triangleIndex, GameSurface** surface ) const;

	/** Returns bounding sphere radius. */
	float			boundSphere() const;

private:
	/** Helper class for mapping BSP polygon to visual polygon. */
	class MeshModelPair
	{
	public:
		P(sg::Mesh)		mesh;
		P(sg::Model)	model;
		P(GameSurface)	surface;

		MeshModelPair() {}
		MeshModelPair( sg::Mesh* meshPtr, sg::Model* modelPtr, GameSurface* gameSurface ) : mesh(meshPtr), model(modelPtr), surface(gameSurface) {}
	};

	P(bsp::BSPTree)					m_tree;
	util::Vector<MeshModelPair>		m_models;
	float							m_boundSphere;

	/** Returns game surface which matches specified material name (or tag in the name). */
	static GameSurface*		getCollisionMaterialType( const lang::String& materialName, const util::Vector<P(GameSurface)>& collisionMaterialTypes );

	/** Builds BSP tree from scene. */
	static P(bsp::BSPTree)	buildBSP( sg::Node* root, int bspBuildPolySkip );

	/** Returns true if mesh should be used in collision checks. */
	static bool				isCollidable( sg::Mesh* mesh );

	/** Returns true if node should be used in rendering. */
	static bool				isVisual( sg::Node* node );

	/** Computes bounding sphere recursively. */
	static float			computeBoundSphereRecurse( bsp::BSPNode* node, float r );

	GameBSPTree( const GameBSPTree& );
	GameBSPTree& operator=( const GameBSPTree& );
};


#endif // _GAMEBSPTREE_H
