#include "MeshUtil.h"
#include <sg/Mesh.h>
#include <sg/Model.h>
#include <sg/Material.h>
#include <sg/VertexAndIndexLock.h>
#include <lang/Debug.h>
#include <util/Vector.h>
#include <math/OBBox.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace sg;
using namespace lang;
using namespace util;
using namespace math;

//-----------------------------------------------------------------------------

namespace sgu
{


void MeshUtil::splitModels( Mesh* mesh, int minPolys, float minSize )
{
	int minIndices = minPolys * 3;
	for ( int i = 0 ; i < mesh->primitives() ; ++i )
	{
		P(Model) model = dynamic_cast<Model*>( mesh->getPrimitive(i) );
		if ( model )
		{
			OBBox box = model->boundBox();
			if ( box.dimensions().x > minSize && model->indices() > minIndices )
			{
				mesh->removePrimitive( i-- );
				splitModel( model, mesh );
			}
		}
	}
}

void MeshUtil::splitModel( Model* model, Mesh* mesh )
{
	// split by main axis
	OBBox box = model->boundBox();
	Vector3 axis = box.rotation().getColumn(0);
	Vector3 point = box.translation();
	float plane[4] = { axis.x, axis.y, axis.z, -(point.dot(axis)) };
	
	// count number of polygons on each side (in/out sides)
	VertexAndIndexLock<Model> modelLock( model, Model::LOCK_READ );
	int vertsOut = 0;
	int vertsIn = 0;
	int indicesOut = 0;
	int indicesIn = 0;
	Vector<bool> vertsUsedOut( Allocator<bool>(__FILE__) );
	Vector<bool> vertsUsedIn( Allocator<bool>(__FILE__) );
	Vector<bool> faceIn( Allocator<bool>(__FILE__) );
	vertsUsedOut.setSize( model->vertices(), false );
	vertsUsedIn.setSize( model->vertices(), false );
	faceIn.setSize( model->indices()/3, false ); faceIn.clear();
	for ( int i = 0 ; i < model->indices() ; i += 3 )
	{
		// get face vertices and classify them (in/out)
		int face[3];
		model->getIndices( i, face, 3 );
		Vector3 verts[3];
		int outcode = 0;
		for ( int k = 0 ; k < 3 ; ++k )
		{
			model->getVertexPositions( face[k], verts+k, 1 );
			if ( verts[k].x*plane[0] + verts[k].y*plane[1] + verts[k].z*plane[2] + plane[3] > 0.f )
				outcode |= (1<<k);
		}

		if ( 0 == outcode ||
			(7 != outcode && 0 != (i&1)) )	// odd intersecting faces
		{
			// in / intersects odd
			faceIn.add( true );
			indicesIn += 3;
			for ( int k = 0 ; k < 3 ; ++k )
			{
				if ( !vertsUsedIn[face[k]] )
				{
					vertsUsedIn[face[k]] = true;
					++vertsIn;
				}
			}
		}
		else // if ( 7 == outcode )
		{
			// out / intersects even
			faceIn.add( false );
			indicesOut += 3;
			for ( int k = 0 ; k < 3 ; ++k )
			{
				if ( !vertsUsedOut[face[k]] )
				{
					vertsUsedOut[face[k]] = true;
					++vertsOut;
				}
			}
		}
	}
	if ( indicesIn < 3 || indicesOut < 3 )
	{
		// nothing to split
		mesh->addPrimitive( model );
		return;
	}
	Debug::println( "sgu: Splitting mesh {0} model ({1,#} polys) to two ({2,#} and {3,#} polys)", mesh->name(), model->indices()/3, indicesIn/3, indicesOut/3 );

	// build primitives
	Vector<int> vertexRemap( Allocator<int>(__FILE__) );
	int modelFaceIndex;

	// In
	P(Model) modelIn = new Model( vertsIn, indicesIn, model->vertexFormat(), model->usage() );
	VertexAndIndexLock<Model> modelInLock( modelIn, Model::LOCK_READWRITE );
	vertexRemap.clear(); vertexRemap.setSize( model->vertices(), -1 );
	int vertIn = 0;
	for ( int i = 0 ; i < model->vertices() ; ++i )
	{
		if ( vertsUsedIn[i] )
		{
			modelIn->copyVertices( vertIn, model, i, 1 );
			vertexRemap[i] = vertIn;
			++vertIn;
		}
	}
	assert( vertIn == vertsIn );
	modelFaceIndex = 0;
	int indexIn = 0;
	for ( int i = 0 ; i < model->indices() ; i += 3 )
	{
		if ( faceIn[modelFaceIndex] )
		{
			int face[3];
			model->getIndices( i, face, 3 );
			for ( int k = 0 ; k < 3 ; ++k )
				face[k] = vertexRemap[ face[k] ];
			modelIn->setIndices( indexIn, face, 3 );
			indexIn += 3;
		}
		++modelFaceIndex;
	}
	assert( indexIn == indicesIn );
	modelIn->setShader( model->shader() );
	mesh->addPrimitive( modelIn );

	// Out
	P(Model) modelOut = new Model( vertsOut, indicesOut, model->vertexFormat(), model->usage() );
	VertexAndIndexLock<Model> modelOutLock( modelOut, Model::LOCK_READWRITE );
	vertexRemap.clear(); vertexRemap.setSize( model->vertices(), -1 );
	int vertOut = 0;
	for ( int i = 0 ; i < model->vertices() ; ++i )
	{
		if ( vertsUsedOut[i] )
		{
			modelOut->copyVertices( vertOut, model, i, 1 );
			vertexRemap[i] = vertOut;
			++vertOut;
		}
	}
	assert( vertOut == vertsOut );
	modelFaceIndex = 0;
	int indexOut = 0;
	for ( int i = 0 ; i < model->indices() ; i += 3 )
	{
		if ( !faceIn[modelFaceIndex] )
		{
			int face[3];
			model->getIndices( i, face, 3 );
			for ( int k = 0 ; k < 3 ; ++k )
				face[k] = vertexRemap[ face[k] ];
			modelOut->setIndices( indexOut, face, 3 );
			indexOut += 3;
		}
		++modelFaceIndex;
	}
	assert( indexOut == indicesOut );
	modelOut->setShader( model->shader() );
	mesh->addPrimitive( modelOut );

	assert( model->indices() == modelIn->indices()+modelOut->indices() );
}

void MeshUtil::restoreBones( Node* root )
{
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* mesh = dynamic_cast<Mesh*>( node );
		if ( mesh && mesh->bones() > 0 )
			mesh->restoreBones( root );
	}
}

void MeshUtil::setRenderPass( sg::Node* root, int passSolid, int passTransparent )
{
	for ( Node* node = root ; node ; node = node->nextInHierarchy() )
	{
		Mesh* root = dynamic_cast<Mesh*>( node );
		if ( root )
		{
			for ( int i = 0 ; i < root->primitives() ; ++i )
			{
				Shader* shader = root->getPrimitive(i)->shader();
				Material* mat = dynamic_cast<Material*>( shader );
				if ( mat && !mat->stencil() )
				{
					if ( mat->destinationBlend() != Material::BLEND_ZERO )
						mat->setPass( passTransparent );
					else
						mat->setPass( passSolid );
				}
			}
		}
	}
}


} // sgu
