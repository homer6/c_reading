#ifndef _MATH_MATRIX4X4_H
#define _MATH_MATRIX4X4_H


#include <math/Vector4.h>
#include <math/Matrix3x3.h>


namespace math
{


/**
 * 4x4 row-major matrix of scalar type float.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Matrix4x4 : 
	public MatrixMxN
{
public:
	/** Constants related to the class. */
	enum Constants 
	{ 
		/** Number of rows in a matrix.*/
		ROWS	= 4, 
		/** Number of columns in a matrix.*/
		COLUMNS	= 4, 
		/** Number of components in a matrix.*/
		SIZE	= 16,
	};

#ifdef _DEBUG
	/** Constructs undefined matrix. */
	Matrix4x4()																		{float v = valueQuietNan(); for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] = v;}
#else
	Matrix4x4()																		{}
#endif
	
	/** Matrix from 4 column vectors. */
	Matrix4x4( const Vector4& x0, const Vector4& y0, 
		const Vector4& z0, const Vector4& w0 )										{setColumn(0,x0); setColumn(1,y0); setColumn(2,z0); setColumn(3,w0);}

	/** Identity matrix multiplied with specified scalar. */
	explicit Matrix4x4( float diagonal );

	/** Transformation matrix from rotation and translation. */
	Matrix4x4( const Matrix3x3& rotation, const Vector3& translation );

	/** Component-wise addition of matrices.*/
	Matrix4x4&		operator+=( const Matrix4x4& other )							{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] += other.m[i][j]; return *this;}
	
	/** Component-wise subtraction of matrices.*/
	Matrix4x4&		operator-=( const Matrix4x4& other )							{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] -= other.m[i][j]; return *this;}
	
	/** Matrix multiplication.*/
	Matrix4x4&		operator*=( const Matrix4x4& other )							{return *this = *this * other;}
	
	/**	Multiplies all components of this matrix with a scalar.*/
	Matrix4x4&		operator*=( float s )											{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) m[i][j] *= s; return *this;}

	/** Access element at (row,column) in the matrix.*/
	float&			operator()( int row, int column )								{return m[row][column];}
	
	/** Sets element at (row,column) in the matrix.*/
	void			set( int row, int column, float value )							{m[row][column]=value;}

	/** Sets ith column of the matrix. */
	void			setColumn( int i, const Vector4& c )							{m[0][i]=c[0]; m[1][i]=c[1]; m[2][i]=c[2]; m[3][i]=c[3];}

	/** Sets ith row of the matrix. */
	void			setRow( int i, const Vector4& c )								{m[i][0]=c[0]; m[i][1]=c[1]; m[i][2]=c[2]; m[i][3]=c[3];}

	/** Sets top-left 3x3 submatrix. */
	void			setRotation( const Matrix3x3& rot );

	/** Sets right column (excluding the last row) of the matrix. */
	void			setTranslation( const Vector3& t );

	/** 
	 * Sets this matrix as inverse transform of Matrix4x4(rot,t). 
	 * Assumes that the matrix is orthonormal.
	 */
	void			setInverseOrthonormalTransform( const Matrix3x3& rot, const Vector3& t );

	/** 
	 * Left handed perspective projection matrix.
	 *
	 * @param fovHorz Horizontal field of view in radians.
	 * @param front Front plane distance.
	 * @param back Back plane distance.
	 * @param aspect Viewport width/height.
	 */
	void			setPerspectiveProjection( float fovHorz, float front, float back, float aspect );

	/** 
	 * Matrix projects points to the plane along 
	 * the line from the projection origin to the point to be projected.
	 *
	 * @param l Projection origin.
	 * @param n Normal of the projection plane.
	 * @param p Point on the projection plane.
	 */
	void			setPointPlaneProjection( const Vector3& l,
						const Vector3& n, const Vector3& p );

	/** 
	 * Matrix projects points to the plane along 
	 * lines parallel to specified direction.
	 *
	 * @param l Projection direction.
	 * @param n Normal of the projection plane.
	 * @param p Point on the projection plane.
	 */
	void			setDirectPlaneProjection( const Vector3& l,
						const Vector3& n, const Vector3& p );

	/** Returns component-wise addition of matrices.*/
	Matrix4x4		operator+( const Matrix4x4& other ) const						{Matrix4x4 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = m[i][j] + other.m[i][j]; return a;}
	
	/** Returns component-wise subtraction of matrices.*/
	Matrix4x4		operator-( const Matrix4x4& other ) const						{Matrix4x4 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = m[i][j] - other.m[i][j]; return a;}

	/** Returns component-wise negation.*/
	Matrix4x4		operator-() const												{Matrix4x4 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = -m[i][j]; return a;}
	
	/**	Returns this matrix multiplied with a scalar.*/
	Matrix4x4		operator*( float s ) const										{Matrix4x4 a; for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) a.m[i][j] = m[i][j] * s; return a;}
	
	/** Matrix multiplication.*/
	Matrix4x4		operator*( const Matrix4x4& other ) const;

	/* Matrix multiplication with 4-vector as 1-column matrix.*/
	Vector4			operator*( const Vector4& v ) const;

	/** Component-wise equality.*/
	bool			operator==( const Matrix4x4& other ) const						{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) if ( m[i][j] != other.m[i][j] ) return false; return true;}
	
	/** Component-wise inequality.*/
	bool			operator!=( const Matrix4x4& other ) const						{for ( int i = 0 ; i < ROWS ; ++i ) for ( int j = 0 ; j < COLUMNS ; ++j ) if ( m[i][j] != other.m[i][j] ) return true; return false;}

	/** Access element at (row,column) in the matrix.*/
	const float&	operator()( int row, int column ) const							{return m[row][column];}

	/** Returns element at (row,column) in the matrix.*/
	float			get( int row, int column ) const								{return m[row][column];}

	/** Returns ith column of the matrix. */
	Vector4			getColumn( int i ) const										{Vector4 d; d[0]=m[0][i]; d[1]=m[1][i]; d[2]=m[2][i]; d[3]=m[3][i]; return d;}

	/** Returns ith row of the matrix. */
	Vector4			getRow( int i ) const											{Vector4 d; d[0]=m[i][0]; d[1]=m[i][1]; d[2]=m[i][2]; d[3]=m[i][3]; return d;}

	/** Returns top-left 3x3 submatrix. */
	Matrix3x3		rotation() const;

	/** Returns top-right 3x1 submatrix. */
	Vector3			translation() const;

	/** Returns true if all elements are in finite range. */
	bool			finite() const													{return inFiniteRange( begin(), end() );}

	/** Returns transposed matrix. */
	Matrix4x4		transpose() const;

	/** Returns determinant of the matrix. */
	float			determinant() const;

	/** Returns inverse of the matrix. */
	Matrix4x4		inverse() const;

	/** Multiplies the matrix and the 3-vector as (x,y,z,1). */
	Vector3			transform( const Vector3& v ) const;

	/** Multiplies the matrix and the 3-vector as (x,y,z,1). */
	void			transform( const Vector3& v, Vector3* v1 ) const;

	/** Multiplies the matrix and the 4-vector as (x,y,z,w). */
	void			transform( const Vector4& v, Vector4* v1 ) const;

	/** Multiplies the matrix and the 3-vector as (x,y,z,0). */
	Vector3			rotate( const Vector3& v ) const;

	/** Multiplies the matrix and the 3-vector as (x,y,z,0). */
	void			rotate( const Vector3& v, Vector3* v1 ) const;

private:
	float			m[ROWS][COLUMNS];

	float*			begin()															{return &m[0][0];}
	float*			end()															{return &m[0][0] + SIZE;}
	const float*	begin() const													{return &m[0][0];}
	const float*	end() const														{return &m[0][0] + SIZE;}
};


#include "Matrix4x4.inl"


} // math


#endif // _MATH_MATRIX4X4_H
