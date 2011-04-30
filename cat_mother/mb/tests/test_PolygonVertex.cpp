#include <tester/Test.h>
#include <mb/Polygon.h>
#include <mb/Vertex.h>
#include <mb/MeshBuilder.h>
#include <assert.h>

//-----------------------------------------------------------------------------

using namespace mb;

//-----------------------------------------------------------------------------

static int test()
{
	MeshBuilder mesh;
	int i;
	Vertex* v[4];
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

	assert( v[0]->polygons() == 2 );
	assert( v[1]->polygons() == 1 );
	assert( v[2]->polygons() == 2 );
	assert( v[3]->polygons() == 1 );

	assert( a->getVertex(2)->index() == 2 );		// vertex index in mesh
	assert( b->getVertex(1)->index() == 2 );		
	assert( b->getVertex(2)->index() == 3 );		

	a->removeVertex( a->getVertex(2) );
	assert( v[2]->polygons() == 1 );
	assert( b->getVertex(1)->index() == 2 );

	Polygon* c = mesh.addPolygon();
	c->addVertex( v[0] );
	c->addVertex( v[1] );
	c->addVertex( v[2] );
	c->addVertex( v[3] );
	c->split( 3, 1 );
	Polygon* c2 = mesh.getPolygon( mesh.polygons()-1 );
	assert( c->getVertex(0) == v[1] );
	assert( c->getVertex(1) == v[2] );
	assert( c->getVertex(2) == v[3] );
	assert( c->vertices() == 3 );
	assert( c2->getVertex(0) == v[0] );
	assert( c2->getVertex(1) == v[1] );
	assert( c2->getVertex(2) == v[3] );
	assert( c2->vertices() == 3 );

	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
