#ifndef _MATH_EIGENUTIL_H
#define _MATH_EIGENUTIL_H


namespace math
{


class Matrix3x3;


/** 
 * Utilities for computing matrix eigenvalues and eigenvectors. 
 */
class EigenUtil
{
public:
	/** 
	 * Computes eigenvalues and eigenvectors of a symmetric 3x3 matrix. 
	 * Uses Jacobi Transformations (see Numerical Recipes in C, chapter 11).
	 * @param mat Source matrix for eigenvalue computation.
	 * @param eigenValues [out] Receives matrix eigenvalues (3). Can be 0.
	 * @param eigenValues [out] Receives matrix eigenvectors (3) as columns. Can be 0.
	 */
	static void	computeSymmetric( const Matrix3x3& mat,
		float* eigenValues, Matrix3x3* eigenVectors );
};


} // math


#endif // _MATH_EIGENUTIL_H
