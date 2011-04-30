#include "Random.h"
//#include <lang/Debug.h>
#include <time.h>
#include "config.h"

//-----------------------------------------------------------------------------

namespace util
{


Random::Random()
{
	time_t t;
	time(&t);
	setSeed( (long)t );
}

Random::Random( long seed )
{
	setSeed( seed );
}

void Random::setSeed( long seed )
{
	m_seed = seed ^ 0x5EECE66D;
}

int Random::nextInt()
{
	return next(32);
}

long Random::nextLong()
{
	return next(32);
}

float Random::nextFloat()
{
	int v = next(32);
	v &= 0x7FFFFFF;
	return float(v) / float(0x8000000);
}

double Random::nextDouble()
{
	int v = next(32);
	v &= 0x7FFFFFF;
	return double(v) / double(0x8000000);
}

bool Random::nextBoolean()
{
	return 0 != (next(32) & 256);
}

int	Random::next( int /*bits*/ )
{
	// Linear congruential generator (LCG):
	// I(k) = ( a * I(k-1) + c ) % m,

	// Minimal standard RNG (Park and Miller 1988):
	const int a = 16807;
	const int c = 0;
	const int m = 2147483647;

	// sample 2 times, use only bits 8-16 from second sample
	m_seed = ( a * m_seed + c ) % m;
	int v = m_seed;
	m_seed = ( a * m_seed + c ) % m;
	v += (m_seed >> 8) & 0xFF;

	//lang::Debug::println( "Random.next: {0,x}", v );
	return v;
}


} // util
