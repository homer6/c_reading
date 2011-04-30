#ifndef _BSP_BSPTREE_H
#define _BSP_BSPTREE_H


#include "BSPStorage.h"
#include <lang/Object.h>


namespace bsp
{


class BSPNode;
class BSPPolygon;
class BSPStorage;


/** 
 * BSP tree container.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class BSPTree :
	public BSPStorage
{
public:
	///
	BSPTree();
	
	///
	~BSPTree();

	/** Returns root node of the tree. */
	BSPNode*			root() const	{return getNode(nodes()-1);}

private:
	BSPTree( const BSPTree& );
	BSPTree& operator=( const BSPTree& );
};


} // bsp


#endif // _BSP_BSPTREE_H
