#ifndef _MATH_VECTOR2_H
#define _MATH_VECTOR2_H


#include <math/VectorN.h>


namespace math
{


/** 
 * 2-vector of scalar type float. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Vector2 : 
	public VectorN
{
public:
	/// Constants related to the class.
	enum Constants 
	{ 
		/// Number of components in a vector.
		SIZE = 2 
	};

	/// Component [0] of the vector.
	float		x;
	
	/// Component [1] of the vector.
	float		y;

#ifdef _DEBUG
	/// Constructs undefined vector.
	Vector2()																		{set( begin(), end(), valueQuietNan() );}
#else
	Vector2()																		{}
#endif
	
	///	Constructs vector from scalar pair.
	Vector2( float X, float Y )														: x(X), y(Y) {}

	/// Component-wise addition of vectors.
	Vector2&	operator+=( const Vector2& other )									{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] += (&other.x)[i]; return *this;}
																					
	/// Component-wise subtraction of vectors.
	Vector2&	operator-=( const Vector2& other )									{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] -= (&other.x)[i]; return *this;}
	
	/// Component-wise scalar multiplication.
	Vector2&	operator*=( float s )												{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] *= s; return *this;}
	
	/// Access to ith component of the vector.
	float&		operator[]( int elementIndex )										{/*assert(elementIndex>=0&&elementIndex<SIZE);*/ return *((&x)+elementIndex);}

	/** Returns a random-access iterator to the first component. */
	float*		begin()																{return &x;}
	
	/** Returns a random-access iterator that points one beyond the last component. */
	float*		end()																{return &x + SIZE;}

	/** Returns component-wise addition of vectors. */
	Vector2		operator+( const Vector2& other ) const								{Vector2 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] + (&other.x)[i]; return v;}
	
	/** Returns component-wise subtraction of vectors. */
	Vector2		operator-( const Vector2& other ) const								{Vector2 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] - (&other.x)[i]; return v;}
	
	/** Returns component-wise negation. */
	Vector2		operator-() const													{Vector2 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = -(&x)[i]; return v;}
	
	/** Returns vector multiplied by given scalar */
	Vector2		operator*( float s ) const											{Vector2 v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] * s; return v;}
																					
	/// Returns ith component of the vector.											
	const float&	operator[]( int elementIndex ) const							{/*assert(elementIndex>=0&&elementIndex<SIZE);*/ return *((&x)+elementIndex);}
																					
	/** Component-wise equality. */
	bool		operator==( const Vector2& other ) const							{for ( int i = 0 ; i < SIZE ; ++i ) if ( (&x)[i] != (&other.x)[i] ) return false; return true;}
	
	/** Component-wise inequality. */
	bool		operator!=( const Vector2& other ) const							{for ( int i = 0 ; i < SIZE ; ++i ) if ( (&x)[i] != (&other.x)[i] ) return true; return false;}

	/** Returns dot/scalar/inner product of *this and Other */
	float		dot( const Vector2& other ) const									{float d = 0.f; for ( int i = 0 ; i < SIZE ; ++i ) d += (&x)[i] * (&other.x)[i]; return d;}
	
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
	Vector2		normalize() const													{return *this * valueRSqrt( lengthSquared() );}

	/** Returns minimum of the vector elements. */
	float		minElement() const													{return MatrixMxN::minElement(begin(),end());}

	/** Returns maximum of the vector elements. */
	float		maxElement() const													{return MatrixMxN::maxElement(begin(),end());}

	/** Returns component-wise minimum. */
	Vector2		minElements( const Vector2& other ) const							{Vector2 mv; VectorN::minElements( mv.begin(), mv.end(), begin(), other.begin() ); return mv;}

	/** Returns component-wise maximum. */
	Vector2		maxElements( const Vector2& other ) const							{Vector2 mv; VectorN::maxElements( mv.begin(), mv.end(), begin(), other.begin() ); return mv;}
};


} // math


#endif // _MATH_VECTOR2_H
