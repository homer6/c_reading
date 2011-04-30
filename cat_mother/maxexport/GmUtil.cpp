#include "StdAfx.h"
#include "GmUtil.h"
#include "GmModel.h"
#include "GmMaterial.h"
#include "TmUtil.h"
#include "PhyExportUtil.h"
#include "ShellMaterial.h"
#include "SkinExportUtil.h"
#include "SceneExportUtil.h"
#include "GmModelPrimitive.h"
#include <mb/Vertex.h>
#include <mb/Polygon.h>
#include <mb/VertexMap.h>
#include <mb/VertexMapFormat.h>
#include <mb/DiscontinuousVertexMap.h>
#include <io/IOException.h>
#include <dev/Profile.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/Character.h>
#include <algorithm>
#include <stdmat.h>
#ifdef SGEXPORT_LODCTRL
#include <LODCtrl.h>
#endif
#ifdef SGEXPORT_PHYSIQUE
#include <PhyExp.h>
#include <BipExp.h>
#endif
#include <bmmlib.h>
#include <modstack.h>
#include <samplers.h>

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

static const Matrix3	s_convtm	= Matrix3( Point3(1,0,0), Point3(0,1,0), Point3(0,0,-1), Point3(0,0,0) );

//-----------------------------------------------------------------------------

static pix::Colorf toColorf( const Color& c )
{
	return pix::Colorf( c.r, c.g, c.b );
}

static Vector3 toVector3( const Point3& p )
{
	return Vector3( p.x, p.y, p.z );
}

static void setLayerTex( GmMaterial::TextureLayer& layer, BitmapTex* bmptex,
	const String& materialName )
{
	int mapChannel = bmptex->GetMapChannel();
	if ( mapChannel > 8 )
		throw IOException( Format("Too large Map Channel ({1}) in material {0}", materialName, mapChannel) );

	layer.filename = bmptex->GetMapName();
	layer.coordset = mapChannel - 1;
	layer.filter = (FILTER_NADA != bmptex->GetFilterType());

	layer.env = GmMaterial::ENV_NONE;
	StdUVGen* uvgen = bmptex->GetUVGen();
	if ( uvgen && uvgen->GetSlotType() == MAPSLOT_ENVIRON )
	{
		int type = uvgen->GetCoordMapping(0);
		switch ( type )
		{
		case UVMAP_EXPLICIT:	layer.env = GmMaterial::ENV_NONE; break;
		case UVMAP_SPHERE_ENV:	layer.env = GmMaterial::ENV_SPHERICAL; break;
		case UVMAP_CYL_ENV:		layer.env = GmMaterial::ENV_CYLINDRICAL; break;
		case UVMAP_SHRINK_ENV:	layer.env = GmMaterial::ENV_SHRINKWRAP; break;
		case UVMAP_SCREEN_ENV:	layer.env = GmMaterial::ENV_SCREEN; break;
		}

		TimeValue time = 0;
		layer.uoffs = 0.5f + uvgen->GetUOffs(time);
		layer.voffs = 0.5f - uvgen->GetVOffs(time);
		layer.uscale = 0.5f * uvgen->GetUScl(time);
		layer.vscale = -0.5f * uvgen->GetVScl(time);
	}
}

static void extractTriangleGeometry( GmModel* gm, INode* node, Mesh* mesh, Mtl* material )
{
#ifdef SGEXPORT_PHYSIQUE
	Modifier*			phyMod		= 0;
	IPhysiqueExport*	phyExport	= 0;
	IPhyContextExport*	mcExport	= 0;
#endif
	Modifier*			skinMod		= 0;
	ISkin*				skin		= 0;
	String				nodeName	( node->GetName() );

	try
	{
		// vertex transform to left-handed system
		Matrix3 pivot = TmUtil::getPivotTransform( node );
		Matrix3 vertexTM = pivot * s_convtm;
		bool insideOut = TmUtil::hasNegativeParity( pivot );

		/*Matrix4x4 pm = TmUtil::toLH( vertexTM );
		Debug::println( "Object {0} vertex local TM is", nodeName );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(0,0), pm(0,1), pm(0,2), pm(0,3) );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(1,0), pm(1,1), pm(1,2), pm(1,3) );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(2,0), pm(2,1), pm(2,2), pm(2,3) );
		Debug::println( "  {0,#.###} {1,#.###} {2,#.###} {3,#.###}", pm(3,0), pm(3,1), pm(3,2), pm(3,3) );*/

		// add vertex positions
		int vertices = mesh->getNumVerts();
		for ( int vi = 0 ; vi < vertices ; ++vi )
		{
			Point3 v = vertexTM * mesh->verts[vi];
			mb::Vertex* vert = gm->addVertex();
			vert->setPosition( v.x, v.y, v.z );
		}

		// add vertex weights (from Physique modifier)
#ifdef SGEXPORT_PHYSIQUE
		phyMod = PhyExportUtil::findPhysiqueModifier( node );
		if ( phyMod )
		{
			Debug::println( "  Found Physique modifier: {0}", gm->name );

			// get (possibly shared) Physique export interface
			phyExport = (IPhysiqueExport*)phyMod->GetInterface( I_PHYINTERFACE );
			if( !phyExport )
				throw Exception( Format("No Physique modifier export interface") );

			// export from initial pose?
			phyExport->SetInitialPose( false );
			
			// get (unique) context dependent export inteface
			mcExport = (IPhyContextExport*)phyExport->GetContextInterface( node );
			if( !mcExport )
				throw Exception( Format("No Physique modifier context export interface") );

			// convert to rigid for time independent vertex assignment
			mcExport->ConvertToRigid( true );

			// allow blending to export multi-link assignments
			mcExport->AllowBlending( true );

			// list bones
			Vector<INode*> bones( Allocator<INode*>(__FILE__,__LINE__) );
			PhyExportUtil::listBones( mcExport, bones );

			// add vertex weight maps
			for ( int i = 0 ; i < bones.size() ; ++i )
			{
				INode* bone = bones[i];
				String name = bone->GetName();
				mb::VertexMap* vmap = gm->addVertexMap( 1, name, mb::VertexMapFormat::VERTEXMAP_WEIGHT );
				PhyExportUtil::addWeights( vmap, bone, mcExport );
			}
		}
#endif // SGEXPORT_PHYSIQUE

		// add vertex weights (from Skin modifier)
		skinMod = SkinExportUtil::findSkinModifier( node );
		if ( skinMod )
		{
			skin = (ISkin*)skinMod->GetInterface(I_SKIN);
			require( skin );
			ISkinContextData* skincx = skin->GetContextInterface( node );
			require( skincx );
			Debug::println( "  Found Skin modifier: {0} ({1} bones, {2} points)", gm->name, skin->GetNumBones(), skincx->GetNumPoints() );
			
			if ( skincx->GetNumPoints() != gm->vertices() )
				throw Exception( Format("Only some vertices ({0}/{1}) of {2} are skinned", skincx->GetNumPoints(), gm->vertices(), gm->name) );

			// list bones
			Vector<INode*> bones( Allocator<INode*>(__FILE__,__LINE__) );
			SkinExportUtil::listBones( skin, bones );

			// add vertex weight maps
			for ( int i = 0 ; i < bones.size() ; ++i )
			{
				INode* bone = bones[i];
				String name = bone->GetName();
				mb::VertexMap* vmap = gm->addVertexMap( 1, name, mb::VertexMapFormat::VERTEXMAP_WEIGHT );
				SkinExportUtil::addWeights( vmap, bone, skin, skincx );
				//Debug::println( "    Bone {0} is affecting {1} vertices", name, vmap->size() );
			}

			// DEBUG: print skin node tm and initial object tm
			/*Matrix3 tm;
			int ok = skin->GetSkinInitTM( node, tm );
			require( ok == SKIN_OK );
			Debug::println( "  NodeInitTM of {0}", nodeName );
			TmUtil::println( tm, 4 );
			ok = skin->GetSkinInitTM( node, tm, true );
			require( ok == SKIN_OK );
			Debug::println( "  NodeObjectTM of {0}", nodeName );
			TmUtil::println( tm, 4 );*/

			// DEBUG: print bones
			/*Debug::println( "  bones of {0}:", nodeName );
			for ( int i = 0 ; i < bones.size() ; ++i )
			{
				Debug::println( "    bone ({0}): {1}", i, String(bones[i]->GetName()) );
				skin->GetBoneInitTM( bones[i], tm );
				Debug::println( "      InitNodeTM:" );
				TmUtil::println( tm, 6 );
				skin->GetBoneInitTM( bones[i], tm, true );
				Debug::println( "      InitObjectTM:" );
				TmUtil::println( tm, 6 );
			}*/

			// DEBUG: print bones used by the points
			/*for ( int i = 0 ; i < skincx->GetNumPoints() ; ++i )
			{
				int bonec = skincx->GetNumAssignedBones(i);
				Debug::println( "    point {0} has {1} bones", i, bonec );
				for ( int k = 0 ; k < bonec ; ++k )
				{
					int boneidx = skincx->GetAssignedBone( i, k );
					float w = skincx->GetBoneWeight( i, k );
					Debug::println( "    point {0} boneidx ({1}): {2}, weight {3}", i, k, boneidx, w );
				}
			}*/
		}

		// ensure clockwise polygon vertex order
		int vx[3] = {2,1,0};
		if ( insideOut )
		{
			int tmp = vx[0];
			vx[0] = vx[2];
			vx[2] = tmp;
		}
	
		// list unique materials used by the triangles
		Vector<ShellMaterial> usedMaterials( Allocator<ShellMaterial>(__FILE__,__LINE__) );
		if ( material )
		{
			for ( int fi = 0 ; fi < mesh->getNumFaces() ; ++fi )
			{
				Face& face = mesh->faces[fi];
				
				int mergesubmaterial = -1;
				int originalsubmaterial = -1;

				for ( int j = 0; j < material->NumSubMtls(); ++j)
				{
					// Get Sub Material Slot name 
					TSTR name = material->GetSubMtlSlotName(j);

					// Light maps are stored in sub material slot named "Baked Material"
					if ( strcmp( name, "Baked Material" ) == 0 ) 
						mergesubmaterial = j;
					else 
						originalsubmaterial = j;				
				}

				if ( mergesubmaterial != -1 ) // A baked material was found, shell materials will be created
				{
					Mtl* mat = material->GetSubMtl( originalsubmaterial );
					Mtl* bakedmtl = material->GetSubMtl( mergesubmaterial );
				
					if ( mat->NumSubMtls() > 0 ) // Check for nested multi-material
					{
						for ( int j = 0; j < mat->NumSubMtls(); ++j)
							usedMaterials.add( ShellMaterial( mat->GetSubMtl( face.getMatID() % mat->NumSubMtls() ), bakedmtl ) );
					} else
						usedMaterials.add( ShellMaterial( mat, bakedmtl ) );
				}
				else if ( material->NumSubMtls() > 0 ) // Multi-material without baked material 
				{  
					usedMaterials.add( ShellMaterial( material->GetSubMtl( face.getMatID() % material->NumSubMtls() ), 0 ) ); 
				}
				else	// Single material without baked material
				{
					usedMaterials.add( ShellMaterial( material, 0 ) );
				}				
			}
			std::sort( usedMaterials.begin(), usedMaterials.end() );
			usedMaterials.setSize( std::unique( usedMaterials.begin(), usedMaterials.end() ) - usedMaterials.begin() );
		}

		// create used materials
		for ( int mi = 0 ; mi < usedMaterials.size() ; ++mi )
		{
			ShellMaterial shellmtl = usedMaterials[mi];
			gm->materials.add( GmUtil::createGmMaterial( shellmtl.original, shellmtl.baked ) );
		}

		// add triangles
		for ( int fi = 0 ; fi < mesh->getNumFaces() ; ++fi )
		{
			mb::Polygon* poly = gm->addPolygon();

			// triangle indices
			Face& face = mesh->faces[fi];
			for ( int vxi = 0 ; vxi < 3 ; ++vxi )
			{
				int vi = face.v[ vx[vxi] ];
				require( vi >= 0 && vi < gm->vertices() );
				mb::Vertex* vert = gm->getVertex( vi );
				poly->addVertex( vert );
			}

			// triangle material
			int polyMaterialIndex = 0;
			if ( material )
			{
				ShellMaterial mat( material, 0 );
				
				int numsubmaterials = material->NumSubMtls();  

				if ( numsubmaterials > 0 )
				{
					mat.original = material->GetSubMtl( face.getMatID() % material->NumSubMtls() );
		
					for ( int j = 0; j < numsubmaterials; ++j) // Is baked material present?
					{
						TSTR name = material->GetSubMtlSlotName(j);
						if ( strcmp( name, "Baked Material" ) == 0 )
							mat.baked = material->GetSubMtl( j );
						if ( strcmp( name, "Original Material" ) == 0 )
							mat.original = material->GetSubMtl( j );
					}
					if ( mat.original->NumSubMtls() > 0 )  // Is there a nested multi-material?
					{
						mat.original = mat.original->GetSubMtl( face.getMatID() % mat.original->NumSubMtls() );
					}
				}

				for ( int mi = 0 ; mi < usedMaterials.size() ; ++mi )
				{
					if ( usedMaterials[mi] == mat )
					{
						polyMaterialIndex = mi;
						break;
					}
				}
			}
			poly->setMaterial( polyMaterialIndex );
		}

		// add vertex colors
		int mp = 0;
		if ( mesh->mapSupport(mp) && mesh->getNumMapVerts(mp) > 0 )
		{
			mb::DiscontinuousVertexMap* vmad = gm->addDiscontinuousVertexMap( 3, "rgb", mb::VertexMapFormat::VERTEXMAP_RGB );

			int tverts = mesh->getNumMapVerts( mp );
			UVVert* tvert = mesh->mapVerts( mp );
			TVFace* tface = mesh->mapFaces( mp );

			//Debug::println( "Vertex colors:" );
			for ( int fi = 0 ; fi < mesh->getNumFaces() ; ++fi )
			{
				Face& face = mesh->faces[fi];
				mb::Polygon* poly = gm->getPolygon( fi );

				for ( int vxi = 0 ; vxi < 3 ; ++vxi )
				{
					int vi = face.v[ vx[vxi] ];
					mb::Vertex* vert = gm->getVertex( vi );
					Point3 tc = tvert[ tface[fi].t[ vx[vxi] ] % tverts ];
					float rgb[3] = {tc.x, tc.y, tc.z};
					vmad->addValue( vert->index(), poly->index(), rgb, 3 );
					//Debug::println( "  vertex[{0}].rgb: {1} {2} {3}", vert->index(), rgb[0], rgb[1], rgb[2] );
				}
			}
		}

		// add texcoord layers
		int lastCoords = MAX_MESHMAPS-2;
		while ( lastCoords > 0 && (!mesh->mapSupport(lastCoords) || 0 == mesh->getNumMapVerts(lastCoords)) )
			--lastCoords;
		if ( lastCoords > 8 )
			throw IOException( Format("Too many texture coordinate sets ({1}) in {0}", gm->name, lastCoords) );

		for ( mp = 1 ; mp <= lastCoords ; ++mp )
		{
			mb::DiscontinuousVertexMap* vmad = gm->addDiscontinuousVertexMap( 2, "uv", mb::VertexMapFormat::VERTEXMAP_TEXCOORD );

			if ( mesh->mapSupport(mp) && mesh->getNumMapVerts(mp) > 0 )
			{
				int tverts = mesh->getNumMapVerts( mp );
				UVVert* tvert = mesh->mapVerts( mp );
				TVFace* tface = mesh->mapFaces( mp );
				
				for ( int fi = 0 ; fi < mesh->getNumFaces() ; ++fi )
				{
					Face& face = mesh->faces[fi];
					mb::Polygon* poly = gm->getPolygon( fi );

					for ( int vxi = 0 ; vxi < 3 ; ++vxi )
					{
						int vi = face.v[ vx[vxi] ];
						mb::Vertex* vert = gm->getVertex( vi );
						Point3 tc = tvert[ tface[fi].t[ vx[vxi] ] % tverts ];
						float uv[2] = {tc.x, 1.f-tc.y};
						vmad->addValue( vert->index(), poly->index(), uv, 2 );
					}
				}
			}
		}

		// compute face vertex normals from smoothing groups
		require( mesh->getNumFaces() == gm->polygons() );
		mb::DiscontinuousVertexMap* vmad = gm->addDiscontinuousVertexMap( 3, "vnormals", mb::VertexMapFormat::VERTEXMAP_NORMALS );
		for ( int fi = 0 ; fi < gm->polygons() ; ++fi )
		{
			mb::Polygon* poly = gm->getPolygon( fi );
			require( poly );
			require( poly->index() >= 0 && poly->index() < mesh->getNumFaces() );
			Face& face = mesh->faces[ poly->index() ];

			require( poly->vertices() == 3 );
			for ( int j = 0 ; j < poly->vertices() ; ++j )
			{
				Vector3 vn(0,0,0);
				mb::Vertex* vert = poly->getVertex( j );

				// sum influencing normals
				for ( int k = 0 ; k < vert->polygons() ; ++k )
				{
					mb::Polygon* vpoly = vert->getPolygon( k );
					require( vpoly );
					require( vpoly->index() >= 0 && vpoly->index() < mesh->getNumFaces() );
					Face& vface = mesh->faces[ vpoly->index() ];

					if ( 0 != (face.smGroup & vface.smGroup) || poly == vpoly )
					{
						Vector3 vpolyn;
						vpoly->getNormal( &vpolyn.x, &vpolyn.y, &vpolyn.z );
						vn += vpolyn;
					}
				}

				// normalize
				float lensqr = vn.lengthSquared();
				if ( lensqr > Float::MIN_VALUE )
					vn *= 1.f / Math::sqrt(lensqr);
				else
					vn = Vector3(0,0,0);

				vmad->addValue( vert->index(), poly->index(), vn.begin(), 3 );
			}
		}

		// re-export mesh points in non-deformed pose if Skin modifier present
		// NOTE: 3ds Mesh must not be used after this, because collapsing can invalidate it
		if ( skin )
		{
			// evaluate derived object before Skin modifier
			TimeValue time = 0;
			bool evalNext = false;
			bool evalDone = false;
			::ObjectState os;
			::Object* obj = node->GetObjectRef();
			while ( obj->SuperClassID() == GEN_DERIVOB_CLASS_ID && !evalDone )
			{
				IDerivedObject* derivedObj = static_cast<IDerivedObject*>(obj);
				for ( int modStack = 0 ; modStack < derivedObj->NumModifiers() ; ++modStack )
				{
					if ( evalNext )
					{
						os = derivedObj->Eval( time, modStack );
						evalDone = true;
						break;
					}

					Modifier* mod = derivedObj->GetModifier(modStack);
					if ( mod->ClassID() == SKIN_CLASSID )
						evalNext = true;
				}
				obj = derivedObj->GetObjRef();
			}

			// evaluate possible non-derived object
			if ( evalNext && !evalDone )
			{
				os = obj->Eval( time );
				evalDone = true;
			}

			// convert to TriObject and get points
			if ( evalDone && os.obj->CanConvertToType( Class_ID(TRIOBJ_CLASS_ID,0) ) )
			{
				Debug::println( "  Evaluating object {0} before Skin modifier", nodeName );

				// get TriObject
				std::auto_ptr<TriObject> triAutoDel(0);
				TriObject* tri = static_cast<TriObject*>( os.obj->ConvertToType( time, Class_ID(TRIOBJ_CLASS_ID,0) ) );
				if ( tri != os.obj )
					triAutoDel = std::auto_ptr<TriObject>( tri );

				// get mesh points before Skin is applied
				//Debug::println( "  Original collapsed mesh has {0} points, before Skin modifier {1} points", mesh->getNumVerts(), tri->mesh.getNumVerts() );
				require( gm->vertices() == tri->mesh.getNumVerts() );
				Mesh* mesh = &tri->mesh;
				int vertices = mesh->getNumVerts();
				for ( int vi = 0 ; vi < vertices ; ++vi )
				{
					Point3 v = vertexTM * mesh->verts[vi];
					mb::Vertex* vert = gm->getVertex( vi );
					vert->setPosition( v.x, v.y, v.z );
				}
			}
		}

		// split vertices with discontinuous vertex map values
		for ( int vmi = 0 ; vmi < gm->discontinuousVertexMaps() ; ++vmi )
		{
			mb::DiscontinuousVertexMap* vmad = gm->getDiscontinuousVertexMap( vmi );
			gm->splitVertexDiscontinuities( vmad );
		}

		// find base texcoord layer
		mb::DiscontinuousVertexMap* texcoords = 0;
		for ( int i = 0 ; i < gm->discontinuousVertexMaps() ; ++i )
		{
			mb::DiscontinuousVertexMap* vmad = gm->getDiscontinuousVertexMap(i);
			if ( vmad->dimensions() == 2 && vmad->format() == mb::VertexMapFormat::VERTEXMAP_TEXCOORD )
			{
				texcoords = vmad;
				break;
			}
		}
		if ( !texcoords )
			Debug::printlnError( "Object {0} must have texture coordinates", gm->name );
			// requires identification of footsteps in MySceneExport::isExportableGeometry
			//throw IOException( Format("Object {0} must have texture coordinates", gm->name) );

		// optimize
		gm->removeUnusedVertices();

		// cleanup export interfaces
#ifdef SGEXPORT_PHYSIQUE
		if ( mcExport )
		{
			require( phyExport );
			phyExport->ReleaseContextInterface( mcExport );
			mcExport = 0;
		}
		if ( phyExport )
		{
			require( phyMod );
			phyMod->ReleaseInterface( I_PHYINTERFACE, phyExport );
			phyExport = 0;
		}
#endif // SGEXPORT_PHYSIQUE
		if ( skin )
		{
			skinMod->ReleaseInterface( I_SKIN, skin );
			skin = 0;
			skinMod = 0;
		}
	}
	catch ( ... )
	{
		// cleanup export interfaces
#ifdef SGEXPORT_PHYSIQUE
		if ( mcExport )
		{
			require( phyExport );
			phyExport->ReleaseContextInterface( mcExport );
			mcExport = 0;
		}
		if ( phyExport )
		{
			require( phyMod );
			phyMod->ReleaseInterface( I_PHYINTERFACE, phyExport );
			phyExport = 0;
		}
#endif // SGEXPORT_PHYSIQUE
		if ( skin )
		{
			skinMod->ReleaseInterface( I_SKIN, skin );
			skin = 0;
			skinMod = 0;
		}
		throw;
	}
}

static void extractShapeGeometry( GmModel* gm, INode* node, ShapeObject* shape )
{
	PolyShape polyShape;
	shape->MakePolyShape( 0, polyShape, PSHAPE_BUILTIN_STEPS, FALSE );
	Debug::println( "Extracted PolyShape({0}) from {1}", polyShape.numLines, gm->name );

	// vertex transform to left-handed system
	Matrix3 pivot = TmUtil::getPivotTransform( node );
	Matrix3 vertexTM = pivot * s_convtm;

	for ( int i = 0 ; i < polyShape.numLines ; ++i )
	{
		PolyLine& polyLine = polyShape.lines[i];
		Debug::println( "Extracted PolyLine({0}) from {1}", polyLine.numPts, gm->name );
		
		P(GmLineList) lineList = new GmLineList;
		for ( int k = 0 ; k < polyLine.numPts ; ++k )
		{
			Point3 pt = vertexTM * polyLine.pts[k].p;
			lineList->points.add( Vector3(pt.x,pt.y,pt.z) );
		}

		gm->lineLists.add( lineList );
	}
}

static void extractPatchGeometry( GmModel* gm, INode* node, PatchMesh* patchmesh, Mtl* material )
{
	patchmesh->computeInteriors();

	// patch material
	if ( material )
		gm->materials.add( GmUtil::createGmMaterial(material) );

	// vertex transform to left-handed system
	Matrix3 pivot = TmUtil::getPivotTransform( node );
	bool insideOut = TmUtil::hasNegativeParity( pivot );
	Matrix3 vertexTM = pivot * s_convtm;

	Point3 p[4][4];
	for ( int k = 0 ; k < patchmesh->numPatches ; ++k )
	{
		const Patch&	src		= patchmesh->patches[k];
		PatchVec*		vecs	= patchmesh->vecs;
		PatchVert*		verts	= patchmesh->verts;

		if ( src.type == PATCH_QUAD )
		{
			// topleft->bottomleft
			p[0][0] = verts[src.v[0]].p;
			p[1][0] = vecs[src.vec[0]].p;
			p[2][0] = vecs[src.vec[1]].p;
			// bottomleft->bottomright
			p[3][0] = verts[src.v[1]].p;
			p[3][1] = vecs[src.vec[2]].p;
			p[3][2] = vecs[src.vec[3]].p;
			// bottomright->topright
			p[3][3] = verts[src.v[2]].p;
			p[2][3] = vecs[src.vec[4]].p;
			p[1][3] = vecs[src.vec[5]].p;
			// topright->topleft
			p[0][3] = verts[src.v[3]].p;
			p[0][2] = vecs[src.vec[6]].p;
			p[0][1] = vecs[src.vec[7]].p;
			// interior points (topleft,bottomleft,bottomright,topright)
			p[1][1] = vecs[src.interior[0]].p;
			p[2][1] = vecs[src.interior[1]].p;
			p[2][2] = vecs[src.interior[2]].p;
			p[1][2] = vecs[src.interior[3]].p;

			// store patch points
			GmPatch dst;
			for ( int i = 0 ; i < 4 ; ++i )
			{
				for ( int j = 0 ; j < 4 ; ++j )
				{
					Vector3 point = toVector3( vertexTM * p[i][j] );
					dst.setControlPoint( i, j, point );
				}
			}
			gm->patches.add( dst );

			// debug info (dst)
			/*for ( int i = 0 ; i < 4 ; ++i )
			{
				for ( int j = 0 ; j < 4 ; ++j )
				{
					Vector3 point = dst.getControlPoint(i,j);
					Debug::println( "p({0},{1}) = ({2} {3} {4})", i, j, point.x, point.y, point.z );
				}
			}*/
		}
	}
}

//-----------------------------------------------------------------------------

P(GmModel) GmUtil::createGmModel( INode* node, Mesh* mesh, PatchMesh* patchmesh, ShapeObject* shape, Mtl* material )
{
	P(GmModel) gm = new GmModel;

	// My Object -> My_Object.gm
	gm->node3ds = node;
	gm->name = node->GetName();
	gm->filename = getGmFilename( gm->name );

	// extract geometry
	if ( mesh )
		extractTriangleGeometry( gm, node, mesh, material );
	if ( patchmesh )
		extractPatchGeometry( gm, node, patchmesh, material );
	if ( shape )
		extractShapeGeometry( gm, node, shape );
	
	return gm;
}

P(GmMaterial) GmUtil::createGmMaterial( Mtl* material, Mtl* bakedmaterial )
{
	require( material );

	P(GmMaterial) s = new GmMaterial;

	// get name
	static int unnamedCount = 0;
	if ( material->GetName().data() )
		s->name = material->GetName().data();
	else
		s->name = "noname #"+String::valueOf( ++unnamedCount );

	// Standard material (+Diffuse) (+ Reflection)
	if ( material->ClassID() == Class_ID(DMTL_CLASS_ID,0) )
	{
		StdMat* stdmat		= static_cast<StdMat*>(material);
	    StdMat* bakedmat	= static_cast<StdMat*>(bakedmaterial);

		// StdMat2?
		StdMat2* stdmat2 = 0;
		if ( stdmat->SupportsShaders() )
			stdmat2 = static_cast<StdMat2*>( stdmat );

		// uniform transparency
		s->opacity = stdmat->GetOpacity(0);

		// self illumination
		s->selfIllum = stdmat->GetSelfIllum(0);

		// two-sided material?
		s->twosided = ( 0 != stdmat->GetTwoSided() );

		// blending mode
		s->blend = GmMaterial::BLEND_COPY;
		if ( s->opacity < 1.f )
			s->blend = GmMaterial::BLEND_MULTIPLY;
		if ( stdmat->GetTransparencyType() == TRANSP_ADDITIVE )
			s->blend = GmMaterial::BLEND_ADD;

		// diffuse color
		s->diffuseColor = toColorf( stdmat->GetDiffuse(0) );

		// specular highlights
		float shinStr = stdmat->GetShinStr(0);
		s->specular = (shinStr > 0.f);
		if ( s->specular )
		{
			float shininess = stdmat->GetShininess(0);
			s->specularExponent = Math::pow( 2.f, shininess*10.f + 2.f );
			s->specularColor = toColorf( stdmat->GetSpecular(0) ) * shinStr;
		}
		if ( bakedmat )
		{
			shinStr = bakedmat->GetShinStr(0);
			s->specular = (shinStr > 0.f);
			if ( s->specular )
			{
				float shininess = bakedmat->GetShininess(0);
				s->specularExponent = Math::pow( 2.f, shininess*10.f + 2.f );
				s->specularColor = toColorf( bakedmat->GetSpecular(0) ) * shinStr;
			}
		}

		// diffuse texture layer
		BitmapTex* tex	= SceneExportUtil::getStdMatBitmapTex( stdmat, ID_DI );
		if ( tex )
		{
			GmMaterial::TextureLayer& layer = s->diffuseLayer;
			setLayerTex( layer, tex, s->name );
		}

		// opacity texture layer
		tex = SceneExportUtil::getStdMatBitmapTex( stdmat, ID_OP );
		if ( tex )
		{
			GmMaterial::TextureLayer& layer = s->opacityLayer;
			setLayerTex( layer, tex, s->name );

			// check alpha channel validity
			Bitmap* bmp = tex->GetBitmap(0);
			if ( bmp && !bmp->HasAlpha() )
				Debug::printlnError( "Material \"{0}\" opacity map \"{1}\" must have image alpha channel.", s->name, tex->GetMapName() );
				//throw IOException( Format("Material \"{0}\" opacity map \"{1}\" must have image alpha channel.", s->name, tex->GetMapName()) );
			s->blend = GmMaterial::BLEND_MULTIPLY;

			// check that opacity map is the same as diffuse map
			if ( s->opacityLayer.filename != s->diffuseLayer.filename )
				throw IOException( Format("Material \"{0}\" diffuse bitmap needs to be the same in opacity map.(diffuse map is \"{1}\" and opacity map is \"{2}\")", s->name, s->diffuseLayer.filename, s->opacityLayer.filename) );
			if ( s->opacityLayer.coordset != s->diffuseLayer.coordset )
				throw IOException( Format("Material \"{0}\" diffuse map texture coordinate set needs to be the same in opacity map.", s->name) );
			if ( s->opacityLayer.env != s->diffuseLayer.env )
				throw IOException( Format("Material \"{0}\" diffuse map texture coordinate generator needs to be the same in opacity map.", s->name) );
		}

		// reflection texture layer
		tex = SceneExportUtil::getStdMatBitmapTex( stdmat, ID_RL );
		if ( tex )
		{
			GmMaterial::TextureLayer& layer = s->reflectionLayer;
			setLayerTex( layer, tex, s->name );
		}

		// glossiness (shininess strength, SS) texture layer
		tex = SceneExportUtil::getStdMatBitmapTex( stdmat, ID_SS );
		if ( tex )
		{
			GmMaterial::TextureLayer& layer = s->glossinessLayer;
			setLayerTex( layer, tex, s->name );

			// check alpha channel validity
			Bitmap* bmp = tex->GetBitmap(0);
			//if ( bmp && !bmp->HasAlpha() )
			//	throw IOException( Format("Material \"{0}\" glossiness map \"{1}\" must have image alpha channel.", s->name, tex->GetMapName()) );
			if ( bmp && !bmp->HasAlpha() )
				Debug::printlnError("Material \"{0}\" glossiness map \"{1}\" must have image alpha channel.", s->name, tex->GetMapName() );

			// check that glossiness map is the same as diffuse map
			if ( s->glossinessLayer.filename != s->diffuseLayer.filename )
				throw IOException( Format("Material \"{0}\" diffuse bitmap needs to be the same in glossiness map.(diffuse map is \"{1}\" and glossiness map is \"{2}\")", s->name, s->diffuseLayer.filename, s->glossinessLayer.filename) );
			if ( s->glossinessLayer.coordset != s->diffuseLayer.coordset )
				throw IOException( Format("Material \"{0}\" diffuse map texture coordinate set needs to be the same in glossiness map.", s->name) );
			if ( s->glossinessLayer.env != s->diffuseLayer.env )
				throw IOException( Format("Material \"{0}\" diffuse map texture coordinate generator needs to be the same in glossiness map.", s->name) );

			// check that reflection map has been set
			if ( s->reflectionLayer.filename.length() == 0 )
				throw IOException( Format("Material \"{0}\" glossiness map requires reflection map to be set.", s->name) );
		}

		// bump texture layer
		tex = SceneExportUtil::getStdMatBitmapTex( stdmat, ID_BU );
		if ( tex )
		{
			GmMaterial::TextureLayer& layer = s->bumpLayer;
			setLayerTex( layer, tex, s->name );
		}

		// specular color texture layer
		tex = SceneExportUtil::getStdMatBitmapTex( stdmat, ID_SP );
		if ( tex )
		{
			GmMaterial::TextureLayer& layer = s->specularColorLayer;
			setLayerTex( layer, tex, s->name );
		}

		// specular level texture layer
		tex = SceneExportUtil::getStdMatBitmapTex( stdmat, ID_SH );
		if ( tex )
		{
			GmMaterial::TextureLayer& layer = s->specularLevelLayer;
			setLayerTex( layer, tex, s->name );
		}

		// lightmap texture layer ( from self-illumination map of baked material )
		tex = SceneExportUtil::getStdMatBitmapTex( stdmat, ID_SI );
		BitmapTex* tex2 = 0;
		if ( bakedmat ) 
			tex2 = SceneExportUtil::getStdMatBitmapTex( bakedmat, ID_SI );
		
		if ( tex || tex2 )
		{
			GmMaterial::TextureLayer& layer = s->lightMapLayer;
	
			if ( tex && !tex2 )
				setLayerTex( layer, tex, s->name );
			else if ( tex2 )
				setLayerTex( layer, tex2, s->name );
		}
	}

	return s;
}

lang::String GmUtil::getGmFilename( const lang::String& nodeName )
{
	const int buflen = 50;
	Char buf[buflen+1];
	int len = 0;
	String str = "";

	for ( int i = 0 ; i < nodeName.length() ; ++i )
	{
		Char ch = nodeName.charAt(i);
		
		if ( Character::isLetterOrDigit(ch) )
			buf[len++] = ch;
		else if ( Character::isWhitespace(ch) )
			buf[len++] = '_';
		else if ( ch == '_' || ch == '-' )
			buf[len++] = ch;
		buf[len] = 0;

		if ( len >= buflen )
			str = str + buf;
	}

	if ( len > 0 )
		str = str + buf;
	return str + ".gm";
}

