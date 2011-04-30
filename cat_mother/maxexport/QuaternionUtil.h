/** 
 * Quaternion algebra utility. 
 * Quaternion is stored as an array of (x,y,z,w). 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class QuaternionUtil
{
public:
	/** Copies quaternion. */
	static void		copy( float* d, const float* q )								{for ( int i = 0 ; i < 4 ; ++i ) d[i] = q[i];}

	/** Adds two quaternions together. */
	static void		add( float* d, const float* q, const float* p )					{for ( int i = 0 ; i < 4 ; ++i ) d[i] = q[i] + p[i];}

	/** Subtracts two quaternions. */
	static void		subtract( float* d, const float* q, const float* p )			{for ( int i = 0 ; i < 4 ; ++i ) d[i] = q[i] - p[i];}

	/** Returns quaternion dot product. */
	static float	dot( const float* p, const float* q )							{float v = 0.f; for ( int i = 0 ; i < 4 ; ++i )	v += p[i] * q[i]; return v;}

	/** Negates quaternion. */
	static void		negate( float* d, const float* q )								{for ( int i = 0 ; i < 4 ; ++i ) d[i] = -q[i];}

	/** Multiplies quaternion by scalar. */
	static void		scale( float* d, const float* q, float s )						{for ( int i = 0 ; i < 4 ; ++i ) d[i] = q[i] * s;}

	/** Returns norm of the quaternion. */
	static float	norm( const float* q );

	/** Spherical linear interpolation. */
	static void		slerp( float* d, float t, const float* p, const float* q );

	/** Cubic spline interpolation. */
	static void		squad( float* d, float t, const float* p, const float* a, const float* b, const float* q );

	/** Normalizes quaternion. */
	static void		normalize( float* d, const float* q );

	/** Inverts quaternion. */
	static void		inverse( float* d, const float* q );

	/** Natural logarithm of the quaternion. */
	static void		log( float* d, const float* q );

	/** Natural exponent of the quaternion. */
	static void		exp( float* d, const float* q );

	/** Quaternion multiply. */
	static void		mul( float* d, const float* a, const float* b );
};
