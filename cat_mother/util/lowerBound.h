#ifndef _UTIL_LOWERBOUND_H
#define _UTIL_LOWERBOUND_H


namespace util
{


/** 
 * Binary search.
 * @param first Random access iterator to the first item.
 * @param last Random access iterator to one beyond the last item.
 * @param value Value to search for.
 * @return Iterator to the first item which is is <= value.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
template <class It, class T>
It lowerBound( It first, It last, const T& value )
{
	unsigned n = last - first;
	while ( 0 < n )
	{
		unsigned n2 = n / 2;
		It m = first + n2;
		if ( *m < value )
			first = ++m,
			n -= n2 + 1;
		else
			n = n2; 
	}
	return first;
} 


} // util


#endif // _UTIL_LOWERBOUND_H
