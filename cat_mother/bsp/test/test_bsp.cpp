#include <tester/Test.h>
#include <io/FileInputStream.h>
#include <io/FileOutputStream.h>
#include <bsp/BSPFile.h>
#include <bsp/BSPNode.h>
#include <bsp/BSPTree.h>
#include <bsp/BSPCollisionUtil.h>
#include <bsp/BSPTreeBuilder.h>
#include <bsp/BSPBoxSplitSelector.h>
#include <bsp/BSPBalanceSplitSelector.h>
#include <lang/Math.h>
#include <math/Vector3.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <config_msvc.h>
#endif // _MSC_VER

//-----------------------------------------------------------------------------

#define countof( ARRAY ) (sizeof(ARRAY)/sizeof(ARRAY[0]))

//-----------------------------------------------------------------------------

using namespace io;
using namespace bsp;
using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

static void printTree( BSPNode* node, int margin )
{
	for ( int i = 0 ; i < margin ; ++i )
		printf( " " );

	Vector3 p = Vector3(node->plane().x,node->plane().y,node->plane().z) * -node->plane().w;
	printf( "normal (%g %g), point (%g %g): ", node->plane().x, node->plane().y, p.x, p.y );

	for ( int i = 0 ; i < node->polygons() ; ++i )
		printf( "%i ", node->getPolygon(i).id() );

	printf( "\n" );

	if ( node->positive() )
		printTree( node->positive(), margin+2 );
	if ( node->negative() )
		printTree( node->negative(), margin+2 );
}

//-----------------------------------------------------------------------------

static int test()
{
	float edgePoints[] =
	{
		10, 10,  20, 20, 
		10, 25,  30, 25, 
		10, 40,  20, 30, 
		25, 30,  25, 50
	};

	// build test tree
	BSPTreeBuilder builder;
	for ( int i = 0 ; i < countof(edgePoints) ; i += 4 )
	{
		Vector3 d( 0, 0, 40 );
		Vector3 v0( edgePoints[i], edgePoints[i+1], -d.z*.5f );
		Vector3 v1( edgePoints[i+2], edgePoints[i+3], -d.z*.5f );
		Vector3 v2 = v1 + d;
		Vector3 v3 = v0 + d;
		Vector3 poly[] = {v0,v1,v2,v3};
		builder.addPolygon( poly, 4, i/4, -1 );
	}
	BSPBalanceSplitSelector splitsel(0);
	P(BSPTree) tree = builder.build( &splitsel );

	// save and load
	{
		printf( "before load/save ------------------------------------------\n" );
		printTree( tree->root(), 0 );

		P(FileOutputStream) outs = new FileOutputStream( "test.bsp" );
		BSPFile outf( tree, outs );
		outs->close();

		P(FileInputStream) ins = new FileInputStream( "test.bsp" );
		BSPFile inf( ins );
		tree = inf.tree();
		ins->close();

		printf( "after load/save ------------------------------------------\n" );
		printTree( tree->root(), 0 );
	}

	// print tree
	BSPNode* root = tree->root();

	// findLineIntersection test
	{
		Vector3 start	( 30, 10, -20 );
		Vector3 end		( 0,  40,  20 );
		Vector3 delta	= end - start;
		float t;
		BSPCollisionUtil::findLineIntersection( root, start, delta, -1, &t );
		Vector3 pos = start + delta*t;
		assert( (pos-Vector3(20,20,pos.z)).length() < 1e-3f );
	}

	// findLastLineIntersection test
	{
		Vector3 start	( 30, 10, -20 );
		Vector3 end		( 0,  40,  20 );
		Vector3 delta	= end - start;
		float t;
		BSPCollisionUtil::findLastLineIntersection( root, start, delta, -1, &t );
		Vector3 pos = start + delta*t;
		assert( (pos-Vector3(15,25,pos.z)).length() < 1e-3f );
	}

	// findMovingSphereIntersection test 1: poly plane intersection
	{
		Vector3 start	( 30, 10, -20 );
		Vector3 end		( 0,  40,  20 );
		Vector3 delta	= end - start; //delta = delta.normalize();
		float r = 5.f;

		float t;
		const BSPPolygon* cpoly = 0;
		Vector3 cnormal(0,0,0);
		Vector3 cpoint(0,0,0);
		BSPCollisionUtil::findMovingSphereIntersection( root, start, delta, r, -1, &t, &cpoly, &cnormal, &cpoint );
		assert( (cpoint-Vector3(20,20,cpoint.z)).length() < 1e-3f );
	}

	// findMovingSphereIntersection test 2: poly vertex intersection
	{
		Vector3 start	( 44, 23, -22.5f );
		Vector3 end		( 3,  23, -22.5f );
		Vector3 delta	= end - start;
		float r = 5.f;

		float t;
		const BSPPolygon* cpoly = 0;
		Vector3 cnormal(0,0,0);
		Vector3 cpoint(0,0,0);
		BSPCollisionUtil::findMovingSphereIntersection( root, start, delta, r, -1, &t, &cpoly, &cnormal, &cpoint );
		assert( (cpoint-Vector3(30,25,cpoint.z)).length() < 1e-3f );
	}

	// findMovingSphereIntersection test 2: poly edge intersection
	/*{
		Vector3 start	( 44, 23, -0.5f );
		Vector3 end		( 3,  23, 0.5f );
		Vector3 delta	= end - start;
		float r = 5.f;

		float t;
		const BSPPolygon* cpoly = 0;
		Vector3 cnormal(0,0,0);
		Vector3 cpoint(0,0,0);
		BSPCollisionUtil::findMovingSphereIntersection( root, start, delta, r, -1, &t, &cpoly, &cnormal, &cpoint );
		assert( (cpoint-Vector3(29.6f,23.7f,cpoint.z)).length() < 1.f );
		assert( (cnormal-Vector3(0.9896f, -0.1437f, 0)).length() < 1e-3f );
	}*/

	return 0;
}

//-----------------------------------------------------------------------------

static tester::Test reg( test, __FILE__ );
