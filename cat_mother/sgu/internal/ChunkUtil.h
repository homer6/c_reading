#ifndef _SGU_CHUNKUTIL_H
#define _SGU_CHUNKUTIL_H


namespace io {
	class ChunkInputStream;
	class ChunkOutputStream;}

namespace anim {
	class Interpolator;}

namespace lang {
	class String;}


namespace sgu
{


/** 
 * Utilities for ChunkInputStream and ChunkOutputStream. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class ChunkUtil
{
public:
	/** Loading flags. */
	enum LoadFlags
	{
		/** Load single key only. */
		NO_ANIMATIONS	= 1,
	};

	/** Reads animation (chunk) from the stream. */
	static void		readAnim( io::ChunkInputStream* in, anim::Interpolator* anim, const lang::String& name, int loadFlags=0 );
};


} // sgu


#endif // _SGU_CHUNKUTIL_H
