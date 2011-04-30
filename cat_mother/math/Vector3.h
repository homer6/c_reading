#ifndef _MATH_VECTOR3_H
#define _MATH_VECTOR3_H


#include <math/VectorN.h>


namespace math
{


/**
 * 3-vector of scalar type float.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Vector3 : 
	public VectorN
{
public:
	/// Constants related to the class.
	enum Constants 
	{ 
		/// Number of components in a vector.
		SIZE = 3
	};

	/// Component [0] of the vector.
	float		x;
	
	/// Component [1] of the vector.
	float		y;
	
	/// Component [2] of the vector.
	float		z;

#ifdef _DEBUG
	/// Constructs an undefined vector.
	Vector3()																		{set( begin(), end(), valueQuietNan() );}
#else																				
	Vector3()																		{}
#endif																				
																					
	///	Constructs vector from scalar triplet.
	Vector3( float X, float Y, float Z )											: x(X), y(Y), z(Z) {}
		
	/// Component-wise addition of vectors.
	Vector3&	operator+=( const Vector3& other )									{x+=other.x; y+=other.y; z+=other.z; return *this;}
																					
	/// Component-wise subtraction of vectors.
	Vector3&	operator-=( const Vector3& other )									{x-=other.x; y-=other.y; z-=other.z; return *this;}

	/// Component-wise scalar multiplication.
	Vector3&	operator*=( float s )												{x*=s; y*=s; z*=s; return *this;}
																					
	/// Access to ith component of the vector.										
	float&		operator[]( int elementIndex )										{/*assert(elementIndex>=0&&elementIndex<SIZE);*/ return *((&x)+elementIndex);}
																					
	/** Returns a random-access iterator to the first component. */					
	float*		begin()																{return &x;}
	
	/** Returns a random-access iterator that points one beyond the last component. */
	float*		end()																{return &x + SIZE;}

	/// Returns component-wise addition of vectors.
	Vector3		operator+( const Vector3& other ) const								{Vector3 v; v.x=x+other.x; v.y=y+other.y; v.z=z+other.z; return v;}
	
	/// Returns component-wise subtraction of vectors.
	Vector3		operator-( const Vector3& other ) const								{Vector3 v; v.x=x-other.x; v.y=y-other.y; v.z=z-other.z; return v;}
	
	/// Returns component-wise negation.
	Vector3		operator-() const													{Vector3 v; v.x=-x; v.y=-y; v.z=-z; return v;}
	
	/** Returns vector multiplied by given scalar */
	Vector3		operator*( float s ) const											{Vector3 v; v.x=x*s; v.y=y*s; v.z=z*s; return v;}
	
	/// Returns ith component of the vector.
	const float&	operator[]( int elementIndex ) const							{/*assert(elementIndex>=0&&elementIndex<SIZE);*/ return *((&x)+elementIndex);}

	/// Component-wise equality.
	bool		operator==( const Vector3& other ) const							{return x==other.x && y==other.y && z==other.z;}
	
	/// Component-wise inequality.
	bool		operator!=( const Vector3& other ) const							{return x!=other.x || y!=other.y || z!=other.z;}

	/** Returns vector/cross/outer product of *this and Other */
	Vector3		cross( const Vector3& other ) const									{return Vector3( y*other.z-z*other.y, z*other.x-x*other.z, x*other.y-y*other.x );}

	/** Returns dot/scalar/inner product of *this and Other */
	float		dot( const Vector3& other ) const									{return x*other.x + y*other.y + z*other.z;}

	//* Returns length of the vector */
	float		length() const														{return valueSqrt( x*x + y*y + z*z );}
	
	//* Returns length of the vector squared */
	float		lengthSquared() const												{return x*x + y*y + z*z;}
	
	//* Returns true if every component of the vector is in range [Min,Max] */
	bool		range( const float& min, const float& max ) const					{return inClosedRange(begin(),end(),min,max);}
	
	//* Returns true if every component is in finite range */
	bool		finite() const														{return inFiniteRange(begin(),end());}

	/** Returns a random-access iterator to the first component. */
	const float*	begin() const													{return &x;}
	
	/** Returns a random-access iterator that points one beyond the last component. */
	const float*	end() const														{return &x + SIZE;}

	/** Returns the vector scaled to unit length. This vector can't be 0-vector. */
	Vector3		normalize() const													{Vector3 nv; float invlen = valueRSqrt(x*x + y*y + z*z); nv.x=x*invlen; nv.y=y*invlen; nv.z=z*invlen; return nv;}

	/** Returns the vector rotated about specified axis by angle. */
	Vector3		rotate( const Vector3& axis, float angle ) const;

	/** Returns minimum of the vector elements. */
	float		minElement() const													{return MatrixMxN::minElement(begin(),end());}

	/** Returns maximum of the vector elements. */
	float		maxElement() const													{return MatrixMxN::maxElement(begin(),end());}

	/** Returns component-wise minimum. */
	Vector3		minElements( const Vector3& other ) const							{Vector3 mv; VectorN::minElements( mv.begin(), mv.end(), begin(), other.begin() ); return mv;}

	/** Returns component-wise maximum. */
	Vector3		maxElements( const Vector3& other ) const							{Vector3 mv; VectorN::maxElements( mv.begin(), mv.end(), begin(), other.begin() ); return mv;}

private:
	float		m_pad128;
};


} // math


#endif // _MATH_VECTOR3_H
