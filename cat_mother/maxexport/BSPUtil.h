#ifndef _BSPUTIL_H
#define _BSPUTIL_H


#include <math/OBBox.h>


namespace bsp {
	class BSPNode;}


/** 
 * BSP tree utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPUtil
{
public:
	/** Computes BSP tree bounding box. */
	static math::OBBox		getOBBox( bsp::BSPNode* root );
};


#endif // _BSPUTIL_H
