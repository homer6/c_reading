#ifndef _BSP_BALANCEBSPSPLITSELECTOR_H
#define _BSP_BALANCEBSPSPLITSELECTOR_H


#include <bsp/BSPSplitSelector.h>
#include <util/Vector.h>


namespace math {
	class Vector4;}


namespace bsp
{


class BSPPolygon;


/** 
 * Functor for selecting polygon set split plane so
 * that the BSP tree will be balanced.
 * O(n^2) plane selection algorithm.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */ 
class BSPBalanceSplitSelector :
	public BSPSplitSelector
{
public:
	/**
	 * @param polySkip Number of polygons to skip when evaluating split plane. For best quality use 0, for faster build use larger value.
	 */
	explicit BSPBalanceSplitSelector( int polySkip );

	/**
	 * Guess amount of work to be done from total number of polygons.
	 */
	double	guessWork( int polygons ) const;

	/** 
	 * Selects polygon set split plane
	 * @param begin Pointer to the first polygon.
	 * @param end Pointer to one beyond the last polygon.
	 * @param splitPlane [out] Receives selected split plane.
	 * @return false If no more splitting is required.
	 */
	bool	getSplitPlane( const util::Vector<BSPPolygon*>& polys,
				ProgressIndicator* progress, math::Vector4* splitPlane ) const;

private:
	int		m_polySkip;

	static int	getPlaneScore( const math::Vector4& plane, 
					const util::Vector<BSPPolygon*>& polys );
};


} // bsp


#endif // _BSP_BALANCEBSPSPLITSELECTOR_H
