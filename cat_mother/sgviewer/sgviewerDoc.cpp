#include "stdafx.h"
#include "sgviewer.h"
#include "sgviewerDoc.h"
#include "createCube.h"
#include <io/File.h>
#include <io/DirectoryInputStreamArchive.h>
#include <sg/Mesh.h>
#include <sg/Texture.h>
#include <sg/LineList.h>
#include <sg/Scene.h>
#include <sg/Model.h>
#include <sg/TriangleList.h>
#include <sg/VertexFormat.h>
#include <sg/VertexLock.h>
#include <sg/Material.h>
#include <sg/DirectLight.h>
#include <sg/PointLight.h>
#include <sg/ShadowVolume.h>
#include <sg/ShadowShader.h>
#include <sgu/MeshUtil.h>
#include <sgu/ShadowUtil.h>
#include <sgu/SceneFile.h>
#include <sgu/CameraUtil.h>
#include <sgu/ModelFileCache.h>
#include <pix/Color.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Throwable.h>
#include <direct.h>
#include "config.h"

//-----------------------------------------------------------------------------

// Light name used to detect shadow volume scenes
static const String SHADOW_KEYLIGHT_NAME = "Light1";

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace sgu;
using namespace pix;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CSgviewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CSgviewerDoc, CDocument)
	//{{AFX_MSG_MAP(CSgviewerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

CSgviewerDoc::CSgviewerDoc() :
	cameras( Allocator<P(Camera)>(__FILE__,__LINE__) )
{
	scene = 0;
	camera = 0;
}

CSgviewerDoc::~CSgviewerDoc()
{
}

BOOL CSgviewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// clear old
	scene = 0;
	camera = 0;

	// creat new scene
	P(Scene) root = new Scene;
	scene = root.ptr();
	root->setName( "scene" );
	root->setAmbientColor( Color(40,40,40) );

	// create scene objects
	P(DirectLight) lt = new DirectLight;
	lt->setName( SHADOW_KEYLIGHT_NAME );
	lt->linkTo( scene );
	lt->setPosition( Vector3(2,2,-1) );
	lt->lookAt( scene );

	P(LineList) axisp = new LineList( 3, LineList::LINES_3D );
	{VertexLock<LineList> lineLock( axisp, LineList::LOCK_WRITE );
	axisp->addLine( Vector3(0,0,0), Vector3(1,0,0), Color(255,255,0) );
	axisp->addLine( Vector3(0,0,0), Vector3(0,1,0), Color(0,255,0) );
	axisp->addLine( Vector3(0,0,0), Vector3(0,0,1), Color(0,0,255) );}
	
	P(Mesh) axis = new Mesh;
	axis->addPrimitive( axisp );
	axis->setName( "axis" );
	axis->linkTo( scene );
	
	const float shadowLength = 50.f;
	P(Material) cubemat = new Material;
	cubemat->setDiffuseColor( Colorf(0,1,0) );
	cube1dim = Vector3( 1,1,1 ) * .5f;
	P(Model) cubep = createCube( cube1dim*2.f );
	cubemat->setVertexFormat( cubep->vertexFormat() );
	cubep->setShader( cubemat );
	P(Mesh) mesh = new Mesh;
	cube1 = mesh;
	mesh->setName( "cube1" );
	mesh->linkTo( scene );
	mesh->setPosition( Vector3(0,5,0) );
	mesh->setRotation( mesh->transform().rotation() * 1.24f );
	mesh->addPrimitive( cubep );
	mesh->addPrimitive( new ShadowVolume( cubep, mesh->worldTransform().inverse().rotation() * lt->worldDirection(), shadowLength ) );

	cubemat = new Material;
	cubemat->setDiffuseColor( Colorf(0,0,1) );
	cube2dim = Vector3( 2, 2.5f, 3.f ) * .5f;
	cubep = createCube( cube2dim*2.f );
	cubemat->setVertexFormat( cubep->vertexFormat() );
	cubep->setShader( cubemat );
	mesh = new Mesh;
	cube2 = mesh;
	mesh->setName( "cube2" );
	mesh->linkTo( scene );
	mesh->setPosition( Vector3(0,2,0) );
	mesh->addPrimitive( cubep );
	mesh->addPrimitive( new ShadowVolume( cubep, mesh->worldTransform().inverse().rotation() * lt->worldDirection(), shadowLength ) );

	P(Mesh) floor = new Mesh;
	floor->setName( "floor" );
	floor->linkTo( scene );
	P(Model) floorp = createCube( Vector3(1000.f, 0.1f, 1000.f) );
	floor->addPrimitive( floorp );
	floor->setPosition( Vector3(0,-1.1f,0) );
	P(Material) floormat = new Material;
	floormat->setDiffuseColor( Colorf(1,0,0) );
	floormat->setVertexFormat( floorp->vertexFormat() );
	floorp->setShader( floormat );
	
	P(Camera) cam = new Camera;
	cam->setHorizontalFov( Math::toRadians(90.f) );
	cam->setName( "cam" );
	cam->setPosition( Vector3(1,6,-5) );
	cam->linkTo( scene );
	cam->lookAt( mesh );

	prepareDoc();

	//CSgviewerApp::getApp().setPropPathToCwd();
	return TRUE;
}

void CSgviewerDoc::Serialize(CArchive& ar)
{
	CSgviewerApp::getApp().setPause( true );

	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		try
		{
			CFile* file = ar.GetFile();
			if ( file )
			{
				Texture::flushTextures();

				CString pathstr = file->GetFilePath();
				String path = (const char*)pathstr;
				String dir = File(path).getParent();
				P(DirectoryInputStreamArchive) arch = new DirectoryInputStreamArchive;
				arch->addPath( dir );

				P(ModelFileCache) modelcache = new ModelFileCache( arch );
				P(SceneFile) sf = new SceneFile( path, modelcache, arch );
				modelcache->clear();
				scene = sf->scene();
				name = path;
				cube1 = cube2 = 0;

				prepareDoc();
			}
		}
		catch ( Throwable& e )
		{
			char buf[1000];
			e.getMessage().format().getBytes( buf, sizeof(buf), "ASCII-7" );
			if ( CSgviewerApp::getApp().m_pMainWnd )
				CSgviewerApp::getApp().m_pMainWnd->MessageBox( buf, "Error - sgviewer", MB_OK|MB_ICONERROR|MB_SYSTEMMODAL );
			else
				MessageBox( 0, buf, "Error - sgviewer", MB_OK|MB_ICONERROR|MB_SYSTEMMODAL );
		}
	}

	CSgviewerApp::getApp().setPause( false );
}

void CSgviewerDoc::prepareDoc()
{
	CSgviewerApp& app = CSgviewerApp::getApp();

	// split large meshes
	/*for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh ) 
			MeshUtil::splitModels( mesh, app.splitPrimitivePolygons(), app.splitPrimitiveSize() );
	}*/

	// collect cameras
	camera = 0;
	cameras.clear();
	P(Camera) flyCam = new Camera;
	flyCam->setName( "fly" );
	flyCam->linkTo( scene );
	cameras.add( flyCam );
	for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Camera* cam = dynamic_cast<Camera*>( node );
		if ( cam )
			cameras.add( cam );
	}
	camera = cameras.size()-1;

	// set fly camera parameters
	Camera* curCam = cameras[camera];
	curCam->setState( 0 );
	flyCam->setHorizontalFov( curCam->horizontalFov() );

	// find 'keylight' to be used as key light
	bool hasLight = false;
	Light* keylt = 0;
	for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Light* lt0 = dynamic_cast<Light*>( node );
		if ( lt0 )
		{
			hasLight = true;
			if ( lt0 && lt0->name() == SHADOW_KEYLIGHT_NAME )
			{
				keylt = lt0;
				break;
			}
		}
	}

	// is there a keylight in the scene?
	if ( keylt )
	{
		P(Shader) shadowShader = new ShadowShader;

		// set shadow volume shaders
		ShadowUtil::setShadowVolumeShaders( scene, shadowShader );

		// create shadow filler
		P(Primitive) fillgeom = ShadowUtil::createShadowFiller( Color(0,0,0,80), 4000, 4000 );
		P(Mesh) fillmesh = new Mesh;
		fillmesh->addPrimitive( fillgeom );
		fillmesh->setName( "ShadowFiller" );
		fillmesh->linkTo( scene );
	}

	// add default light if none
	defaultLight = 0;
	if ( !hasLight )
	{
		defaultLight = new PointLight;
		defaultLight->setName( SHADOW_KEYLIGHT_NAME );
		defaultLight->linkTo( scene );
	}
		
	// replace lightmap materials with lightmap shader
	/*for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				P(Primitive) prim = mesh->getPrimitive(i);
				P(Material) mat = dynamic_cast<Material*>( prim->shader() );
				if ( mat && !mat->lighting() && mat->isTextureLayerEnabled(0) && mat->isTextureLayerEnabled(1) && !mat->isTextureLayerEnabled(2) &&
					mat->sourceBlend() == Material::BLEND_ONE && mat->destinationBlend() == Material::BLEND_ZERO )
				{
					Material::TextureArgument arg1, arg2;
					Material::TextureOperation op;
					mat->getTextureColorCombine( 0, &arg1, &op, &arg2 );
					if ( arg1 == Material::TA_TEXTURE && arg2 == Material::TA_DIFFUSE && op == Material::TOP_MODULATE )
					{
						mat->getTextureColorCombine( 1, &arg1, &op, &arg2 );
						if ( arg1 == Material::TA_TEXTURE && arg2 == Material::TA_CURRENT && op == Material::TOP_MODULATE )
						{
							Debug::println( "Replacing lightmap material {0} with shader", mat->name() );

							P(BaseTexture) dif = mat->getTexture(0);
							P(BaseTexture) lmap = mat->getTexture(1);

							P(Shader) fx = app.lightmapShader->clone();
							fx->setVertexFormat( prim->vertexFormat() );
							fx->setTexture( "tDiffuse", dif );
							fx->setTexture( "tLightMap", lmap );

							prim->setShader( fx );
						}
					}
				}
			}
		}
	}*/

	// load objects to rendering device
	/*for ( Node* node = scene ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh )
		{
			for ( int i = 0 ; i < mesh->primitives() ; ++i )
			{
				Primitive* prim = mesh->getPrimitive(i);
				prim->load();
			}
		}
	}*/
}

#ifdef _DEBUG
void CSgviewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSgviewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

Camera*	CSgviewerDoc::activeCamera() const
{
	if ( camera >= 0 && camera < cameras.size() )
		return cameras[camera];
	else
		return 0;
}

void CSgviewerDoc::selectNextCamera()
{
	if ( camera >= 0 && camera < cameras.size() )
		camera = (camera+1) % cameras.size();
}

void CSgviewerDoc::selectFlyCamera()
{
	camera = 0;
}

sg::Camera*	CSgviewerDoc::flyCamera() const
{
	if ( camera >= 0 && camera < cameras.size() )
		return cameras.firstElement();
	else
		return 0;
}

sg::Node* CSgviewerDoc::pickNode( float x, float y, float* distance )
{
	if ( scene )
	{
		Camera* cam = cameras[camera];

		float dx = ( Math::tan( cam->horizontalFov() / 2.f ) * cam->front() * x );
		float dy = ( Math::tan( cam->verticalFov() / 2.f ) * cam->front() * y );
		Vector3 wtarget = ( cam->worldTransform().translation() + 
			cam->worldTransform().rotation().getColumn(0) * dx +
			cam->worldTransform().rotation().getColumn(1) * dy +
			cam->worldTransform().rotation().getColumn(2) * cam->front() );

		Vector3 wpos = cam->worldTransform().translation();
		Vector3 wdir = (wtarget - wpos).normalize();

		Node* node = CameraUtil::pick( scene, wpos, wdir, distance );
		if ( node )
		{
			OutputDebugString( "Picked " );
			char str[256];
			node->name().getBytes( str, sizeof(str), "ASCII-7" );
			OutputDebugString( str );
			OutputDebugString( "\n" );
		}

		return node;
	}
	return 0;
}

void CSgviewerDoc::dragNode( float dx, float dy, float dz, sg::Node* node, float dist )
{
	if ( scene )
	{
		Camera* cam = cameras[camera];

		float s = dist / cam->front();
		float w = 2.f * Math::tan( cam->horizontalFov() / 2.f ) * cam->front();
		float h = 2.f * Math::tan( cam->verticalFov() / 2.f ) * cam->front();
		float dvx = w * dx * s;
		float dvy = h * dy * s;
		float dvz = Math::sqrt(w*w+h*h) * dz * s;
		Vector3 wdelta = 
			cam->worldTransform().rotation().getColumn(0) * dvx +
			cam->worldTransform().rotation().getColumn(1) * dvy + 
			cam->worldTransform().rotation().getColumn(2) * dvz;

		Vector3 nodeDelta = node->parent()->worldTransform().inverse().rotate( wdelta );
		node->setPosition( node->transform().translation() + nodeDelta );
	}
}

void CSgviewerDoc::dragNodeRotate( float dx, float dy, sg::Node* node )
{
	if ( scene )
	{
		Camera* cam = cameras[camera];

		float xang = dy*3.14f;
		float yang = -dx*3.14f;

		Matrix3x3 xrot( Vector3(1,0,0), xang );
		Matrix3x3 yrot( Vector3(0,1,0), yang );

		Matrix3x3 rot = yrot * xrot;

		node->setRotation( node->transform().rotation() * rot );
	}
}
