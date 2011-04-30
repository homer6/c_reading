namespace mb
{


/** 
 * Minimal support for 3-vector operations. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Vec3
{
public:
	/** d = s. */
	static void set( float* d, float s )
	{
		for ( int i = 0 ; i < 3 ; ++i )
			d[i] = s;
	}

	/** d = a * s. */
	static void scale( float* d, const float* a, float s )
	{
		for ( int i = 0 ; i < 3 ; ++i )
			d[i] = a[i] * s;
	}

	/** d = a + b. */
	static void add( float* d, const float* a, const float* b )
	{
		for ( int i = 0 ; i < 3 ; ++i )
			d[i] = a[i] + b[i];
	}

	/** d = a - b. */
	static void sub( float* d, const float* a, const float* b )
	{
		for ( int i = 0 ; i < 3 ; ++i )
			d[i] = a[i] - b[i];
	}

	/** d = a cross b. */
	static void cross( float* d, const float* a, const float* b )
	{
		d[0] = a[1]*b[2] - a[2]*b[1];
		d[1] = a[2]*b[0] - a[0]*b[2];
		d[2] = a[0]*b[1] - a[1]*b[0];
	}

	/** Returns a dot b. */
	static float dot( const float* a, const float* b )
	{
		float s = 0.f;
		for ( int i = 0 ; i < 3 ; ++i )
			s += a[i] * b[i];
		return s;
	}
};


} // mb
