#ifndef _SGNODE_H
#define _SGNODE_H


#include <lang/Object.h>
#include <lang/String.h>
#include <util/Vector.h>
#include <math/Matrix4x4.h>
#include "KeyFrameContainer.h"


namespace io {
	class ChunkOutputStream;}


/** 
 * Node to be exported. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SgNode :
	public lang::Object
{
public:
	lang::String					name;
	KeyFrameContainer				pos;
	KeyFrameContainer				rot;
	KeyFrameContainer				scl;
	int								parent;			// -1 none
	int								target;			// -1 none
	SgNode*							parentNode;
	SgNode*							targetNode;
	bool							castShadows;
	bool							recvShadows;
	bool							renderable;
	bool							resampleAnimations;
	util::Vector<math::Matrix4x4>	tmAnim;

	SgNode();
	virtual ~SgNode();

	virtual void			write( io::ChunkOutputStream* out ) const;
	virtual bool			isAnimated() const;
	virtual math::Matrix4x4	getTransform( float time ) const;
	virtual math::Matrix4x4	getWorldTransform( float time ) const;

	/** Writes all node related chunks as subchunks. */
	void	writeNodeChunks( io::ChunkOutputStream* out ) const;

	/** 
	 * Resamples pos, rot and scl animations from tmAnim. 
	 * @param animRange Animation range to export.
	 * @param node3ds 3DS Max frame of reference.
	 */
	void	resampleTransformAnimation( Interval animRange, INode* node3ds );
};


#endif // _SGNODE_H
