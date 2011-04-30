#include "createCube.h"
#include <io/File.h>
#include <io/IOException.h>
#include <io/FileInputStream.h>
#include <sg/Font.h>
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/Effect.h>
#include <sg/PolygonAdjacency.h>
#include <sg/VertexAndIndexLock.h>
#include <sg/VertexFormat.h>
#include <sg/Light.h>
#include <sg/Scene.h>
#include <sg/Texture.h>
#include <sg/CubeTexture.h>
#include <sg/Material.h>
#include <sg/Context.h>
#include <sg/Camera.h>
#include <sg/PointLight.h>
#include <sg/DirectLight.h>
#include <sg/ShadowShader.h>
#include <sg/ShadowVolume.h>
#include <sgu/LookAtControl.h>
#include <pix/Image.h>
#include <pix/Color.h>
#include <pix/SurfaceFormat.h>
#include <win/Window.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Array.h>
#include <lang/System.h>
#include <lang/Throwable.h>
#include <util/Vector.h>
#include <util/Hashtable.h>
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Matrix3x3.h>
#include <math/Matrix4x4.h>
#include <math/Quaternion.h>
#include <anim/VectorInterpolator.h>
#include <anim/QuaternionInterpolator.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace sg;
using namespace pix;
using namespace win;
using namespace lang;
using namespace util;
using namespace math;
using namespace anim;

//-----------------------------------------------------------------------------

P(Texture) loadTex( String name )
{
	FileInputStream texin( name );
	P(Texture) tex = new Texture( &texin );
	texin.close();
	return tex;
}

P(CubeTexture) loadCubeTex( String name )
{
	FileInputStream texin( name );
	P(CubeTexture) tex = new CubeTexture( &texin );
	texin.close();
	return tex;
}

int WINAPI WinMain( HINSTANCE inst, HINSTANCE, LPSTR cmdLine, int cmdShow )
{
	Window wnd;

	P(Context)				context		= 0;
	int						sw			= 640;
	int						sh			= 480;
	int						sbpp		= 0;
	bool					fullscreen	= (0 != sbpp);
	P(Scene)				root		= 0;

	try
	{
		// args
		String fxname = cmdLine;
		if ( fxname == "" )
			fxname = "bump.fx";

		// init window
		DWORD style = WS_VISIBLE;
		DWORD styleEx = 0;
		if ( fullscreen )
		{
			style |= WS_POPUP;
			styleEx |= WS_EX_TOPMOST;
		}
		else
		{
			style |= WS_OVERLAPPEDWINDOW;
		}
		wnd.create( "sgtestclass", "sg test", style, styleEx, 0, 0, sw, sh, inst, 0 );

		// init context
		context = new Context( "gd_dx8" );
		context->open( sw, sh, sbpp, 75,
			Context::SURFACE_TARGET + 
			Context::SURFACE_DEPTH + 
			Context::SURFACE_STENCIL,
			Context::RASTERIZER_HW, 
			Context::VERTEXP_HW,
			Context::TC_DISABLED );

		// scene
		root = new Scene;
		root->setName( "root" );

		// camera
		P(Camera) cam = new Camera;
		cam->setName( "cam" );
		cam->setPosition( Vector3(0,1.5f,-3) );
		Matrix3x3 rot = Matrix3x3( Vector3(1,0,0), Math::toRadians(30.f) );
		cam->setRotation( rot );
		cam->setHorizontalFov( Math::toRadians(100.f) );
		cam->setFront( 0.2f );
		cam->setBack( 100.f );
		cam->linkTo( root );

		// light
		P(DirectLight) lt = new DirectLight;
		lt->setName( "keylight" );
		lt->setIntensity( 0.5f );
		lt->linkTo( root );
		lt->lookAt( Vector3(-0.5f,-1,0) );

		// resources
		P(Texture) difmap = loadTex( "diffuse_map.tga" );
		P(Texture) normmap = loadTex( "normal_map.tga" );
		P(CubeTexture) envmap = loadCubeTex( "TestCubeTexture1.dds" );

		// read vb and ib data
		FileInputStream vbin( "vb.dat" );
		Array<uint8_t> vbbytes( vbin.available() );
		vbin.read( vbbytes.begin(), vbbytes.size() );
		int vertexSize = 4*3*4;
		int vertices = vbbytes.size() / vertexSize;
		FileInputStream ibin( "ib.dat" );
		Array<uint8_t> ibbytes( ibin.available() );
		ibin.read( ibbytes.begin(), ibbytes.size() );
		int indexSize = 2;
		int indices = ibbytes.size() / indexSize;

		// geometry (indirectly from vb and ib dat)
		VertexFormat vf; 
		vf.addNormal().addTextureCoordinate(3);
		P(Model) model = new Model( vertices, indices, vf );
		{
			VertexAndIndexLock<Model> lk( model, Model::LOCK_READWRITE );

			for ( int i = 0 ; i < vertices ; ++i )
			{
				model->setVertexPositions( i, (Vector3*)(vbbytes.begin()+i*vertexSize) );
				model->setVertexNormals( i, (Vector3*)(vbbytes.begin()+12+i*vertexSize) );
				model->setVertexTextureCoordinates( i, 0, 3, (float*)(vbbytes.begin()+24+i*vertexSize) );
			}

			void* idata;
			int istride;
			model->getIndexData( &idata, &istride );
			memcpy( idata, ibbytes.begin(), ibbytes.size() );
		}

		// env transform
		Matrix4x4 textm(0);
		textm(0,0) = 1.f;
		textm(1,1) = 1.f;
		textm(2,2) = 1.f;
		textm(0,3) = 0.f;
		textm(1,3) = 0.f;		
		textm(2,3) = 0.f;

		// material
		P(Material) mat = new Material;

		mat->setTexture( 0, difmap );
		
		mat->setTexture( 1, envmap );
		mat->setTextureColorCombine( 1, Material::TA_TEXTURE, Material::TOP_ADD, Material::TA_CURRENT );		
		mat->setTextureCoordinateTransform( 1, Material::TTFF_COUNT3, textm );
		mat->setTextureCoordinateSource( 1, Material::TCS_CAMERASPACENORMAL );

		mat->setVertexFormat( model->vertexFormat() );
		model->setShader( mat );

		// mesh
		P(Mesh) mesh = new Mesh;
		mesh->setName( "mesh" );
		mesh->linkTo( root );
		mesh->addPrimitive( model );
		cam->setRotationController( new sgu::LookAtControl(cam,mesh) );

		// floor
		vf = VertexFormat();
		vf.addNormal().addTextureCoordinate(2).addTextureCoordinate(2);
		P(Model) floormodel = createCube( 10, 0.01f, 10, vf );
		mat = new Material;
		mat->setDiffuseColor( Colorf(0,0,0) );
		mat->setEmissiveColor( Colorf(0.2f,0.2f,0.2f) );
		mat->setVertexFormat( floormodel->vertexFormat() );
		floormodel->setShader( mat );
		P(Mesh) floor = new Mesh;
		floor->setName( "floor" );
		floor->linkTo( root );
		floor->setPosition( Vector3(0,-2,0) );
		floor->addPrimitive( floormodel );

		// mesh position animation
		int keys = 8;
		{P(VectorInterpolator) anim = new VectorInterpolator(3);
		anim->setKeys( keys );
		for ( int i = 0 ; i < keys ; ++i )
		{
			float f = (float)i / (keys-1);
			float v[3] = { Math::sin(f*Math::PI*2.f)*2.f, 0, 0 };
			anim->setKeyValue( i, v, 3 );
			anim->setKeyTime( i, f * 4.f );
		}
		anim->sortKeys();
		mesh->setPositionController( anim );}

		// mesh rotation animation
		{P(QuaternionInterpolator) anim = new QuaternionInterpolator;
		anim->setKeys( keys );
		for ( int i = 0 ; i < keys ; ++i )
		{
			float f = (float)i / (keys-1);
			float ang = f * Math::PI*2.f;
			Quaternion quat( Vector3(0,1,0), ang );
			float v[] = {quat.x, quat.y, quat.z, quat.w};
			anim->setKeyValue( i, v, 4 );
			anim->setKeyTime( i, f * 3.f );
		}
		anim->sortKeys();
		mesh->setRotationController( anim );}

		// mesh scale animation
		/*{P(VectorInterpolator) anim = new VectorInterpolator(3);
		anim->setKeys( keys );
		for ( int i = 0 ; i < keys ; ++i )
		{
			float f = (float)i / (keys-1);
			float v[3] = { Math::abs(Math::sin(f*Math::PI*2.f))+.5f, 1.f, 1.f };
			anim->setKeyValue( i, v, 3 );
			anim->setKeyTime( i, f * 5.f );
		}
		anim->sortKeys();
		mesh->setScaleController( anim );}*/

		// shadow volume
		P(ShadowVolume) shadow = new ShadowVolume( model, lt->worldDirection(), 20.f );
		shadow->setShader( new ShadowShader );
		mesh->addPrimitive( shadow );

		// shadow filler
		VertexFormat fillvf;
		fillvf.addRHW().addDiffuse();
		P(Material) fillmat = new Material;
		fillmat->setDepthEnabled( false );
		fillmat->setDepthWrite( false );
		fillmat->setBlend( Material::BLEND_SRCALPHA, Material::BLEND_INVSRCALPHA );
		fillmat->setTextureColorCombine( 0, Material::TA_DIFFUSE, Material::TOP_SELECTARG1, Material::TextureArgument() );
		fillmat->setTextureAlphaCombine( 0, Material::TA_DIFFUSE, Material::TOP_SELECTARG1, Material::TextureArgument() );
		fillmat->setLighting( false );
		fillmat->setVertexColor( true );
		fillmat->setStencil( true );
		fillmat->setStencilRef( 1 );
		fillmat->setStencilFunc( Material::CMP_LESSEQUAL );
		fillmat->setStencilPass( Material::STENCILOP_KEEP );
		fillmat->setPass( ShadowShader::DEFAULT_SHADOW_FILL_PASS );
		fillmat->setVertexFormat( fillvf );

		// shadow filler geometry
		P(Model) fillgeom = new Model( 4, 6, fillvf );
		{VertexAndIndexLock<Model> lock( fillgeom, Model::LOCK_WRITE );
		Vector4 fillpos[4] = { Vector4(0,0,0,0), Vector4(sw,0,0,0), Vector4(sw,sh,0,0), Vector4(0,sh,0,0) };
		Color fillcolr[4] = { Color(0,0,0,80), Color(0,0,0,80), Color(0,0,0,80), Color(0,0,0,80) };
		int fillind[6] = { 0,1,2, 0,2,3 };
		fillgeom->setVertexPositionsRHW( 0, fillpos, 4 );
		fillgeom->setVertexDiffuseColors( 0, fillcolr, 4 );
		fillgeom->setIndices( 0, fillind, 6 );}

		fillgeom->setShader( fillmat );
		mesh->addPrimitive( fillgeom );

		// mainloop
		long prevTime = System::currentTimeMillis();
		float time = 0.f;
		while ( Window::flushWindowMessages() )
		{
			long curTime = System::currentTimeMillis();
			float dt = 1e-3f * (float)(curTime - prevTime);
			prevTime = curTime;

			if ( wnd.active() )
			{
				if ( GetKeyState('S') < 0 )
					dt *= 0.1f;
				if ( GetKeyState('R') < 0 )
					time = 0.f;
				if ( GetKeyState(VK_ESCAPE) < 0 )
					break;
				time += dt;

				for ( Node* node = root ; node ; node = node->nextInHierarchy() )
					node->setState( time );

				context->beginScene();
				cam->render();
				context->endScene();
				context->present();
				context->clear();
			}
		}

		context->destroy();
		wnd.destroy();
	}
	catch ( Throwable& e )
	{
		if ( context )
			context->destroy();

		MoveWindow( wnd.handle(), 0, 0, 0, 0, TRUE );
		char msg[1024] = "";
		e.getMessage().format().getBytes( msg, sizeof(msg), "ASCII-7" );
 		MessageBox( 0, msg, "Error", MB_OK );

		wnd.destroy();
	}
	return 0;
}
