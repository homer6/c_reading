#ifndef _MATH_VECTOR4_H
#define _MATH_VECTOR4_H


#include <math/VectorN.h>


namespace math
{


/**
 * 4-vector of scalar type float.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Vector4 : 
	public VectorN
{
public:
	/// Constants related to the class.
	enum Constants 
	{ 
		/// Number of components in a vector.
		SIZE = 4
	};

	/// Component [0] of the vector.
	float		x;
	
	/// Component [1] of the vector.
	float		y;
	
	/// Component [2] of the vector.
	float		z;
	
	/// Component [3] of the vector.
	float		w;

#ifdef _DEBUG
	/// Constructs undefined vector.
	Vector4()																		{set( begin(), end(), valueQuietNan() );}
#else
	Vector4()																		{}
#endif
	
	///	Constructs vector from scalar quadruple.
	Vector4( float x0, float y0, float z0, float w0 )								: x(x0), y(y0), z(z0), w(w0) {}

	/** Component-wise addition. */
	Vector4&	operator+=( const Vector4& other )									{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] += (&other.x)[i]; return *this;}
	
	/** Component-wise subtraction. */
	Vector4&	operator-=( const Vector4& other )									{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] -= (&other.x)[i]; return *this;}
	
	/** Component-wise scalar multiplication. */
	Vector4&	operator*=( float s )												{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] *= s; return *this;}
	
	/// Returns ith component of the vector.
	float&		operator[]( int elementIndex )										{/*assert(elementIndex>=0&&elementIndex<SIZE);*/ return *((&x)+elementIndex);}

	/** Returns a random-access iterator to the first component. */
	float*		begin()																{return &x;}
	
	/** Returns a random-access iterator that points just beyond the last component. */
	float*		end()																{return &x + SIZE;}

	/** Returns component-wise addition of vectors. */
	Vector4		operator+( const Vector4& other ) const								{Vector4 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] + (&other.x)[i]; return v;}
	
	/** Returns component-wise subtraction of vectors. */
	Vector4		operator-( const Vector4& other ) const								{Vector4 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] - (&other.x)[i]; return v;}
	
	/** Returns component-wise negation. */
	Vector4		operator-() const													{Vector4 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = -(&x)[i]; return v;}
	
	/** Returns vector multiplied by given scalar */
	Vector4		operator*( float s ) const											{Vector4 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] * s; return v;}
	
	/** Access to ith component of the vector. */
	const float&	operator[]( int elementIndex ) const							{/*assert(elementIndex>=0&&elementIndex<SIZE);*/ return *((&x)+elementIndex);}

	/** Component-wise equality. */
	bool		operator==( const Vector4& other ) const							{for ( int i = 0 ; i < SIZE ; ++i ) if ( (&x)[i] != (&other.x)[i] ) return false; return true;}
	
	/** Component-wise inequality. */
	bool		operator!=( const Vector4& other ) const							{for ( int i = 0 ; i < SIZE ; ++i ) if ( (&x)[i] != (&other.x)[i] ) return true; return false;}

	/** Returns dot/scalar/inner product of *this and Other */
	float		dot( const Vector4& other ) const									{float d = 0.f; for ( int i = 0 ; i < SIZE ; ++i ) d += (&x)[i] * (&other.x)[i]; return d;}

	/** Returns length of the vector. */
	float		length() const														{return valueSqrt( lengthSquared() );}
	
	/** Returns length of the vector squared. */
	float		lengthSquared() const												{return dot(*this);}
	
	/** Returns true if every component of the vector is in range [Min,Max] */
	bool		range( const float& min, const float& max ) const					{return inClosedRange(begin(),end(),min,max);}
	
	/** Returns true if every component is in finite range */
	bool		finite() const														{return inFiniteRange(begin(),end());}

	/** Returns a random-access iterator to the first component. */
	const float*	begin() const													{return &x;}
	
	/** Returns a random-access iterator that points one beyond the last component. */
	const float*	end() const														{return &x + SIZE;}

	/** Returns the vector scaled to unit length. This vector can't be 0-vector. */
	Vector4		normalize() const													{return *this * valueRSqrt( lengthSquared() );}

	/** Returns minimum of the vector elements. */
	float		minElement() const													{return MatrixMxN::minElement(begin(),end());}

	/** Returns maximum of the vector elements. */
	float		maxElement() const													{return MatrixMxN::maxElement(begin(),end());}

	/** Returns component-wise minimum. */
	Vector4		minElements( const Vector4& other ) const							{Vector4 mv; VectorN::minElements( mv.begin(), mv.end(), begin(), other.begin() ); return mv;}

	/** Returns component-wise maximum. */
	Vector4		maxElements( const Vector4& other ) const							{Vector4 mv; VectorN::maxElements( mv.begin(), mv.end(), begin(), other.begin() ); return mv;}
};


} // math


#endif // _MATH_VECTOR4_H
