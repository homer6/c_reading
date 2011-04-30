#include "Hashtable.h"
#include "config.h"

//-----------------------------------------------------------------------------

namespace util
{


int Hashtable_getLargerInt( int n )
{
	static const int primes[] = 
	{
		37, 67, 131, 257, 521, 1031, 2053, 4099, 
		8209, 16411, 32771, 65543, 129403 
	};
	
	for ( int i = 0 ; i < sizeof(primes)/sizeof(primes[0]) ; ++i )
		if ( primes[i] > n )
			return primes[i];

	n += primes[ sizeof(primes)/sizeof(primes[0]) - 1 ];
	n |= 1;
	return n;
}


} // util
