#include "StdAfx.h"
#include "BSPUtil.h"
#include <bsp/BSPNode.h>
#include <math/OBBoxBuilder.h>

//-----------------------------------------------------------------------------

using namespace math;

//-----------------------------------------------------------------------------

/** Adds BSP polygons to the OBB builder recursively. */
static void addPoints( OBBoxBuilder& builder, bsp::BSPNode* node )
{
	if ( !node )
		return;

	for ( int i = 0 ; i < node->polygons() ; ++i )
	{
		const bsp::BSPPolygon& poly = node->getPolygon( i );
		for ( int k = 0 ; k < poly.vertices() ; ++k )
			builder.addPoints( &poly.getVertex(k), 1 );
	}

	addPoints( builder, node->positive() );
	addPoints( builder, node->negative() );
}

//-----------------------------------------------------------------------------

OBBox BSPUtil::getOBBox( bsp::BSPNode* root )
{
	OBBox obb;
	OBBoxBuilder builder;
	
	while ( builder.nextPass() )
		addPoints( builder, root );

	return builder.box();
}
