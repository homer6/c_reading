#ifndef _SGLOD_H
#define _SGLOD_H


#include "SgNode.h"


/**
 * LOD to be exported.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgLOD :
	public SgNode
{
public:
	/** Level of detail ID of the children. */
	lang::String	lodID;

	SgLOD();

	void			write( io::ChunkOutputStream* out ) const;
};


#endif // _SGLOD_H
