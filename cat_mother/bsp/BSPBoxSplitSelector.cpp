#include "BSPBoxSplitSelector.h"
#include "BSPPolygon.h"
#include "ProgressIndicator.h"
#include <lang/Math.h>
#include <math/Vector4.h>
#include <math/OBBoxBuilder.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


double BSPBoxSplitSelector::guessWork( int polygons ) const
{
	double depthGuess = (int)( Math::log((float)polygons) / Math::log(2.f) * 1.5f );
	if ( depthGuess < 1 )
		depthGuess = 1;
	double workTotal = (double)(polygons+1) * depthGuess;
	return workTotal;
}

bool BSPBoxSplitSelector::getSplitPlane( const util::Vector<BSPPolygon*>& polys,
	ProgressIndicator* progress, Vector4* splitPlane ) const
{
	assert( polys.size() > 0 );

	double workUnit = polys.size();

	if ( progress )
		progress->addProgress( workUnit );

	OBBoxBuilder builder;
	while ( builder.nextPass() )
	{
		for ( int i = 0 ; i < polys.size() ; ++i )
		{
			const BSPPolygon& poly = *polys[i];
			for ( int k = 0 ; k < poly.vertices() ; ++k )
				builder.addPoints( &poly.getVertex(k), 1 );
		}
	}

	OBBox box = builder.box();
	Vector3 center = box.translation();
	Vector3 normal = box.rotation().getColumn(0);
	*splitPlane = Vector4( normal.x, normal.y, normal.z, -center.dot(normal) );
	return Math::abs(normal.length()-1.f) < 1e-3f && splitPlane->finite();
}


} // bsp
