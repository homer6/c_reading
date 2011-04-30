#include "TextureUtil.h"
#include <pix/Image.h>
#include <pix/Color.h>
#include <pix/Colorf.h>
#include <pix/Surface.h>
#include <pix/SurfaceFormat.h>
#include <math/Vector3.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace pix;
using namespace math;

//-----------------------------------------------------------------------------

namespace sgu
{


P(CubeTexture) TextureUtil::createNormalizerCubeTexture( int cubeSide )
{
	P(Image) img = new Image( cubeSide, cubeSide, 6, SurfaceFormat::SURFACE_A8R8G8B8 );
	float invWidthMinusOne = 1.f / (float)(img->surface(0).width()-1);
	float invHeightMinusOne = 1.f / (float)(img->surface(0).height()-1);
	{
		int face = 0;
		Surface* surf = 0;

		// x+
		surf = &img->surface(face++);
		for ( int j = 0 ; j < surf->height() ; ++j )
		{
			for ( int i = 0 ; i < surf->width() ; ++i )
			{
				float u = ((float)i * invWidthMinusOne - .5f)*2;
				float v = ((float)j * invHeightMinusOne - .5f)*2;
				Vector3 nv = Vector3( 1, -v, -u ).normalize() * 0.5f + Vector3(0.5f,0.5f,0.5f);
				Color c = Color(Colorf(nv.x,nv.y,nv.z));
				surf->setPixel( i, j, c.toInt32() );
			}
		}

		// x-
		surf = &img->surface(face++);
		for ( int j = 0 ; j < surf->height() ; ++j )
		{
			for ( int i = 0 ; i < surf->width() ; ++i )
			{
				float u = ((float)i * invWidthMinusOne - .5f)*2;
				float v = ((float)j * invHeightMinusOne - .5f)*2;
				Vector3 nv = Vector3( -1, -v, u ).normalize() * 0.5f + Vector3(0.5f,0.5f,0.5f);
				Color c = Color(Colorf(nv.x,nv.y,nv.z));
				surf->setPixel( i, j, c.toInt32() );
			}
		}

		// y+
		surf = &img->surface(face++);
		for ( int j = 0 ; j < surf->height() ; ++j )
		{
			for ( int i = 0 ; i < surf->width() ; ++i )
			{
				float u = ((float)i * invWidthMinusOne - .5f)*2;
				float v = ((float)j * invHeightMinusOne - .5f)*2;
				Vector3 nv = Vector3( u, 1, v ).normalize() * 0.5f + Vector3(0.5f,0.5f,0.5f);
				Color c = Color(Colorf(nv.x,nv.y,nv.z));
				surf->setPixel( i, j, c.toInt32() );
			}
		}

		// y-
		surf = &img->surface(face++);
		for ( int j = 0 ; j < surf->height() ; ++j )
		{
			for ( int i = 0 ; i < surf->width() ; ++i )
			{
				float u = ((float)i * invWidthMinusOne - .5f)*2;
				float v = ((float)j * invHeightMinusOne - .5f)*2;
				Vector3 nv = Vector3( u, -1, -v ).normalize() * 0.5f + Vector3(0.5f,0.5f,0.5f);
				Color c = Color(Colorf(nv.x,nv.y,nv.z));
				surf->setPixel( i, j, c.toInt32() );
			}
		}

		// z+
		surf = &img->surface(face++);
		for ( int j = 0 ; j < surf->height() ; ++j )
		{
			for ( int i = 0 ; i < surf->width() ; ++i )
			{
				float u = ((float)i * invWidthMinusOne - .5f)*2;
				float v = ((float)j * invHeightMinusOne - .5f)*2;
				Vector3 nv = Vector3( u, -v, 1 ).normalize() * 0.5f + Vector3(0.5f,0.5f,0.5f);
				Color c = Color(Colorf(nv.x,nv.y,nv.z));
				surf->setPixel( i, j, c.toInt32() );
			}
		}

		// z-
		surf = &img->surface(face++);
		for ( int j = 0 ; j < surf->height() ; ++j )
		{
			for ( int i = 0 ; i < surf->width() ; ++i )
			{
				float u = ((float)i * invWidthMinusOne - .5f)*2;
				float v = ((float)j * invHeightMinusOne - .5f)*2;
				Vector3 nv = Vector3( -u, -v, -1 ).normalize() * 0.5f + Vector3(0.5f,0.5f,0.5f);
				Color c = Color(Colorf(nv.x,nv.y,nv.z));
				surf->setPixel( i, j, c.toInt32() );
			}
		}
	}

	P(CubeTexture) normalizerCubeMap = new CubeTexture( cubeSide, SurfaceFormat::SURFACE_A8R8G8B8 );
	for ( int i = 0 ; i < 6 ; ++i )
		normalizerCubeMap->blt( &img->surface(i), i );

	/*for ( int i = 0 ; i < 6 ; ++i )
	{
		P(Image) img2 = new Image( cubeSide, cubeSide, SurfaceFormat::SURFACE_A8R8G8B8 );
		img2->surface(0).blt( &img->surface(i) );
		P(FileOutputStream) img2out = new FileOutputStream( Format("normalizerTest{0}.tga",i).format() );
		img2->save( img2out );
	}*/

	return normalizerCubeMap;
}


} // sgu
