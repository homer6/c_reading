#ifndef _ANIM_INTERPOLATION_H
#define _ANIM_INTERPOLATION_H


class KeyFrame;


/** 
 * Interface to interpolation objects. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Interpolation
{
public:
	/** 
	 * Interpolates key frames. 
	 * @param key0prev The key before the active segment if any.
	 * @param key0 The first key of the active segment.
	 * @param key1 The second key of the active segment.
	 * @param key1next The key after the active segment if any.
	 * @param t Fraction [0,1) of the active segment to interpolate.
	 * @param tlength Length of the active segment.
	 * @param channels Number of channels in the data.
	 * @param value [out] Receives interpolated values.
	 */
	virtual void interpolate( 
		const KeyFrame* key0prev, const KeyFrame* key0,
		const KeyFrame* key1, const KeyFrame* key1next,
		float t, float tlength, int channels, float* values ) const = 0;
};


#endif // _ANIM_INTERPOLATION_H
