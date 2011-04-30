#ifndef _BSP_BOXSPLITSELECTOR_H
#define _BSP_BOXSPLITSELECTOR_H


#include <bsp/BSPSplitSelector.h>


namespace math {
	class Vector4;}


namespace bsp
{


class BSPPolygon;


/** 
 * Functor for selecting polygon set split plane so
 * that the split plane halves oriented bounding box.
 * O(n) plane selection algorithm.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */ 
class BSPBoxSplitSelector :
	public BSPSplitSelector
{
public:
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
};


} // bsp


#endif // _BSP_BOXSPLITSELECTOR_H
