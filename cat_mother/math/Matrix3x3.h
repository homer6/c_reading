#ifndef _MATH_MATRIX3X3_H
#define _MATH_MATRIX3X3_H


#include <math/Vector3.h>


namespace math
{


class Quaternion;


/**
 * 3x3 row-major matrix of scalar type float.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Matrix3x3 : 
	public MatrixMxN
{
public:
	/** Constants related to the class.*/
	enum Constants 
	{ 
		/** Number of rows in a matrix.*/
		ROWS	= 3, 
		/** Number of columns in a matrix.*/
		COLUMNS	= 3, 
		/** Number of components in a matrix.*/
		SIZE	= 9,
	};

#ifdef _DEBUG
	/** Constructs undefined matrix.*/
	Matrix3x3()																		{float v = valueQuietNan(); for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] = v;}
#else
	Matrix3x3()																		{}
#endif

	/** Rotation matrix from axis-angle representation. */
	Matrix3x3( const Vector3& axis, float angle );
	
	/** Identity matrix multiplied with specified value. */
	explicit Matrix3x3( float diagonal );
	
	/** Rotation matrix from the quaternion. */
	explicit Matrix3x3( const Quaternion& q );

	/** Component-wise addition of matrices.*/
	Matrix3x3&		operator+=( const Matrix3x3& other )							{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] += other.m[i][j]; return *this;}
	
	/** Component-wise subtraction of matrices.*/
	Matrix3x3&		operator-=( const Matrix3x3& other )							{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] -= other.m[i][j]; return *this;}
	
	/** Matrix multiplication.*/
	Matrix3x3&		operator*=( const Matrix3x3& other )							{return *this = *this * other;}
	
	/**	Multiplies all components of this matrix with a scalar.*/
	Matrix3x3&		operator*=( float s )											{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] *= s; return *this;}

	/** Access element at (row,column) in the matrix.*/
	float&			operator()( int row, int column )								{return m[row][column];}
	
	/** Sets element at (row,column) in the matrix.*/
	void			set( int row, int column, float value )							{m[row][column]=value;}

	/** Sets ith column of the matrix. */
	void			setColumn( int i, const Vector3& c )							{m[0][i]=c[0]; m[1][i]=c[1]; m[2][i]=c[2];}

	/** Sets ith row of the matrix. */
	void			setRow( int i, const Vector3& c )								{m[i][0]=c[0]; m[i][1]=c[1]; m[i][2]=c[2];}

	/** Generates orthonormal coordinate system about specified axis. */
	void			generateOrthonormalBasis( const Vector3& axis );

	/** Returns component-wise addition of matrices.*/
	Matrix3x3		operator+( const Matrix3x3& other ) const						{Matrix3x3 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = m[i][j] + other.m[i][j]; return a;}
	
	/** Returns component-wise subtraction of matrices.*/
	Matrix3x3		operator-( const Matrix3x3& other ) const						{Matrix3x3 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = m[i][j] - other.m[i][j]; return a;}

	/** Returns component-wise negation.*/
	Matrix3x3		operator-() const												{Matrix3x3 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = -m[i][j]; return a;}
	
	/**	Returns this matrix multiplied with a scalar.*/
	Matrix3x3		operator*( float s ) const										{Matrix3x3 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = m[i][j] * s; return a;}
	
	/** Matrix multiplication.*/
	Matrix3x3		operator*( const Matrix3x3& other ) const;

	/* Matrix multiplication with 3-vector as 1-column matrix.*/
	Vector3			operator*( const Vector3& v ) const;
	
	/** Component-wise equality.*/
	bool			operator==( const Matrix3x3& other ) const						{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) if ( m[i][j] != other.m[i][j] ) return false; return true;}
	
	/** Component-wise inequality.*/
	bool			operator!=( const Matrix3x3& other ) const						{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) if ( m[i][j] != other.m[i][j] ) return true; return false;}

	/** Access element at (row,column) in the matrix.*/
	const float&	operator()( int row, int column ) const							{return m[row][column];}

	/** Returns element at (row,column) in the matrix.*/
	float			get( int row, int column ) const								{return m[row][column];}

	/** Returns ith column of the matrix. */
	Vector3			getColumn( int i ) const										{Vector3 d; d[0]=m[0][i]; d[1]=m[1][i]; d[2]=m[2][i]; return d;}

	/** Returns ith row of the matrix. */
	Vector3			getRow( int i ) const											{Vector3 d; d[0]=m[i][0]; d[1]=m[i][1]; d[2]=m[i][2]; return d;}

	/** Returns determinant of the matrix. */
	float			determinant() const;
	
	/** Returns true if all elements are in finite range. */
	bool			finite() const													{return inFiniteRange( begin(), end() );}
	
	/** Returns true if the matrix describes lefthanded rotation. */
	bool			lefthand() const;

	/** Returns transposed matrix. */
	Matrix3x3		transpose() const;

	/** Returns inverse of the matrix. */
	Matrix3x3		inverse() const;

	/** Returns the matrix with its column vectors orthonormalized. */
	Matrix3x3		orthonormalize() const;

	/** Multiplies the matrix and the 3-vector. */
	void			rotate( const Vector3& v, Vector3* v1 ) const;

	/** Multiplies the matrix and the 3-vector. */
	Vector3			rotate( const Vector3& v ) const;

private:
	float			m[ROWS][COLUMNS];

	float*			begin()															{return &m[0][0];}
	float*			end()															{return &m[0][0] + SIZE;}
	const float*	begin() const													{return &m[0][0];}
	const float*	end() const														{return &m[0][0] + SIZE;}
};


#include "Matrix3x3.inl"


} // math


#endif // _MATH_MATRIX3X3_H
