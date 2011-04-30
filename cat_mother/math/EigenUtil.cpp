#include "EigenUtil.h"
#include "Matrix3x3.h"
#include <math.h>
#include <float.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

#define ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau); \
	a[k][l]=h+s*(g-h*tau);

#define MAX_ITERATIONS 50

//-----------------------------------------------------------------------------

namespace math
{


/** 
 * Computes all eigenvalues and eigenvectors of a real symmetric
 * matrix a[1..n][1..n]. On output, elements of a above the diagonal
 * are destroyed. d[1..n] returns the eigenvalues of a. v[1..n][1..n]
 * is a matrix whose columns contain, on output, the normalized
 * eigenvectors of a. nrot returns the number of Jacobi rotations that
 * were required.
 *
 * See Numerical Recipes in C, chapter 11.
 */
static void jacobi(float **a, int n, float d[], float **v, int *nrot) 
{ 
	assert( n <= 10 );

	float b[10+1];
	float z[10+1];

	int j,iq,ip,i; 
	float tresh,theta,tau,t,sm,s,h,g,c; 

	/* Initialize to the identity matrix. */			
	for (ip=1;ip<=n;ip++) { 	  
		for (iq=1;iq<=n;iq++) 
			v[ip][iq]=0.0f;
		v[ip][ip]=1.0; 
	} 

	/* Initialize b and d to the diagonal of a. */
	for (ip=1;ip<=n;ip++) {
		b[ip]=d[ip]=a[ip][ip]; 
		z[ip]=0.0f; 
		/* z - This vector will accumulate terms of the form tapq as
		   in equa- tion (11.1.14). */
	} 

	*nrot=0; 

	for (i=1;i<=MAX_ITERATIONS;i++) {
		sm=0.0f; 
		/* Sum off-diagonal elements. */
		for (ip=1;ip<=n-1;ip++) {
			for (iq=ip+1;iq<=n;iq++) 
				sm += fabsf(a[ip][iq]); 
		} 

		/*The normal return, which relies on quadratic convergence to
		  machine underflow.*/
		if ( sm < FLT_MIN )
			 return; 

		if (i < 4) 
			tresh=0.2f*sm/(n*n); /*...on the  rst three sweeps. */ 
		else 
			tresh=0.0f; /*...thereafter. */
		
		for  ( ip = 1;ip <= n-1; ip++) {
			for (iq=ip+1;iq<=n;iq++) {
				g=100.f*fabsf(a[ip][iq]); 
				/* After four sweeps, skip the rotation if the off
				   diagonal element is small.*/

				if	( ( i > 4 )  
					  &&  ((float)(fabsf(d[ip])+g) == (float)fabsf(d[ip]))
					  && ((float)(fabsf(d[iq])+g) == (float)fabsf(d[iq])))
					a[ip][iq]=0.0f;
				else 
					if (fabsf(a[ip][iq]) > tresh) {
						h=d[iq]-d[ip]; 
						if ((float)(fabsf(h)+g) == (float)fabsf(h))
							t = (a[ip][iq])/h; /* t = 1=(2 ) */
						else {
							theta=0.5f*h/(a[ip][iq]); /* Equation (11.1.10).*/
							t=1.f/(fabsf(theta)+sqrtf(1.f+theta*theta)); 
							if (theta < 0.0f) 
								t = -t; 
						} 
						c=1.f/sqrtf(1+t*t); 
						s=t*c; 
						tau=s/(1.f+c);
						h=t*a[ip][iq];
						z[ip] -= h;
						z[iq] += h;
						d[ip] -= h;
						d[iq] += h; 
						a[ip][iq]=0.f; 
						for (j=1;j<=ip-1;j++)  {
							/* Case of rotations 1	 j <p.*/
							ROTATE(a,j,ip,j,iq);
						} 
						for (j=ip+1;j<=iq-1;j++) {
							/* Case of rotations p <j<q. */
							ROTATE(a,ip,j,j,iq) ;
						}
						for (j=iq+1;j<=n;j++) {
							/* Case of rotations q <j	n. */
							ROTATE(a,ip,j,iq,j);
						} 
						for (j=1;j<=n;j++) {
							ROTATE(v,j,ip,j,iq); 
						} 
						++(*nrot); 
					} 
			} 
		} 
		for (ip=1;ip<=n;ip++) {
			b[ip] += z[ip]; 
			d[ip]=b[ip]; 
			/*Update d with the sum of tapq,*/
			z[ip]=0.0f;/* and reinitialize z. */
		}
	} 

	assert( i < MAX_ITERATIONS );
}

/** 
 * Given the eigenvalues d[1..n] and eigenvectors v[1..n][1..n] as
 * output from jacobi ( x 11.1) or tqli ( x 11.3), this routine
 * sorts the eigenvalues into descending order, and rearranges the
 * columns of v correspondingly. The method is straight insertion.
 */
static void eigsrt(float d[], float **v, int n) 
{
	int   k,j,i;
	float	p;

	for  ( i = 1; i < n; i++ ) {
		p = d[k = i];
		
		for (j=i+1;j<=n;j++) 
			if (d[j] >= p) 
				p=d[k=j];
		if (k != i) { 
			d[k]=d[i];
			d[i]=p;
			for (j=1;j<=n;j++) { 
				p=v[j][i]; 
				v[j][i]=v[j][k];				v[j][k]=p; 
			} 
		} 
	} 
}

//-----------------------------------------------------------------------------

void EigenUtil::computeSymmetric( const Matrix3x3& mat,
	float* eigenValues, Matrix3x3* eigenVectors )
{
	float buf1[3*4];
	float buf2[3*4];

	float* p_mat[4];
	p_mat[0] = buf1;
	p_mat[1] = buf1;
	p_mat[2] = buf1+3;
	p_mat[3] = buf1+3*2;

	float* p_out_vec[4];
	p_out_vec[0] = buf2;
	p_out_vec[1] = buf2;
	p_out_vec[2] = buf2+3;
	p_out_vec[3] = buf2+3*2;

	float  eigen_vals[ 10 ];
	int rot = 0;

	for  ( int	ind = 0; ind < 3; ind++ )
		for  ( int	jnd = 0; jnd < 3; jnd++ )
			p_mat[ ind + 1 ][ jnd + 1 ] = mat( ind , jnd );
	
	jacobi( p_mat, 3, eigen_vals, p_out_vec, &rot );
	eigsrt( eigen_vals, p_out_vec, 3 );

	if ( eigenValues )
	{
		eigenValues[ 0 ] = eigen_vals[ 1 ];
		eigenValues[ 1 ] = eigen_vals[ 2 ];
		eigenValues[ 2 ] = eigen_vals[ 3 ];
	}

	if ( eigenVectors )
	{
		Matrix3x3& evec = *eigenVectors;
		for  ( int	ind = 0; ind < 3; ind++ )
			for  ( int	jnd = 0; jnd < 3; jnd++ )
				evec( ind , jnd ) = p_out_vec[ jnd + 1 ][ ind + 1 ];
	}
}


} // math
