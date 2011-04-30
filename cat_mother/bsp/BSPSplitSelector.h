#ifndef _BSP_BSPSPLITSELECTOR_H
#define _BSP_BSPSPLITSELECTOR_H


#include <util/Vector.h>


namespace math {
	class Vector4;}


namespace bsp
{


class BSPPolygon;
class ProgressIndicator;


/** 
 * Functor for selecting polygon set split plane.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */ 
class BSPSplitSelector
{
public:
	///
	virtual ~BSPSplitSelector();

	/**
	 * Guess amount of work to be done from total number of polygons.
	 */
	virtual double	guessWork( int polygons ) const = 0;

	/** 
	 * Selects polygon set split plane
	 * @param begin Pointer to the first polygon.
	 * @param end Pointer to one beyond the last polygon.
	 * @param progress Interface to progress indicator.
	 * @param splitPlane [out] Receives selected split plane.
	 * @return false If no more splitting is required.
	 */
	virtual bool	getSplitPlane( const util::Vector<BSPPolygon*>& polys,
						ProgressIndicator* progress,
						math::Vector4* splitPlane ) const = 0;
};


} // bsp


#endif // _BSP_BSPSPLITSELECTOR_H
