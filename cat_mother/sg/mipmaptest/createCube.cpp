#include <sg/Model.h>
#include <sg/VertexAndIndexLock.h>
#include <sg/VertexFormat.h>
#include <pix/Color.h>
#include <math/Vector3.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace pix;
using namespace math;

//-----------------------------------------------------------------------------

P(Model) createCube( float w, float h, float d, VertexFormat vf )
{
	P(Model) model = new Model( 4*6, 3*2*6, vf );
	VertexAndIndexLock<Model> lock( model, Model::LOCK_READWRITE );
	int i;
	
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
	for ( i = 0 ; i < 24 ; ++i )
	{
		positions[i].x *= w/2.f;
		positions[i].y *= h/2.f;
		positions[i].z *= d/2.f;
	}
	model->setVertexPositions( 0, positions, 24 );

	Color vertcolor[24];
	for ( i = 0 ; i < 24 ; ++i )
	{
		Color c;
		if ( positions[i].x < 0.f )
			c.setRed( 0 );
		if ( positions[i].y < 0.f )
			c.setGreen( 0 );
		if ( positions[i].z < 0.f )
			c.setBlue( 0 );
		vertcolor[i] = c;
	}
	if ( vf.hasDiffuse() )
		model->setVertexDiffuseColors( 0, vertcolor, 24 );

	float texcoords[24*2] =
	{
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1,
		0,0, 1,0, 1,1, 0,1
	};
	for ( int i = 0 ; i < vf.textureCoordinates() ; ++i )
	{
		for ( int k = 0 ; k < model->vertices() ; ++k )
		{
			float uv[4] = { texcoords[k*2], texcoords[k*2+1], 0, 0 };
			int uvsize = vf.getTextureCoordinateSize(i);
			model->setVertexTextureCoordinates( k, i, uvsize, uv, 1 );
		}
	}

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
