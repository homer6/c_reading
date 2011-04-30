#include "MainWindow.h"
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/Light.h>
#include <sg/LineList.h>
#include <sg/Texture.h>
#include <sg/VertexFormat.h>
#include <sg/Material.h>
#include <sg/Context.h>
#include <sg/Camera.h>
#include <sg/PointLight.h>
#include <sg/DirectLight.h>
#include <sg/VertexLock.h>
#include <sg/VertexAndIndexLock.h>
#include <ps/SpriteParticleSystem.h>
#include <ps/Box.h>
#include <ps/Flow.h>
#include <ps/Sphere.h>
#include <ps/Gravity.h>
#include <ps/HalfSphere.h>
#include <ps/ParticleSystemManager.h>
#include <io/IOException.h>
#include <io/CommandReader.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <io/InputStreamReader.h>
#include <io/DirectoryInputStreamArchive.h>
#include <pix/Image.h>
#include <pix/Color.h>
#include <pix/SurfaceFormat.h>
#include <win/Window.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <lang/Throwable.h>
#include <math/Noise.h>
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Matrix3x3.h>
#include <math/Matrix4x4.h>
#include <math/Quaternion.h>
#include <util/Random.h>
#include <dev/Profile.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>

//-----------------------------------------------------------------------------

#define PSTEST_VER "1.1"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace ps;
using namespace io;
using namespace pix;
using namespace win;
using namespace lang;
using namespace math;
using namespace anim;
using namespace util;

//-----------------------------------------------------------------------------

static P(Model) createCube()
{
	VertexFormat vf;
	vf.addNormal().addTextureCoordinate(2);
	P(Model) model = new Model( 4*6, 3*2*6, vf );
	VertexAndIndexLock<Model> lock( model, Model::LOCK_READWRITE );
	
	Vector3 positions[24] =
	{
		// front
		Vector3(-1,1,-1), Vector3(1,1,-1), Vector3(1,-1,-1), Vector3(-1,-1,-1),
		// back
		Vector3(1,1,1), Vector3(-1,1,1), Vector3(-1,-1,1), Vector3(1,-1,1), 
		// right
		Vector3(1,1,-1), Vector3(1,1,1), Vector3(1,-1,1), Vector3(1,-1,-1),
		// left
		Vector3(-1,1,1), Vector3(-1,1,-1), Vector3(-1,-1,-1), Vector3(-1,-1,1),
		// top
		Vector3(-1,1,1), Vector3(1,1,1), Vector3(1,1,-1), Vector3(-1,1,-1),
		// bottom
		Vector3(-1,-1,-1), Vector3(1,-1,-1), Vector3(1,-1,1), Vector3(-1,-1,1)
	};
	model->setVertexPositions( 0, positions, 24 );

	float texcoords[24*2] =
	{
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1
	};
	model->setVertexTextureCoordinates( 0, 0, 2, texcoords, 24 );

	int indices[3*2*6] =
	{
		0*4+0,0*4+1,0*4+2, 0*4+0,0*4+2,0*4+3,
		1*4+0,1*4+1,1*4+2, 1*4+0,1*4+2,1*4+3,
		2*4+0,2*4+1,2*4+2, 2*4+0,2*4+2,2*4+3,
		3*4+0,3*4+1,3*4+2, 3*4+0,3*4+2,3*4+3,
		4*4+0,4*4+1,4*4+2, 4*4+0,4*4+2,4*4+3,
		5*4+0,5*4+1,5*4+2, 5*4+0,5*4+2,5*4+3,
	};
	model->setIndices( 0, indices, 3*2*6 );

	model->computeVertexNormals();
	return model;
}

/** Draws a line to the image. Endpoint excluded. */
/*static void drawLine( Image* img, int startx, int starty, int endx, int endy, long c )
{
	int x0 = startx;
	int y0 = starty;
	int x1 = endx;
	int y1 = endy;
	
	// handle horizontal line
	if ( y0 == y1 )
	{
		if ( x0 > x1 )
		{
			// swap x
			int tmp = x0;
			x0 = x1;
			x1 = tmp;
		}

		for ( int x = x0 ; x < x1 ; ++x )
			img->setPixel( x, y0, c );
	}
	else // normal slope line
	{
		int imgw = img->width();
		int imgh = img->height();

		// reorder coordinates so that we can draw with uniform loop
		if ( y0 > y1 )
		{
			// swap x
			int tmp = y0;
			y0 = y1;
			y1 = tmp;
			// swap y
			tmp = x0;
			x0 = x1;
			x1 = tmp;
		}

		// draw slope
		int dy = y1 - y0;
		int dx16 = (int)( (x1-x0)*65536.f / (float)dy );
		int x16 = x0 << 16;
		for ( int y = y0 ; y < y1 ; ++y )
		{
			int x = x16>>16;
			x16 += dx16;
			img->setPixel( x, y, c );
			
			int xnext = x16>>16;
			if ( xnext >= imgw )
				xnext = imgw;
			for ( ++x ; x < xnext ; ++x )
				img->setPixel( x, y, c );
		}
	}
}

static Vector3 getRandomPointInSphereReject( float r )
{
	Vector3 point3(0,0,0);
	int n = 7;
	float r2 = r*r;
	float twor = r*2.f;
	float sqr;
	do
	{
		sqr = 0.f;
		for ( int i = 0 ; i < 3 ; ++i )
		{
			float v = (Math::random()-0.5f) * twor;
			sqr += v*v;
			point3[i] = v;
		}
	} while ( sqr > r2 && --n > 0 );
	if ( 0 == n )
		point3 = Vector3(0,0,0);
	return point3;
}

static Vector3 getRandomPointInSphereTrig( float r )
{
	static Random rng( time(0) );
	float x = rng.nextFloat()*2.f - 1.f;
	float d2 = 1.f - x*x;
	if ( d2 < 0.f )
		d2 = 0.f;
	float d = Math::sqrt( d2 );
	float y = (rng.nextFloat()*2.f - 1.f) * d;
	d2 = 1.f - x*x - y*y;
	if ( d2 < 0.f )
		d2 = 0.f;
	d = Math::sqrt( d2 );
	float z = (rng.nextFloat()*2.f - 1.f) * d;
	return Vector3( x*r, y*r, z*r );
}

static Vector3 getRandomPointInSphere( float r )
{
	return getRandomPointInSphereTrig( r );
}

static void randTest()
{
	P(Image) img = new Image( 256, 256, SurfaceFormat::SURFACE_R5G5B5 );

	drawLine( img, 128, 0, 128, 256, 0xFF00FF00 );
	drawLine( img, 0, 128, 256, 128, 0xFF00FF00 );

	for ( int i = 0 ; i < 100 ; ++i )
	{
		long argb = 0xFFFFFFFF;
		float r = 60.f;
		Vector3 p = getRandomPointInSphere( r );
		img->setPixel( 64+(int)p[0], 64+(int)p[1], argb );
		img->setPixel( 128+64+(int)p[2], 64+(int)p[1], argb );
		img->setPixel( 64+(int)p[0], 128+64+(int)p[2], argb );
	}

	FileOutputStream out( "/tmp/out/xyz.tga" );
	img->save( &out, out.toString() );
}

static Vector3 rv3( float r )
{
	return Vector3( (Math::random()-.5f)*2.f*r, (Math::random()-.5f)*2.f*r, (Math::random()-.5f)*2.f*r );
}

static Vector3 rv3( float r, const Vector3& d )
{
	Vector3 n = d.normalize();
	Vector3 v = rv3(r);
	v -= n * v.dot(n);
	return v;
}

static void noiseTest()
{
	P(Image) img = new Image( 256, 256, SurfaceFormat::SURFACE_R5G5B5 );

	float sx = 0.03123523f;
	float sy = 0.03123523f/1.2f;

	float maxv = -Float::MAX_VALUE;
	float minv = Float::MAX_VALUE;

	for ( int j = 0 ; j < img->height() ; ++j )
	{
		for ( int i = 0 ; i < img->width() ; ++i )
		{
			float t = Noise::noise2(i*sx,j*sy);
			if ( t < minv ) minv = t;
			if ( t > maxv ) maxv = t;
			long c = (int)(t*32.f+32.f);
			img->setPixel( j, i, c );
		}
	}

	FileOutputStream out( "/tmp/out/noise.tga" );
	img->save( &out, out.toString() );
}*/

//-----------------------------------------------------------------------------

int WINAPI WinMain( HINSTANCE inst, HINSTANCE, LPSTR cmdLine, int cmdShow )
{
	MainWindow	wnd;
	P(Context)	context		= 0;

	try
	{
		// window
		DWORD style = WS_VISIBLE;
		DWORD styleEx = 0;
		style |= WS_OVERLAPPEDWINDOW;
		wnd.create( "pstestclass", "pstest " PSTEST_VER, 640, 480, false, inst );

		// rendering context
		context = new Context( "gd_dx9" );
		context->open();

		// mesh material
		P(Material) mat = new Material;
		mat->setDiffuseColor( Colorf(1,0,0) );
		mat->setDiffuseColorSource( Material::MCS_MATERIAL );
		mat->setTextureColorCombine( 0, Material::TA_DIFFUSE, Material::TOP_SELECTARG1, Material::TA_CURRENT );

		// mesh
		P(Model) model = createCube();
		mat->setVertexFormat( model->vertexFormat() );
		model->setShader( mat );
		P(Mesh) mesh = new Mesh;
		//mesh->addPrimitive( model );
		P(Node) root = new Node;
		mesh->linkTo( root );

		// axes
		P(Mesh) axisMesh = new Mesh;
		P(LineList) axisLines = new LineList( 3, LineList::LINES_3D );
		{VertexLock<LineList> axisLinesLock( axisLines, LineList::LOCK_WRITE );
		axisLines->addLine( Vector3(0,0,0), Vector3(1,0,0), Color(255,0,0) );
		axisLines->addLine( Vector3(0,0,0), Vector3(0,1,0), Color(0,255,0) );
		axisLines->addLine( Vector3(0,0,0), Vector3(0,0,1), Color(0,0,255) );}
		axisMesh->addPrimitive( axisLines );
		axisMesh->linkTo( root );

		// camera
		P(Camera) cam = new Camera;
		cam->setPosition( Vector3(1,3.5f,-6) );
		cam->setHorizontalFov( Math::toRadians(90.f) );
		cam->setFront( 0.2f );
		cam->setBack( 100.f );
		cam->linkTo( root );
		cam->lookAt( mesh );

		// light
		P(DirectLight) lt = new DirectLight;
		lt->setAmbientColor( Colorf(0.1f, 0.1f, 0.1f) );
		lt->setPosition( Vector3(-2,2,-3) );
		lt->linkTo( cam );

		// test SpriteParticleSystem
		/*P(Texture) tex = new Texture( "smallflare.jpg" );
		P(SpriteParticleSystem) ps = new SpriteParticleSystem;
		ps->linkTo( mesh );
		ps->setPositionShape( new HalfSphere(Vector3(0,0,0),1.f) );
		ps->setVelocityShape( new HalfSphere(Vector3(0,0,0),10.f) );
		ps->setEmissionRate( 100 );
		ps->setParticleLifeTime( 3.f );
		ps->setSystemLifeTime( 120.f );
		ps->setMaxParticles( 500 );
		ps->setKillType( ParticleSystem::KILL_RANDOM );
		ps->addForce( new Gravity(9.8f) );
		ps->setImage( tex );*/

		// test particle system
		P(DirectoryInputStreamArchive) arch = new DirectoryInputStreamArchive;
		arch->addPath( "./" );
		arch->addPath( "particles" );
		P(ParticleSystemManager) mgr = new ParticleSystemManager( arch );
		mgr->setScene( root);

		// mainloop
		long prevTime = System::currentTimeMillis();
		bool canStartNew = true;
		float time = 0.f;
		while ( Window::flushWindowMessages() )
		{
			long curTime = System::currentTimeMillis();
			float dt = 1e-3f * (float)(curTime - prevTime);
			prevTime = curTime;

			if ( wnd.active() )
			{
				if ( GetKeyState('P') < 0 )
					dt = 0.f;
				if ( GetKeyState('S') < 0 )
					dt *= 0.25f;
				if ( GetKeyState('F') < 0 )
					dt *= 3.f;
				if ( GetKeyState('B') < 0 )
					dt *= -3.f;
				if ( GetKeyState('R') < 0 )
					time = 0.f, dt = 0.f;
				if ( GetKeyState('Z') < 0 )
					cam->setPosition( cam->transform().translation() + cam->transform().rotation().getColumn(2) * dt * 4.f );
				if ( GetKeyState('X') < 0 )
					cam->setPosition( cam->transform().translation() + cam->transform().rotation().getColumn(2) * dt * -4.f );
				if ( GetKeyState(VK_F11) < 0 )
					mgr->clear();
				if ( GetKeyState(VK_SPACE) < 0 && canStartNew )
				{
					for ( int i = 1 ; i < __argc ; ++i )
						mgr->play( String(__argv[i]), 0 );
					canStartNew = false;
				}
				else if ( GetKeyState(VK_SPACE) >= 0 )
				{
					canStartNew = true;
				}
				time += dt;

				mgr->update( dt );

				//for ( Node* node = root ; node ; node = node->nextInHierarchy() )
				//	node->setState( time );

				context->beginScene();
				cam->render();
				context->endScene();
				context->present();
				context->clear();
			}
		}

		context->destroy();
	}
	catch ( Throwable& e )
	{
		if ( context )
			context->destroy();
		char msg[1024] = "";
		e.getMessage().format().getBytes( msg, sizeof(msg), "ASCII-7" );
		MessageBox( 0, msg, "Error", MB_OK );
		wnd.destroy();
	}
	return 0;
}
