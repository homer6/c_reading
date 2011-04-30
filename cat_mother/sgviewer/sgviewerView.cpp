#include "stdafx.h"
#include "sgviewer.h"
#include "sgviewerDoc.h"
#include "sgviewerView.h"
#include "sgviewer.h"
#include "printMemoryState.h"
#include "version.h"
#include <sg/LOD.h>
#include <sg/Mesh.h>
#include <sg/Light.h>
#include <sg/Model.h>
#include <sg/Scene.h>
#include <sg/Shader.h>
#include <sg/LineList.h>
#include <sg/VertexLock.h>
#include <sg/ShadowVolume.h>
#include <sg/TriangleList.h>
#include <sg/VertexAndIndexLock.h>
#include <io/FileInputStream.h>
#include <sgu/NodeUtil.h>
#include <dev/Profile.h>
#include <dev/TimeStamp.h>
#include <pix/Color.h>
#include <pix/Colorf.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <lang/Character.h>
#include <lang/Format.h>
#include <lang/Throwable.h>
#include <util/Vector.h>
#include <math/OBBox.h>
#include <math/Vector3.h>
#include <math/Intersection.h>
#include <sgu/CameraUtil.h>
#include <sgu/ContextUtil.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define MAX_LOD_LEVEL 10

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace sgu;
using namespace dev;
using namespace sgu;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

static int s_mouseEventCount = 0;

//-----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CSgviewerView, CView)

BEGIN_MESSAGE_MAP(CSgviewerView, CView)
	//{{AFX_MSG_MAP(CSgviewerView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_SYSCOMMAND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

/** Gets number of polygons in the mesh. */
static void getMeshPolygons( Mesh* mesh, 
	int* totalTriangles, int* shadowTriangles )
{
	int totalc = 0;
	int shadowc = 0;

	for ( int i = 0 ; i < mesh->primitives() ; ++i )
	{
		Primitive* prim = mesh->getPrimitive(i);
		
		Model* model = dynamic_cast<Model*>( prim );
		if ( model )
		{
			totalc += model->indices()/3;
			continue;
		}

		TriangleList* trimesh = dynamic_cast<TriangleList*>( prim );
		if ( trimesh )
		{
			totalc += trimesh->vertices()/3;
			continue;
		}

		ShadowVolume* shadowvol = dynamic_cast<ShadowVolume*>( prim );
		if ( shadowvol )
		{
			int c = shadowvol->shadowTriangles();
			shadowc += c;
			totalc += c;
			continue;
		}

		LineList* linelist = dynamic_cast<LineList*>( prim );
		if ( linelist )
		{
			totalc += linelist->lines();
			continue;
		}
	}

	if ( totalTriangles )
		*totalTriangles = totalc;
	if ( shadowTriangles )
		*shadowTriangles = shadowc;
}

//-----------------------------------------------------------------------------

CSgviewerView::CSgviewerView() :
	m_boneMatrices( Allocator<Matrix4x4>(__FILE__) ),
	m_boneMatrixPtrs( Allocator<const Matrix4x4*>(__FILE__) )
{
	m_flySpeed		= 100.f;
	m_flySpeedRot	= 50.f;
	m_statistics	= false;
	m_dragNode		= 0;
	m_dragRotate	= false;
	m_distance		= 0.f;
	m_distanceNode	= 0;
	m_distanceDirty	= true;
	m_paused		= false;
	m_dt			= 0.f;
	m_bvolVis		= false;
	m_dbgLines		= 0;
	m_dbgMesh		= 0;
	m_grabScreen	= false;
}

CSgviewerView::~CSgviewerView()
{
}

BOOL CSgviewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CSgviewerView::OnDraw(CDC* pDC)
{
	try
	{
		OnDrawImpl( pDC );
	}
	catch ( Throwable& e )
	{
		Debug::printlnError( e.getMessage().format() );
	}
}

void CSgviewerView::printNodeTree( Node* root, int margin )
{
	String mg = "";
	for ( int i = 0 ; i < margin ; ++i )
		mg = mg + " ";

	for ( Node* node = root->firstChild() ; node ; node = root->getNextChild(node) )
	{
		Vector3 pos = node->worldTransform().translation();
		Debug::println( "{0}{1} ({2} {3} {4})", mg, node->name(), pos.x, pos.y, pos.z );
		
		printNodeTree( node, margin+2 );
	}
}

void CSgviewerView::OnDrawImpl( CDC* )
{
	try
	{
		draw();
	}
	catch ( Throwable& e )
	{
		CSgviewerApp& app = CSgviewerApp::getApp();
		Context* context = app.context;
		if ( context )
			context->endScene();

		Debug::println( "Error: {0}", e.getMessage().format() );
	}
}

void CSgviewerView::draw()
{
	CSgviewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// debug line buffer
	if ( !m_dbgMesh )
		m_dbgMesh = new Mesh;
	if ( !m_dbgLines )
	{
		m_dbgLines = new LineList( CSgviewerApp::getApp().debugLines(), LineList::LINES_3D );
		m_dbgMesh->addPrimitive( m_dbgLines );
		m_dbgLines->shader()->setPass( 6 );
	}
	m_dbgLines->removeLines();

	// delta time
	long curTime = System::currentTimeMillis();
	float dt0 = 1e-3f * (float)(curTime - m_prevTime);
	m_prevTime = curTime;
	
	// filter time changes
	float dt = dt0 * .5f + m_dt * .5f;
	m_dt = dt0;

	// check that we can render
	CSgviewerApp& app = CSgviewerApp::getApp();
	Context* context = app.context;
	if ( !context || !context->ready() || app.pause() )
		return;

	Camera* camera	= pDoc->activeCamera();
	Node*	root	= pDoc->scene;
	Scene*	scene	= dynamic_cast<Scene*>( root );

	if ( camera && scene )
	{
		if ( GetKeyState('O') < 0 )
		{
			Debug::println( "Scene graph -------------------------------------------------------------" );
			printNodeTree( root );
		}

		// default light?
		if ( pDoc->defaultLight && GetKeyState('P') < 0 )
			pDoc->defaultLight->setTransform( camera->transform() );

		// playback mode
		if ( GetKeyState('R') < 0 )
			m_time = dt = 0.f;

		// fly mode
		float deltaPos = m_flySpeed / 3.6f * dt;
		float deltaRot = Math::toRadians(m_flySpeedRot) * dt;
		Vector3 pos = camera->transform().translation();
		Vector3 dx = camera->transform().rotation().getColumn(0) * deltaPos;
		Vector3 dy = camera->transform().rotation().getColumn(1) * deltaPos;
		Vector3 dz = camera->transform().rotation().getColumn(2) * deltaPos;
		Matrix3x3 rot = camera->transform().rotation();
		bool shiftDown = (GetKeyState(VK_SHIFT) < 0);

		for ( int i = 0 ; i < app.movementSpeeds() ; ++i )
			if ( GetKeyState('1'+i) < 0 )
				m_flySpeed = app.getMovementSpeed(i);

		for ( int i = 0 ; i < app.rotationSpeeds() ; ++i )
			if ( GetKeyState('1'+i) < 0 )
				m_flySpeedRot = app.getRotationSpeed(i);

		if ( GetKeyState('W') < 0 )
		{
			pDoc->selectFlyCamera();
			pos += dz;
			m_distanceDirty = true;
		}
		if ( GetKeyState('S') < 0 )
		{
			pDoc->selectFlyCamera();
			pos -= dz;
			m_distanceDirty = true;
		}
		if ( GetKeyState('A') < 0 || 
			(GetKeyState(VK_LEFT) < 0 && shiftDown) )
		{
			pDoc->selectFlyCamera();
			pos -= dx;
			m_distanceDirty = true;
		}
		if ( GetKeyState('D') < 0 ||
			(GetKeyState(VK_RIGHT) < 0 && shiftDown) )
		{
			pDoc->selectFlyCamera();
			pos += dx;
			m_distanceDirty = true;
		}
		if ( GetKeyState(VK_UP) < 0 && shiftDown )
		{
			pDoc->selectFlyCamera();
			pos += dy;
			m_distanceDirty = true;
		}
		if ( GetKeyState(VK_DOWN) < 0 && shiftDown )
		{
			pDoc->selectFlyCamera();
			pos -= dy;
			m_distanceDirty = true;
		}
		if ( GetKeyState(VK_LEFT) < 0 && !shiftDown )
		{
			pDoc->selectFlyCamera();
			rot *= Matrix3x3(Vector3(0,1,0), -deltaRot);
			m_distanceDirty = true;
		}
		if ( GetKeyState(VK_RIGHT) < 0 && !shiftDown )
		{
			pDoc->selectFlyCamera();
			rot *= Matrix3x3(Vector3(0,1,0), deltaRot);
			m_distanceDirty = true;
		}
		if ( GetKeyState(VK_UP) < 0 && !shiftDown )
		{
			pDoc->selectFlyCamera();
			rot *= Matrix3x3(Vector3(1,0,0), deltaRot);
			m_distanceDirty = true;
		}
		if ( GetKeyState(VK_DOWN) < 0 && !shiftDown )
		{
			pDoc->selectFlyCamera();
			rot *= Matrix3x3(Vector3(1,0,0), -deltaRot);
			m_distanceDirty = true;
		}
		// eliminate banking
		rot.setColumn( 0, rot.getColumn(0) - Vector3(0,1,0)*rot.getColumn(0).dot(Vector3(0,1,0)) );
		camera->setPosition( pos );
		camera->setRotation( rot.orthonormalize() );

		// ambient control
		if ( GetKeyState('M') < 0 )
		{
			int b = 2;
			if ( GetKeyState(VK_SHIFT) < 0 )
				b = -b;
			Color amb = scene->ambientColor();
			if ( amb.red()+b <= 255 && amb.red()+b >= 0 )
				 amb += Color(b,b,b);
			scene->setAmbientColor( Color(amb) );
		}

		// slow motion, rewind
		if ( GetKeyState('L') < 0 )
			dt *= .25f;
		if ( GetKeyState('B') < 0 )
			dt *= -1.f;

		// update animations
		if ( !m_paused )
		{
			m_time += dt;
			for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
			{
				node->setState( m_time );

				// animate Mesh primitives
				Mesh* mesh = dynamic_cast<Mesh*>( node );
				if ( mesh )
				{
					for ( int i = 0 ; i < mesh->primitives() ; ++i )
						mesh->getPrimitive(i)->setState( m_time );
				}
			}
			pDoc->flyCamera()->setTransform( camera->transform() );
		}

		// set camera front/back planes
		CameraUtil::setFrontAndBackPlanes( camera, scene );
		if ( app.front() > 0.f )
			camera->setFront( app.front() );
		if ( app.back() > 0.f )
			camera->setBack( app.back() );
	}

	// statistics
	int	buildSilhuettes = ShadowVolume::buildSilhuettes();
	int clippedSilhuetteQuads = ShadowVolume::clippedSilhuetteQuads();
	int	clippedTriangleVolumes = ShadowVolume::clippedTriangleVolumes();
	int	volumeCapPolygons = ShadowVolume::volumeCapPolygons();
	int renderedShadowTriangles = ShadowVolume::renderedShadowTriangles();
	int processedObjects = 0;
	int renderedLights = 0;
	int renderedObjects = 0;
	int renderedPrimitives = 0;
	int	renderedTriangles = 0;
	int totalPrimitives = 0;
	int totalTriangles = 0;
	int activeLODs[MAX_LOD_LEVEL];
	for ( int k = 0 ; k < MAX_LOD_LEVEL ; ++k )
		activeLODs[k] = 0;
	if ( camera )
		camera->resetStatistics();

	// compute fps
	static TimeStamp t0 = TimeStamp();
	TimeStamp t1 = TimeStamp();
	float sec = (t1-t0).seconds();
	t0 = t1;
	float fps = 0.f;
	if ( sec > Float::MIN_VALUE )
		fps = 1.f / sec;

	// visualize bones
	if ( scene && app.prop().getBoolean("DrawBones") )
	{
		Vector3 offs = camera->worldTransform().rotation().getColumn(0) * app.prop().getFloat("DrawSkinOffset");
		VertexLock<LineList> dbgLinesLock( m_dbgLines, LineList::LOCK_WRITE );
		for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
		{
			if ( node->name().indexOf("Bip") >= 0 )
			{
				Vector3 w0 = node->worldTransform().translation();
				w0 += offs;
				for ( Node* child = node->firstChild() ; child ; child = node->getNextChild(child) )
				{
					Vector3 w1 = child->worldTransform().translation();
					w1 += offs;
					m_dbgLines->addLine( w0, w1, Color(0,255,0) );

					Vector3 dx = child->rotation().getColumn(0)*.05f;
					m_dbgLines->addLine( w1-dx, w1+dx, Color(255,0,0) );
				}
			}
		}
	}

	// visualize skin
	if ( scene && app.prop().getBoolean("DrawSkin") )
	{
		Vector3 offs = camera->worldTransform().rotation().getColumn(0) * app.prop().getFloat("DrawSkinOffset");
		scene->validateHierarchy();
		for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh && mesh->bones() > 0 )
			{
				Vector<Matrix4x4> bonetm( Allocator<Matrix4x4>(__FILE__) );
				bonetm.setSize( mesh->bones()+1 );
				mesh->getBoneMatrix4x3Array( bonetm.begin(), bonetm.size() );

				for ( int i = 0 ; i < mesh->primitives() ; ++i )
				{
					Model* model = dynamic_cast<Model*>( mesh->getPrimitive(i) );
					if ( model && model->vertexFormat().weights() > 0 )
					{
						VertexAndIndexLock<Model> lk( model, Model::LOCK_READ );
						const int* usedBoneArray = model->usedBoneArray();
						const int usedBones = model->usedBones();
						float* vdata;
						int vpitch;
						model->getVertexPositionData( &vdata, &vpitch );
						
						for ( int k = 0 ; k < model->indices() ; k += 3 )
						{
							int ind[3];
							model->getIndices( k, ind, 3 );

							Vector3 wp[3];
							for ( int j = 0 ; j < 3 ; ++j )
							{
								float weights[4] = {0,0,0,0};
								int bones[4] = {0,0,0,0};
								float* v = vdata + ind[j]*vpitch;
								Vector3 op( v[0], v[1], v[2] );

								int nb = model->getVertexWeights( ind[j], bones, weights, 4 );

								wp[j] = Vector3(0,0,0);
								for ( int n = 0 ; n < nb ; ++n )
								{
									int bx = (int)bones[n];
									assert( bx >= 0 && bx < usedBones );
									bx = usedBoneArray[bx];
									Matrix4x4 m = bonetm[bx];
									Vector3 r0( m(0,0), m(0,1), m(0,2) );
									Vector3 r1( m(1,0), m(1,1), m(1,2) );
									Vector3 r2( m(2,0), m(2,1), m(2,2) );
									Vector3 r3( m(3,0), m(3,1), m(3,2) );
									wp[j] += Vector3( r0.dot(op)+r3.x, r1.dot(op)+r3.y, r2.dot(op)+r3.z ) * weights[n];
								}
							}

							m_dbgLines->addLine( wp[0]+offs, wp[1]+offs, Color(0,0,128) );
						}
					}
				}
			}
		}
	}

	// visualize lights
	if ( scene && app.prop().getBoolean("DrawLights") )
	{
		VertexLock<LineList> dbgLinesLock( m_dbgLines, LineList::LOCK_WRITE );
		for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
		{
			Light* light = dynamic_cast<Light*>( node );
			if ( light )
			{
				Matrix4x4 tm = light->worldTransform();
				Vector3 t = tm.translation();
				Matrix3x3 r = tm.rotation();
				m_dbgLines->addLine( t, t+r.getColumn(0), Color(255,255,0) );
				m_dbgLines->addLine( t, t+r.getColumn(1), Color(255,255,0) );
				m_dbgLines->addLine( t, t+r.getColumn(2), Color(255,255,128) );
			}
		}
	}

	// visualize normals
	if ( scene && app.prop().getBoolean("DrawNormals") )
	{
		VertexLock<LineList> dbgLinesLock( m_dbgLines, LineList::LOCK_WRITE );
		for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh )
			{
				Matrix4x4 tm = mesh->worldTransform();
				for ( int i = 0 ; i < mesh->primitives() ; ++i )
				{
					Model* model = dynamic_cast<Model*>( mesh->getPrimitive(i) );
					if ( model )
					{
						VertexLock<Model> lk( model, Model::LOCK_READ );
						for ( int k = 0 ; k < model->vertices() ; ++k )
						{
							Vector3 p, n;
							model->getVertexNormals( k, &n );
							model->getVertexPositions( k, &p );
							p = tm.transform(p);
							n = tm.rotate(n);
							m_dbgLines->addLine( p, p+n, Color(0,0,255) );
						}
					}
				}
			}
		}
	}

	// visualize bounding boxes
	if ( m_bvolVis && scene )
	{
		m_distanceNode = pDoc->pickNode( m_dragStartX, m_dragStartY, &m_distance );
		m_distanceDirty = false;

		VertexLock<LineList> dbgLinesLock( m_dbgLines, LineList::LOCK_WRITE );
		for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh && mesh == m_distanceNode )
			{
				for ( int i = 0 ; i < mesh->primitives() ; ++i )
				{
					Primitive* prim = mesh->getPrimitive( i );
					Model* model = dynamic_cast<Model*>( prim );

					if ( model )
					{
						OBBox box = model->boundBox();
						Vector3 v[8];
						box.getVertices( mesh->worldTransform(), v, 8 );
						int lines[] = {0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 4,0, 7,3, 5,1, 6,2};
						Color colr = Color(0,255,0);

						for ( int k = 0 ; k < sizeof(lines)/sizeof(lines[0]) ; k += 2 )
						{
							if ( m_dbgLines->lines() < m_dbgLines->maxLines() )
								m_dbgLines->addLine( v[lines[k]], v[lines[k+1]], colr );
						}
					}
				}
			}
		}
	}

	// render scene
	context->beginScene();
	if ( camera && scene ) 
	{
		Profile pr( "camera.render" );
		m_dbgMesh->linkTo( scene );

		// set shader parameters
		scene->validateHierarchy();
		for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
		{
			Mesh* mesh = dynamic_cast<Mesh*>( node );
			if ( mesh )
			{
				for ( int i = 0 ; i < mesh->primitives() ; ++i )
				{
					Primitive* prim = mesh->getPrimitive( i );
					Shader* shader = prim->shader();
					if ( shader )
						setShaderParams( prim, shader, mesh, camera, scene );
				}
			}
		}

		camera->render();
	}

	// render text
	Font* font = CSgviewerApp::getApp().font;
	if ( font )
	{
		if ( CSgviewerApp::getApp().stats() )
		{
			// DEBUG mark
			#ifdef _DEBUG
			font->drawText( 4, context->height()-font->height(), "DEBUG", 0, 0 );
			#endif

			// rendering statistics
			if ( camera )
			{
				Profile pr( "stats" );

				// general
				processedObjects = camera->processedObjects();
				renderedLights = camera->renderedLights();
				renderedObjects = camera->renderedObjects();
				renderedPrimitives = camera->renderedPrimitives();
				renderedTriangles = camera->renderedTriangles();
				
				// shadows
				buildSilhuettes = ShadowVolume::buildSilhuettes() - buildSilhuettes;
				clippedSilhuetteQuads = ShadowVolume::clippedSilhuetteQuads() - clippedSilhuetteQuads;
				clippedTriangleVolumes = ShadowVolume::clippedTriangleVolumes() - clippedTriangleVolumes;
				volumeCapPolygons = ShadowVolume::volumeCapPolygons() - volumeCapPolygons;
				renderedShadowTriangles = ShadowVolume::renderedShadowTriangles() - renderedShadowTriangles;

				// compute number of triangles / primitives
				for ( Node* node = camera->root() ; node ; node = node->nextInHierarchy() )
				{
					Mesh* mesh = dynamic_cast<Mesh*>( node );
					if ( mesh )
					{
						// update lod counts
						Node* parent = mesh->parent();
						LOD* lod = dynamic_cast<LOD*>( parent );
						if ( lod )
						{
							int level = lod->level();
							if ( -1 != level && 
								lod->get(level) == mesh &&
								mesh->renderedInLastFrame() )
							{
								int id = -1;
								
								// get lod level index from object name
								int tag = mesh->name().indexOf( "_LOD" );
								if ( -1 != tag && 
									tag+4 < mesh->name().length() &&
									Character::isDigit( mesh->name().charAt(tag+4) ) )
								{
									id = mesh->name().charAt(tag+4) - '0';
								}

								if ( id >= 0 && id < MAX_LOD_LEVEL )
									activeLODs[id]++;
							}
						}

						// update triangle / primitive counts
						int totalTris;
						getMeshPolygons( mesh, &totalTris, 0 );
						totalPrimitives += mesh->primitives();
						totalTriangles += totalTris;
					}
				}
			}

			float x = 4.f;
			float y = 4.f;

			// statistics
			if ( camera ) 
				font->drawText( x, y, Format("front/back {0,#.00} / {1,#.00}", camera->front(), camera->back()).format(), 0, &y );
			font->drawText( x, y, Format("shadow tri {0,#}", renderedShadowTriangles).format(), 0, &y );
			font->drawText( x, y, Format("tri {0,#} / {1,#}", renderedTriangles, totalTriangles).format(), 0, &y );
			font->drawText( x, y, Format("pri {0,#} / {1,#}", renderedPrimitives, totalPrimitives).format(), 0, &y );
			font->drawText( x, y, Format("obj {0,#} / {1,#}", renderedObjects, processedObjects).format(), 0, &y );
			font->drawText( x, y, Format("lts {0,#}", renderedLights).format(), 0, &y );
			for ( int k = 0 ; k < MAX_LOD_LEVEL ; ++k )
			{
				if ( activeLODs[k] != 0 )
					font->drawText( x, y, Format("lod{0,#} {1,#}", k, activeLODs[k]).format(), 0, &y );
			}

			// distance to mouse cursor position
			if ( !m_distanceDirty && m_distanceNode )
			{
				int polys = 0;
				int shadowTriangles = 0;
				int prims = 0;
				float diameter = 0;
				Node* node = m_distanceNode;
				Mesh* mesh = dynamic_cast<Mesh*>( node );
				if ( mesh )
				{
					getMeshPolygons( mesh, &polys, &shadowTriangles );
					prims = mesh->primitives();
					LOD* lod = dynamic_cast<LOD*>( mesh->parent() );
					if ( lod )
						diameter = camera->getProjectedSize( lod->cachedDistanceToCamera(), lod->radius()*2.f );
				}
				font->drawText( x, context->height()-font->height(), Format("distance: {0} ({1}, {2,#} polys, {5,#} shadow polys, {3,#} primitives, lod {4,#} pixels)", m_distance, m_distanceNode->name(), polys, prims, diameter, shadowTriangles).format(), 0, &y );
			}

			// cube intersection status
			if ( pDoc->cube1 && pDoc->cube2 )
			{
				Vector3 dim1 = pDoc->cube1dim;
				Matrix4x4 tm1 = pDoc->cube1->worldTransform();
				Vector3 dim2 = pDoc->cube2dim;
				Matrix4x4 tm2 = pDoc->cube2->worldTransform();
				
				Matrix4x4 tm = tm1.inverse() * tm2;
				bool overlap = Intersection::testBoxBox( dim1, tm, dim2 );
				font->drawText( x, y, Format("boxes overlap: {0,#}", overlap).format(), 0, &y );
			}

			// profiled blocks
			if ( CSgviewerApp::getApp().profiling() )
			{
				font->drawText( x, y, Format(" ").format(), 0, &y );
				font->drawText( x, y, Format("Profiled blocks:").format(), 0, &y );
				for ( int i = 0 ; i < Profile::count() ; ++i )
				{
					Profile::BlockInfo* pb = Profile::get(i);
					font->drawText( x, y, Format("  {0}: count={1,#}x, time={2,#.###} ms", 
						pb->name(), pb->count(), pb->time()*1e3f).format(), 0, &y );
				}
				Profile::reset();
			}
		}
	}

	// fps
	if ( font )
		font->drawText( context->width()-font->getWidth("fps 123456"), 4, Format("fps {0,#.#}", fps).format(), 0, 0 );

	context->endScene();

	// grab screen
	if ( m_grabScreen )
	{
		ContextUtil::grabScreen( "jpg" );
		m_prevTime = System::currentTimeMillis();
		m_grabScreen = false;
	}
	
	// update screen
	context->present();
	context->clear();

	// invalidate frame
	InvalidateRect( 0, FALSE );
}

void CSgviewerView::OnInitialUpdate()
{
	m_prevTime = System::currentTimeMillis();
	m_time = 0.f;
	m_statistics = true;
	m_dragNode = 0;
}

#ifdef _DEBUG
void CSgviewerView::AssertValid() const
{
	CView::AssertValid();
}

void CSgviewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSgviewerDoc* CSgviewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSgviewerDoc)));
	return (CSgviewerDoc*)m_pDocument;
}
#endif //_DEBUG

BOOL CSgviewerView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
	//return CView::OnEraseBkgnd(pDC);
}

void CSgviewerView::OnKeyDown( UINT ch, UINT rep, UINT flags )
{
	CSgviewerDoc* doc = GetDocument();
	ASSERT_VALID(doc);

	if ( 'T' == ch || VK_F12 == ch )
	{
		CSgviewerApp::getApp().setStats( !CSgviewerApp::getApp().stats() );
	}
	else if ( VK_ESCAPE == ch )
	{
		CSgviewerApp::getApp().m_pMainWnd->DestroyWindow();
	}
	else if ( VK_DELETE == ch )
	{
		P(Node) node = doc->pickNode( m_dragStartX, m_dragStartY );
		if ( node )
			node->unlink();
	}
	else if ( VK_PAUSE == ch )
	{
		m_paused = !m_paused;
	}
	else if ( 'I' == ch )
	{
		m_bvolVis = !m_bvolVis;
	}
	else if ( VK_F12 == ch )
	{
		printMemoryState();
	}
	else if ( VK_F9 == ch )
	{
		m_grabScreen = true;
	}
}

void CSgviewerView::OnSysCommand( UINT nID, LPARAM lParam )
{
	switch ( nID )
	{
	case SC_CONTEXTHELP:
	case SC_DEFAULT:
	case SC_HOTKEY:
	case SC_HSCROLL:
	case SC_KEYMENU:
	case SC_MAXIMIZE:
	case SC_MINIMIZE:
	case SC_MONITORPOWER:
	case SC_MOUSEMENU:
	case SC_MOVE:
	case SC_NEXTWINDOW:
	case SC_PREVWINDOW:
	case SC_RESTORE:
	case SC_SCREENSAVE:
	case SC_SIZE:
	case SC_TASKLIST:
	case SC_VSCROLL:
		if ( CSgviewerApp::getApp().context && CSgviewerApp::getApp().context->fullscreen() )
			break;
	default:
		CView::OnSysCommand( nID, lParam );
		break;
	}
}

void CSgviewerView::OnLButtonDown( UINT flags, CPoint point )
{
	startDrag( flags, point );
	m_dragRotate = false;
}

void CSgviewerView::OnLButtonUp( UINT flags, CPoint point )
{
	stopDrag();
}

void CSgviewerView::OnRButtonDown( UINT flags, CPoint point )
{
	startDrag( flags, point );
	m_dragRotate = true;
}

void CSgviewerView::OnRButtonUp( UINT flags, CPoint point )
{
	stopDrag();
}

void CSgviewerView::startDrag( UINT flags, CPoint point )
{
	if ( !m_dragNode )
	{
		CSgviewerDoc* doc = GetDocument();
		assert( doc );

		RECT cr;
		GetClientRect( &cr );
		float viewX = cr.right/2;
		float viewY = cr.bottom/2;
		float x = (point.x - viewX) / viewX;
		float y = -(point.y - viewY) / viewY;
		m_dragNode = doc->pickNode( x, y, &m_dragDist );
		m_dragStartX = x;
		m_dragStartY = y;

		if ( m_dragNode )
		{
			m_dragNode->setPositionController( 0 );
			m_dragNode->setRotationController( 0 );
			m_dragNode->setScaleController( 0 );
		}
	}

	SetCapture();
}

void CSgviewerView::stopDrag()
{
	m_dragNode = 0;
	m_dragRotate = false;
	ReleaseCapture();
}

void CSgviewerView::OnMouseMove( UINT flags, CPoint point )
{
	CSgviewerDoc* doc = GetDocument();
	ASSERT_VALID(doc);
	
	RECT cr;
	GetClientRect( &cr );
	float viewX = cr.right/2;
	float viewY = cr.bottom/2;
	float x = (point.x - viewX) / viewX;
	float y = -(point.y - viewY) / viewY;
	m_distanceDirty = true;

	if ( m_dragNode )
	{
		float dx = x - m_dragStartX;
		float dy = y - m_dragStartY;
		if ( !m_dragRotate )
		{
			if ( flags & MK_SHIFT )
				doc->dragNode( dx, 0.f, dy, m_dragNode, m_dragDist );
			else
				doc->dragNode( dx, dy, 0.f, m_dragNode, m_dragDist );
		}
		else
		{
			doc->dragNodeRotate( dx, dy, m_dragNode );
		}
	}

	m_dragStartX = x;
	m_dragStartY = y;
}

void CSgviewerView::pauseTime()
{
	m_prevTime = System::currentTimeMillis();
}

void CSgviewerView::setShaderParams( Primitive* prim, Shader* fx, Mesh* mesh, Camera* camera, Node* root )
{
	Vector3 cameraWPos = camera->worldTransform().translation();
	Matrix4x4 worldTm = mesh->worldTransform();
	Matrix4x4 worldTmInv = worldTm.inverse();
	Matrix4x4 projTm = camera->projectionTransform();
	Matrix4x4 viewTm = camera->viewTransform();
	Matrix4x4 viewProjTm = camera->viewProjectionTransform();

	// build bone matrices
	m_boneMatrices.setSize( mesh->bones()+1 );
	mesh->getBoneMatrix4x3Array( m_boneMatrices.begin(), m_boneMatrices.size() );

	bool shaderBonesSet = false;
	for ( int i = 0 ; i < fx->parameters() ; ++i )
	{
		Shader::ParameterDesc desc;
		fx->getParameterDesc( i, &desc );
		
		if ( desc.name == "mWorldA" )
		{
			shaderBonesSet = true;
			int usedBones = prim->usedBones();
			if ( usedBones == 0 )
				Debug::printlnError( "Shader {0} uses bones (mWorldA), but mesh {1} doesn't have bones", fx->name(), mesh->name() );
			if ( usedBones > desc.elements )
				Debug::printlnError( "Shader {0} supports {1} bones, but mesh {2} uses {3} bones", fx->name(), desc.elements, mesh->name(), usedBones );
		}
		else if ( desc.dataType == Shader::PT_TEXTURE && desc.name == "tNormalize" )
			fx->setTexture( "tNormalize", CSgviewerApp::getApp().normalizerCubeMap );
		else if ( desc.dataType == Shader::PT_FLOAT && desc.dataClass == Shader::PC_VECTOR4 && 
			desc.name.length() > 0 && Character::isLowerCase(desc.name.charAt(0)) &&
			desc.name.indexOf("Camera") == -1 )
		{
			bool paramResolved = false;
			bool objSpace = ( desc.name.length() > 2 && desc.name.charAt(1) == 'o' );
			int baseNameOffset = objSpace ? 2 : 1;

			Node* node = NodeUtil::findNodeByName( root, desc.name.substring(baseNameOffset) );
			if ( node )
			{
				Char ch = desc.name.charAt(0);
				if ( ch == 'd' )
				{
					Vector3 v = node->worldTransform().rotation().getColumn(2);
					if ( objSpace )
						v = worldTmInv.rotate(v);
					fx->setVector4( desc.name, Vector4(v.x, v.y, v.z, 0.f) );
					paramResolved = true;
				}
				if ( ch == 'p' )
				{
					Vector3 v = node->worldTransform().translation();
					if ( objSpace )
						v = worldTmInv.transform(v);
					fx->setVector4( desc.name, Vector4(v.x, v.y, v.z, 1.f) );
					paramResolved = true;
				}
			}

			if ( !paramResolved )
			{
				Debug::printlnError( "Mesh {2} shader {0} parameter {1} could not be resolved", fx->name(), desc.name, mesh->name() );
			}
		}
	}

	if ( m_boneMatrices.size() > 1 && !shaderBonesSet && fx->parameters() > 0 )
		Debug::printlnError( "Mesh {1} has bones, but shader {0} doesn't use bones", fx->name(), mesh->name() );
}
