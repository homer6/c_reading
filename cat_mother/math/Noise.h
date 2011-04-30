#ifndef _MATH_NOISE_H
#define _MATH_NOISE_H


namespace math
{



/** 
 * Perlin noise functions. 
 * Implementation based on Ken Perlin's source code.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Noise
{
public:
	/** Coherent noise function over 1 dimension. */
	static float	noise1( float x );

	/** Coherent noise function over 2 dimensions. */
	static float	noise2( float x, float y );

	/** Coherent noise function over 3 dimensions. */
	static float	noise3( float x, float y, float z );
};


} // math


#endif // _MATH_NOISE_H
