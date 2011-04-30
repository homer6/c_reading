#ifndef _MATH_QUATERNION_H
#define _MATH_QUATERNION_H


#include <math/Vector3.h>


namespace math
{


class Matrix3x3;


/**
 * Quaternion of scalar type float.
 * Implementation based on Dave Eberly's source code.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Quaternion : 
	public VectorN
{
public:
	/** Constants related to the class. */
	enum Constants 
	{ 
		/** Number of components in a quaternion. */
		SIZE = 4
	};

	/** X-component of the imaginary vector part of the quaternion. */
	float		x;

	/** Y-component of the imaginary vector part of the quaternion. */
	float		y;

	/** Z-component of the imaginary vector part of the quaternion. */
	float		z;
	
	/** Real part of the quaternion. */
	float		w;

#ifdef _DEBUG
	/** Constructs undefined quaternion. */
	Quaternion()																	{x=y=z=w=valueQuietNan();}
#else
	Quaternion()																	{}
#endif
	
	/** Constructs the quaternion from quadruple. */
	Quaternion( float x0, float y0, float z0, float w0 );
	
	/** Quaternion from rotation about Axis by Angle. */
	Quaternion( const Vector3& axis, float angle );
	
	/** Quaternion from rotation matrix. */
	Quaternion( const Matrix3x3& rot );

	/** Component-wise addition. */
	Quaternion&		operator+=( const Quaternion& other )							{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] += (&other.x)[i]; return *this;}
	
	/** Component-wise subtraction. */
	Quaternion&		operator-=( const Quaternion& other )							{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] -= (&other.x)[i]; return *this;}
	
	/** Component-wise scalar multiplication. */
	Quaternion&		operator*=( float s )											{for ( int i = 0 ; i < SIZE ; ++i ) (&x)[i] *= s; return *this;}
	
	/** Quaternion multiplication. */
	Quaternion&		operator*=( const Quaternion& other )							{*this = *this * other; return *this;}

	/** Returns norm of quaternion. */
	float			norm() const													{return valueSqrt( normSquared() );}
	
	/** Returns squared norm. */
	float			normSquared() const												{return dot(*this);}
	
	/** Returns true if every element is finite number. */
	bool			finite() const													{return inFiniteRange(&x,&x+SIZE);}

	/** Component-wise equality. */
	bool			operator==( const Quaternion& other ) const						{for ( int i = 0 ; i < SIZE ; ++i ) if ( (&x)[i] != (&other.x)[i] ) return false; return true;}
	
	/** Component-wise inequality. */
	bool			operator!=( const Quaternion& other ) const						{for ( int i = 0 ; i < SIZE ; ++i ) if ( (&x)[i] != (&other.x)[i] ) return true; return false;}

	/** Returns component-wise addition. */
	Quaternion		operator+( const Quaternion& other ) const						{Quaternion v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] + (&other.x)[i]; return v;}
	
	/** Returns component-wise subtraction of vectors. */
	Quaternion		operator-( const Quaternion& other ) const						{Quaternion v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] - (&other.x)[i]; return v;}
	
	/** Returns component-wise negation. */
	Quaternion		operator-() const												{Quaternion v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = -(&x)[i]; return v;}
	
	/** Returns dot product of this and other quaternion (viewed as 4-vector). */
	float			dot( const Quaternion& other ) const							{float d = 0.f; for ( int i = 0 ; i < SIZE ; ++i ) d += (&x)[i] * (&other.x)[i]; return d;}
	
	/** Returns quaternion multiplied by given scalar. */
	Quaternion		operator*( float s ) const										{Quaternion v; for ( int i = 0 ; i < SIZE ; ++i ) (&v.x)[i] = (&x)[i] * s; return v;}
	
	/** Quaternion multiplication. */
	Quaternion		operator*( const Quaternion& other ) const;

	/** Returns quaternion of unit length. */
	Quaternion 		normalize() const;

	/** Returns conjugate of quaternion. */
	Quaternion		conjugate() const;

	/** Returns inverse of quaternion. */
	Quaternion		inverse() const;

	/** Returns exponent of unit quaternion. */
	Quaternion		exp() const;

	/** Returns logarithm of unit quaternion. */
	Quaternion		log() const;

	/** Returns unit quaternion raised to power. */
	Quaternion		pow( float t ) const;

	/** 
	 * Returns spherical linear interpolation for unit quaternions. 
	 *
	 * @param t Interpolation phase [0,1]
	 * @param q Quaternion [n+1]
	 * @return Interpolated quaternion.
	 */
	Quaternion		slerp( float t, const Quaternion& q ) const;

	/** 
	 * Returns spherical cubic interpolation for unit quaternions. 
	 * This quaternion is used as element [n-1] in the interpolation.
	 *
	 * @param t Interpolation phase [0,1]
	 * @param a Quaternion [n]
	 * @param b Quaternion [n+1]
	 * @param q Quaternion [n+2]
	 * @return Interpolated quaternion.
	 */
	Quaternion		squad( float t, const Quaternion& a, const Quaternion& b, const Quaternion& q ) const;
};


} // math


#endif // _MATH_QUATERNION_H
