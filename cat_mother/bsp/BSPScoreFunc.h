#ifndef _BSP_BSPSCOREFUNC_H
#define _BSP_BSPSCOREFUNC_H


namespace math {
	class Vector3;
	class Vector4;}


namespace bsp
{


class BSPPolygon;


/** 
 * Score function evaluates quality of the to-be-generated BSP 
 * if the polygons are split against specified plane. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */ 
class BSPScoreFunc
{
public:
	/** 
	 * Computes quality of the to-be-generated BSP for specified split plane.
	 * Default implementation tries to balance the tree.
	 * @return Split plane score. Higher is better.
	 */
	virtual int getPlaneScore( const math::Vector4& plane, 
		const BSPPolygon* begin, const BSPPolygon* end ) const;
};


} // bsp


#endif // _BSP_BSPSCOREFUNC_H
