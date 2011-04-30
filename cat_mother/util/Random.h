#ifndef _UTIL_RANDOM_H
#define _UTIL_RANDOM_H


#include <lang/Object.h>


namespace util
{


/** 
 * Linear congruential pseudo-random number generator.
 * Not synchronized.
 *
 * Formula:
 * I(k) = ( a * I(k-1) + c ) % m,
 *
 * Minimal standard RNG (Park and Miller 1988):
 * <ul>
 * <li>a = 16807
 * <li>c = 0
 * <li>m = 2147483647
 * </ul>
 *
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Random :
	public lang::Object
{
public:
	/** Seeds RNG from system clock. */
	Random();

	/** Seeds RNG with explicit seed value. */
	explicit Random( long seed );

	/** Sets random number generator seed. */
	void	setSeed( long seed );

	/** Returns random int. */
	int		nextInt();

	/** Returns random long. */
	long	nextLong();
	
	/** Returns random float in range [0,1). */
	float	nextFloat();

	/** Returns random float in range [0,1). */
	double	nextDouble();

	/** Returns random bool. */
	bool	nextBoolean();

private:
	long m_seed;

	virtual int	next( int bits );
};


} // util


#endif // _UTIL_RANDOM_H
