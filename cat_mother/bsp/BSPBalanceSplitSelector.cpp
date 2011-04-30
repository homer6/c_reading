#include "BSPBalanceSplitSelector.h"
#include "BSPPolygon.h"
#include "BSPNode.h"
#include "ProgressIndicator.h"
#include <lang/Math.h>
#include <math/Vector4.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace lang;
using namespace math;

//-----------------------------------------------------------------------------

namespace bsp
{


BSPBalanceSplitSelector::BSPBalanceSplitSelector( int polySkip ) :
	m_polySkip( polySkip )
{
	if ( m_polySkip < 0 )
		m_polySkip = 0;
}

double BSPBalanceSplitSelector::guessWork( int polygons ) const
{
	double depthGuess = (int)( Math::log((float)polygons) / Math::log(2.f) );
	if ( depthGuess < 1 )
		depthGuess = 1;
	double workTotal = (double)(polygons/2+1) * (double)(polygons/2+1) * depthGuess / 1.5;
	return workTotal / (m_polySkip+1);
}

bool BSPBalanceSplitSelector::getSplitPlane( const util::Vector<BSPPolygon*>& polys,
	ProgressIndicator* progress, Vector4* splitPlane ) const
{
	assert( polys.size() > 0 );

	int maxPolys = polys.size();
	int maxScore = -1;
	int maxScorePoly = -1;
	double workUnit = maxPolys;

	for ( int i = 0 ; i < maxPolys ; i += m_polySkip+1 )
	{
		if ( progress )
			progress->addProgress( workUnit );

		int poly = i;
		int score = getPlaneScore( polys[poly]->plane(), polys );
		if ( score > maxScore )
		{
			maxScore = score;
			maxScorePoly = poly;
		}
	}

	assert( maxScore >= 0 );
	assert( maxScorePoly >= 0 );
	*splitPlane = polys[maxScorePoly]->plane();
	return maxScore > 0 && splitPlane->finite();
}

int BSPBalanceSplitSelector::getPlaneScore( const Vector4& plane, 
	const util::Vector<BSPPolygon*>& polys )
{
	int pos = 0;
	int neg = 0;
	int onplane = 0;

	for ( int i = 0 ; i < polys.size() ; ++i )
	{
		const BSPPolygon* poly = polys[i];
		int vpos = 0;
		int vneg = 0;
		int vonplane = 0;

		const int verts = poly->vertices();
		for ( int k = 0 ; k < verts ; ++k )
		{
			const Vector3& v = poly->getVertex(k);
			float sdist = v.x*plane.x + v.y*plane.y + v.z*plane.z + plane.w;

			if ( sdist >= BSPNode::PLANE_THICKNESS )
				++vpos;
			else if ( sdist <= -BSPNode::PLANE_THICKNESS )
				++vneg;
			else
				++vonplane;
		}

		if ( verts == vpos )
			++pos;
		else if ( verts == vneg )
			++neg;
		else if ( verts == vonplane )
			++onplane;
	}

	return Math::min(pos, neg);
}


} // bsp
