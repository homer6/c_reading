#include <tester/Test.h>
#include <lang/String.h>
#include <mb/Polygon.h>
#include <mb/Vertex.h>
#include <mb/VertexMap.h>
#include <mb/MeshBuilder.h>
#include <mb/VertexMapFormat.h>
#include <memory.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace mb;

//-----------------------------------------------------------------------------

static int test()
{
	MeshBuilder mesh;
	Vertex* v[4];
	int i;
	for ( i = 0 ; i < 4 ; ++i )		// 0 1
		v[i] = mesh.addVertex();	// 3 2

	Polygon* a = mesh.addPolygon();
	a->addVertex( v[0] );
	a->addVertex( v[1] );
	a->addVertex( v[2] );

	Polygon* b = mesh.addPolygon();
	b->addVertex( v[0] );
	b->addVertex( v[2] );
	b->addVertex( v[3] );

	VertexMap* vmap = mesh.addVertexMap( 2, "TXUV", VertexMapFormat::VERTEXMAP_TEXCOORD );
	float uv[4][2] = { {0, 0}, {1,0}, {1,1}, {0,1} };
	vmap->addValue( v[2]->index(), uv[2], 2 );
	vmap->addValue( v[1]->index(), uv[1], 2 );
	vmap->addValue( v[3]->index(), uv[3], 2 );
	vmap->addValue( v[0]->index(), uv[0], 2 );

	v[1] = a->getVertex(1)->clone();
	a->setVertex( 1, v[1] );
	assert( mesh.vertices() == 5 );

	float uv1[4][2];
	memset( uv1, 0, sizeof(uv1) );
	for ( i = 0 ; i < 4 ; ++i )
	{
		vmap->getValue( v[i]->index(), uv1[i], 2 );
		assert( !memcmp(uv[i],uv1[i],sizeof(uv[i])) );
	}

	assert( vmap->dimensions() == 2 );
	assert( vmap->name() == "TXUV" );
	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
