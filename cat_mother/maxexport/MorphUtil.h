#ifndef _MORPHUTIL_H
#define _MORPHUTIL_H


#include <lang/String.h>
#include <util/Vector.h>


class morphChannel;
class MorphR3;


/** 
 * Morph export related utilities. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MorphUtil
{
public:
	/** Returns morpher modifier if any. */
	static MorphR3*		getMorpherModifier( INode* node );

	/** 
	 * Samples morph channel weight animation frames.
	 * See sampleRate() to get number of samples per second.
	 */
	static void			getWeightAnimation( const morphChannel& chn, Interval animRange, util::Vector<float>* anim );

	/** Returns number of samples per second in raw sampled animations. */
	static int			sampleRate();
};


#endif // _MORPHUTIL_H
