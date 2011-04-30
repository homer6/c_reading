#ifndef _ANIM_INTERPOLATIONSTEPPED_H
#define _ANIM_INTERPOLATIONSTEPPED_H


#include "Interpolation.h"


/** Stepped interpolation. */
class InterpolationStepped : 
	public Interpolation
{
public:
	/** 
	 * Interpolates key frames using stepped 'interpolation'. 
	 * @param key0prev The key before the active segment.
	 * @param key0 The first key of the active segment.
	 * @param key1 The second key of the active segment.
	 * @param key1next The key after the active segment.
	 * @param t Fraction [0,1) of the active segment to interpolate.
	 * @param tlength Length of the active segment.
	 * @param channels Number of channels in the data.
	 * @param value [out] Receives interpolated values.
	 */
	void interpolate( 
		const KeyFrame* key0prev, const KeyFrame* key0,
		const KeyFrame* key1, const KeyFrame* key1next,
		float t, float tlength, int channels, float* values ) const;
};


#endif // _ANIM_INTERPOLATIONSTEPPED_H
