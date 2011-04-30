#ifndef _SGDUMMY_H
#define _SGDUMMY_H


#include "SgNode.h"
#include <math/Vector3.h>


/** 
 * Dummy object to be exported.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgDummy :
	public SgNode
{
public:
	math::Vector3	boxMin;
	math::Vector3	boxMax;

	SgDummy();

	void	write( io::ChunkOutputStream* out ) const;
};


#endif // _SGDUMMY_H
