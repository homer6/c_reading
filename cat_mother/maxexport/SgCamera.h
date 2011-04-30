#ifndef _SGCAMERA_H
#define _SGCAMERA_H


#include "SgNode.h"


/** 
 * Camera node. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgCamera :
	public SgNode
{
public:
	/** Field of view in radians. */
	float	fov;

	///
	SgCamera();

	///
	void	write( io::ChunkOutputStream* out ) const;
};


#endif // _SGCAMERA_H
