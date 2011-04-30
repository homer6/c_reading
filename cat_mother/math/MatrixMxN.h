#ifndef _MATH_BASICMATRIXMXN_H
#define _MATH_BASICMATRIXMXN_H


namespace math
{


/**
 * Implementation class for float valued m*n-matrix types.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class MatrixMxN
{
public:
	static float	valueMax();
	static float	valueMin();
	static float	valueAbs( float value );
	static float	valueSqrt( float value );
	static float	valueRSqrt( float value );
	static float	valueSin( float value );
	static float	valueASin( float value );
	static float	valueCos( float value );
	static float	valueACos( float value );
	static float	valueTan( float value );
	static float	valueATan( float value );
	static bool		valueFinite( float value );
	static bool		valueIsSigned();
	static float	valueQuietNan();
	
	static void		set( float* begin, float* end, float value );
	static bool		inFiniteRange( const float* begin, const float* end );
	static bool		inClosedRange( const float* begin, const float* end, float min, float max );
	static float	minElement( const float* begin, const float* end );
	static float	maxElement( const float* begin, const float* end );
	static void		minElements( float* begin, float* end, const float* a, const float* b );
	static void		maxElements( float* begin, float* end, const float* a, const float* b );

	static float	det3( const float* m1, const float* m2, const float* m3, int i, int j, int k );
};


} // math


#endif // _MATH_BASICMATRIXMXN_H
